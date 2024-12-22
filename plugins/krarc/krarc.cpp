/*
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krarc.h"
#include "../../app/defaults.h"

// QtCore
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextCodec>
#include <qplatformdefs.h>

#include <KFileItem>
#include <KIO/Job>
#include <KIO/AuthInfo>
#include <KLocalizedString>
#include <KProcess>
#include <KTar>

#include <kio_version.h>

#include "../../app/compat.h"
#include <errno.h>

#define MAX_IPC_SIZE (1024 * 32)
#define TRIES_WITH_PASSWORDS 3

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.krarc" FILE "krarc.json")
};

using namespace KIO;
extern "C" {

#ifdef KRARC_ENABLED
/* This codec is for being able to handle files which encoding differs from the current locale.
 *
 * Unfortunately QProcess requires QString parameters for arguments which are encoded to Local8Bit
 * If we want to use unzip with ISO-8852-2 when the current locale is UTF-8, it will cause problems.
 *
 * Workaround:
 *  1. encode the QString to QByteArray ( according to the selected remote encoding, ISO-8852-2 )
 *  2. encode QByteArray to QString again
 *     unicode 0xE000-0xF7FF is for private use
 *     the byte array is mapped to 0xE000-0xE0FF unicodes
 *  3. KrArcCodec maps 0xE000-0xE0FF to 0x0000-0x00FF, while calls the default encoding routine
 *     for other unicodes.
 */

class KrArcCodec : public QTextCodec
{
public:
    KrArcCodec(QTextCodec *codec)
        : originalCodec(codec)
    {
    }
    ~KrArcCodec() override = default;

    QByteArray name() const override
    {
        return originalCodec->name();
    }
    QList<QByteArray> aliases() const override
    {
        return originalCodec->aliases();
    }
    int mibEnum() const override
    {
        return originalCodec->mibEnum();
    }

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const override
    {
        return originalCodec->toUnicode(in, length, state);
    }
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const override
    {
        // the QByteArray is embedded into the unicode charset (QProcess hell)
        QByteArray result;
        for (int i = 0; i != length; i++) {
            if (((in[i].unicode()) & 0xFF00) == 0xE000) // map 0xE000-0xE0FF to 0x0000-0x00FF
                result.append((char)(in[i].unicode() & 0xFF));
            else
                result.append(originalCodec->fromUnicode(in + i, 1, state));
        }
        return result;
    }

private:
    QTextCodec *originalCodec;
} *krArcCodec;

#define SET_KRCODEC                                                                                                                                            \
    QTextCodec *origCodec = QTextCodec::codecForLocale();                                                                                                      \
    QTextCodec::setCodecForLocale(krArcCodec);
#define RESET_KRCODEC QTextCodec::setCodecForLocale(origCodec);

#endif // KRARC_ENABLED

class DummyWorker : public KIO::WorkerBase
{
public:
    DummyWorker(const QByteArray &pool_socket, const QByteArray &app_socket)
        : WorkerBase("kio_krarc", pool_socket, app_socket){

        }
};

int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    if (argc != 4) {
        qWarning() << "Usage: kio_krarc  protocol domain-socket1 domain-socket2" << QT_ENDL;
        exit(-1);
    }

    // At least, that fixes the empty name in the warning that says:  Please fix the "" KIO slave
    // There is more information in https://bugs.kde.org/show_bug.cgi?id=384653
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kio_krarc"));

#ifdef KRARC_ENABLED
    kio_krarcProtocol worker(argv[2], argv[3]);

#else
    DummyWorker worker(argv[2], argv[3]);

#endif

    worker.dispatchLoop();

    return 0;
}

} // extern "C"

#ifdef KRARC_ENABLED
kio_krarcProtocol::kio_krarcProtocol(const QByteArray &pool_socket, const QByteArray &app_socket)
    : WorkerBase("kio_krarc", pool_socket, app_socket)
    , archiveChanged(true)
    , arcFile(nullptr)
    , extArcReady(false)
    , password(QString())
    , codec(nullptr)
{
    KRFUNC;
    KConfigGroup group(&krConf, "General");
    QString tmpDirPath = group.readEntry("Temp Directory", _TempDirectory);
    QDir tmpDir(tmpDirPath);
    if (!tmpDir.exists()) {
        for (qsizetype i = 1; i != -1; i = tmpDirPath.indexOf('/', i + 1))
            QDir().mkdir(tmpDirPath.left(i));
        QDir().mkdir(tmpDirPath);
    }

    arcTempDir = tmpDirPath + DIR_SEPARATOR;
    QString dirName = "krArc" + QDateTime::currentDateTime().toString(Qt::ISODate);
    dirName.replace(QRegularExpression(":"), "_");
    tmpDir.mkdir(dirName);
    arcTempDir = arcTempDir + dirName + DIR_SEPARATOR;

    krArcCodec = new KrArcCodec(QTextCodec::codecForLocale());
}

/* ---------------------------------------------------------------------------------- */
kio_krarcProtocol::~kio_krarcProtocol()
{
    KRFUNC;
    // delete the temp directory
    KProcess proc;
    proc << fullPathName("rm") << "-rf" << arcTempDir;
    proc.start();
    proc.waitForFinished();
}

/* ---------------------------------------------------------------------------------- */

KIO::WorkerResult kio_krarcProtocol::checkWriteSupport()
{
    KRFUNC;
    krConf.reparseConfiguration();
    if (KConfigGroup(&krConf, "kio_krarc").readEntry("EnableWrite", false))
        return WorkerResult::pass();

    return WorkerResult::fail(ERR_UNSUPPORTED_ACTION,
                              i18n("krarc: write support is disabled.\n"
                                   "You can enable it on the 'Archives' page in Konfigurator."));
}

void kio_krarcProtocol::receivedData(KProcess *, QByteArray &d)
{
    KRFUNC;
    const QByteArray &buf(d);
    data(buf);
    processedSize(d.length());
    decompressedLen += d.length();
}

KIO::WorkerResult kio_krarcProtocol::mkdir(const QUrl &url, int permissions)
{
    KRFUNC;
    const QString path = getPath(url);
    KRDEBUG(path);

    const auto writeSupportResult = checkWriteSupport();
    if (!writeSupportResult.success())
        return writeSupportResult;

        // In case of KIO::mkpath call there is a mkdir call for every path element.
        // Therefore the path all the way up to our archive needs to be checked for existence
        // and reported as success.
    if (QDir().exists(path)) {
        return WorkerResult::pass();
    }

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }

    if (newArchiveURL && !initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, path);
    }

    if (putCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Creating folders is not supported with %1 archives.", arcType));
    }

    const QString arcFilePath = getPath(arcFile->url());

    if (arcType == "arj" || arcType == "lha") {
        QString arcDir = path.mid(arcFilePath.length());
        if (arcDir.right(1) != DIR_SEPARATOR)
            arcDir = arcDir + DIR_SEPARATOR;

        if (dirDict.find(arcDir) == dirDict.end())
            addNewDir(arcDir);
        return WorkerResult::pass();
    }

    QString arcDir = findArcDirectory(url);
    QString tempDir = arcDir.mid(1) + path.mid(path.lastIndexOf(DIR_SEPARATOR) + 1);
    if (tempDir.right(1) != DIR_SEPARATOR)
        tempDir = tempDir + DIR_SEPARATOR;

    if (permissions == -1)
        permissions = 0777; // set default permissions

    QByteArray arcTempDirEnc = arcTempDir.toLocal8Bit();
    for (qsizetype i = 0; i < tempDir.length() && i >= 0; i = tempDir.indexOf(DIR_SEPARATOR, i + 1)) {
        QByteArray newDirs = encodeString(tempDir.left(i));
        newDirs.prepend(arcTempDirEnc);
        QT_MKDIR(newDirs.constData(), permissions);
    }

    if (tempDir.endsWith(DIR_SEPARATOR))
        tempDir.truncate(tempDir.length() - 1);

    // pack the directory
    KrLinecountingProcess proc;
    proc << putCmd << arcFilePath << localeEncodedString(tempDir);
    infoMessage(i18n("Creating %1...", url.fileName()));
    QDir::setCurrent(arcTempDir);

    SET_KRCODEC
    proc.start();
    RESET_KRCODEC

    proc.waitForFinished();

    // delete the temp directory
    QDir().rmdir(arcTempDir);

    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())) {
        return WorkerResult::fail(ERR_CANNOT_WRITE, path + "\n\n" + proc.getErrorMsg());
    }

    //  force a refresh of archive information
    initDirDict(url, true);
    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::put(const QUrl &url, int permissions, KIO::JobFlags flags)
{
    KRFUNC;
    KRDEBUG(getPath(url));

    const auto writeSupportResult = checkWriteSupport();
    if (!writeSupportResult.success())
        return writeSupportResult;

    bool overwrite = !!(flags & KIO::Overwrite);
    bool resume = !!(flags & KIO::Resume);

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }
    if (newArchiveURL && !initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }

    if (putCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Writing to %1 archives is not supported.", arcType));
    }
    if (!overwrite && findFileEntry(url)) {
        return WorkerResult::fail(ERR_FILE_ALREADY_EXIST, getPath(url));
    }

    QString arcDir = findArcDirectory(url);
    if (arcDir.isEmpty())
        KRDEBUG("arcDir is empty.");

    QString tempFile = arcDir.mid(1) + getPath(url).mid(getPath(url).lastIndexOf(DIR_SEPARATOR) + 1);
    QString tempDir = arcDir.mid(1);
    if (tempDir.right(1) != DIR_SEPARATOR)
        tempDir = tempDir + DIR_SEPARATOR;

    if (permissions == -1)
        permissions = 0777; // set default permissions

    QByteArray arcTempDirEnc = arcTempDir.toLocal8Bit();
    for (qsizetype i = 0; i < tempDir.length() && i >= 0; i = tempDir.indexOf(DIR_SEPARATOR, i + 1)) {
        QByteArray newDirs = encodeString(tempDir.left(i));
        newDirs.prepend(arcTempDirEnc);
        QT_MKDIR(newDirs.constData(), 0755);
    }

    int fd;
    if (resume) {
        QByteArray ba = encodeString(tempFile);
        ba.prepend(arcTempDirEnc);
        fd = QT_OPEN(ba.constData(), O_RDWR); // append if resuming
        QT_LSEEK(fd, 0, SEEK_END); // Seek to end
    } else {
        // WABA: Make sure that we keep writing permissions ourselves,
        // otherwise we can be in for a surprise on NFS.
        mode_t initialMode;
        if (permissions != -1)
            initialMode = permissions | S_IWUSR | S_IRUSR;
        else
            initialMode = 0666;

        QByteArray ba = encodeString(tempFile);
        ba.prepend(arcTempDirEnc);
        fd = QT_OPEN(ba.constData(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
    }

    QByteArray buffer;
    int readResult;
    bool isIncomplete = false;
    do {
        dataReq();
        readResult = readData(buffer);
        auto bytesWritten = ::write(fd, buffer.constData(), buffer.size());
        if (bytesWritten < buffer.size()) {
            isIncomplete = true;
            break;
        }
    } while (readResult > 0);
    ::close(fd);

    if (isIncomplete) {
        return WorkerResult::fail(ERR_CANNOT_WRITE, getPath(url));
    }

    // pack the file
    KrLinecountingProcess proc;
    proc << putCmd << getPath(arcFile->url()) << localeEncodedString(tempFile);
    infoMessage(i18n("Packing %1...", url.fileName()));
    QDir::setCurrent(arcTempDir);

    SET_KRCODEC
    proc.start();
    RESET_KRCODEC

    proc.waitForFinished();

    // remove the files
    QDir().rmdir(arcTempDir);

    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())) {
        return WorkerResult::fail(ERR_CANNOT_WRITE, getPath(url) + "\n\n" + proc.getErrorMsg());
    }
    //  force a refresh of archive information
    initDirDict(url, true);
    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::get(const QUrl &url)
{
    KRFUNC;
    return get(url, TRIES_WITH_PASSWORDS);
}

KIO::WorkerResult kio_krarcProtocol::get(const QUrl &url, int tries)
{
    KRFUNC;
    KRDEBUG(getPath(url));
    bool decompressToFile = false;

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }
    if (newArchiveURL && !initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }

    if (getCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Retrieving data from %1 archives is not supported.", arcType));
    }
    UDSEntry *entry = findFileEntry(url);
    if (!entry) {
        return WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, getPath(url));
    }
    if (KFileItem(*entry, url).isDir()) {
        return WorkerResult::fail(KIO::ERR_IS_DIRECTORY, getPath(url));
    }

    KIO::filesize_t expectedSize = KFileItem(*entry, url).size();
    // for RPM files extract the cpio file first
    if (!extArcReady && arcType == "rpm") {
        KrLinecountingProcess cpio;
        cpio << "rpm2cpio" << getPath(arcFile->url(), QUrl::StripTrailingSlash);
        cpio.setStandardOutputFile(arcTempDir + "contents.cpio");

        cpio.start();
        cpio.waitForFinished();

        if (cpio.exitStatus() != QProcess::NormalExit || !checkStatus(cpio.exitCode())) {
            return WorkerResult::fail(ERR_CANNOT_READ, getPath(url) + "\n\n" + cpio.getErrorMsg());
        }
        extArcReady = true;
    }
    // for DEB files extract the tar file first
    if (!extArcReady && arcType == "deb") {
        KrLinecountingProcess dpkg;
        dpkg << cmd << "--fsys-tarfile" << getPath(arcFile->url(), QUrl::StripTrailingSlash);
        dpkg.setStandardOutputFile(arcTempDir + "contents.cpio");

        dpkg.start();
        dpkg.waitForFinished();

        if (dpkg.exitStatus() != QProcess::NormalExit || !checkStatus(dpkg.exitCode())) {
            return WorkerResult::fail(ERR_CANNOT_READ, getPath(url) + "\n\n" + dpkg.getErrorMsg());
        }
        extArcReady = true;
    }

    // Use the external unpacker to unpack the file
    QString file = getPath(url).mid(getPath(arcFile->url()).length() + 1);
    KrLinecountingProcess proc;
    if (extArcReady) {
        proc << getCmd << arcTempDir + "contents.cpio" << '*' + localeEncodedString(file);
    } else if (arcType == "arj" || arcType == "ace" || arcType == "7z") {
        proc << getCmd << getPath(arcFile->url(), QUrl::StripTrailingSlash) << localeEncodedString(file);
        if (arcType == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
            proc.setStandardInputFile("/dev/ptmx");
        file = url.fileName();
        decompressToFile = true;
    } else {
        decompressedLen = 0;
        // Determine the mimetype of the file to be retrieved, and emit it.
        // This is mandatory in all slaves (for KRun/BrowserRun to work).
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(arcTempDir + file);
        if (mt.isValid())
            mimeType(mt.name());

        QString escapedFilename = file;
        if (arcType == "zip") // left bracket needs to be escaped
            escapedFilename.replace('[', "[[]");
        proc << getCmd << getPath(arcFile->url());
        if (arcType != "gzip" && arcType != "bzip2" && arcType != "lzma" && arcType != "xz")
            proc << localeEncodedString(escapedFilename);
        connect(&proc, &KrLinecountingProcess::newOutputData, this, &kio_krarcProtocol::receivedData);
        proc.setMerge(false);
    }
    infoMessage(i18n("Unpacking %1...", url.fileName()));
    // change the working directory to our arcTempDir
    QDir::setCurrent(arcTempDir);

    SET_KRCODEC
    proc.setTextModeEnabled(false);
    proc.start();
    RESET_KRCODEC

    // Wait until the external unpacker has finished
    if (!proc.waitForFinished(-1)) {
        return WorkerResult::fail(KIO::ERR_WORKER_DEFINED,
                                  i18n("An error has happened, related to the external program '%1'. "
                                       "The error message is: '%2'.",
                                       cmd,
                                       proc.errorString()));
    }

    if (!extArcReady && !decompressToFile) {
        if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())
            || (arcType != "bzip2" && arcType != "lzma" && arcType != "xz" && expectedSize != decompressedLen)) {
            if (encrypted && tries) {
                invalidatePassword();
                return get(url, tries - 1);
            }
            return WorkerResult::fail(KIO::ERR_ACCESS_DENIED, getPath(url) + "\n\n" + proc.getErrorMsg());
        }
    } else {
        if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode()) || !QFileInfo::exists(arcTempDir + file)) {
            if (decompressToFile)
                QFile(arcTempDir + file).remove();
            if (encrypted && tries) {
                invalidatePassword();
                return get(url, tries - 1);
            }
            return WorkerResult::fail(KIO::ERR_ACCESS_DENIED, getPath(url));
        }
        // the following block is ripped from KDE file KIO::Slave
        // $Id: krarc.cpp,v 1.43 2007/01/13 13:39:51 ckarai Exp $
        QByteArray _path(QFile::encodeName(arcTempDir + file));
        QT_STATBUF buff;
        if (QT_LSTAT(_path.constData(), &buff) == -1) {
            if (errno == EACCES)
                return WorkerResult::fail(KIO::ERR_ACCESS_DENIED, getPath(url));
            return WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, getPath(url));
        }
        if (S_ISDIR(buff.st_mode)) {
            return WorkerResult::fail(KIO::ERR_IS_DIRECTORY, getPath(url));
        }
        if (!S_ISREG(buff.st_mode)) {
            return WorkerResult::fail(KIO::ERR_CANNOT_OPEN_FOR_READING, getPath(url));
        }
        int fd = QT_OPEN(_path.constData(), O_RDONLY);
        if (fd < 0) {
            return WorkerResult::fail(KIO::ERR_CANNOT_OPEN_FOR_READING, getPath(url));
        }
        // Determine the mimetype of the file to be retrieved, and emit it.
        // This is mandatory in all slaves (for KRun/BrowserRun to work).
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(arcTempDir + file);
        if (mt.isValid())
            mimeType(mt.name());

        KIO::filesize_t processed_size = 0;

        QString resumeOffset = metaData("resume");
        if (!resumeOffset.isEmpty()) {
            bool ok;
            KIO::fileoffset_t offset = resumeOffset.toLongLong(&ok);
            if (ok && (offset > 0) && (offset < buff.st_size)) {
                if (QT_LSEEK(fd, offset, SEEK_SET) == offset) {
                    canResume();
                    processed_size = offset;
                }
            }
        }

        totalSize(buff.st_size);

        char buffer[MAX_IPC_SIZE];
        while (1) {
            int n = static_cast<int>(::read(fd, buffer, MAX_IPC_SIZE));
            if (n == -1) {
                if (errno == EINTR)
                    continue;
                ::close(fd);
                return WorkerResult::fail(KIO::ERR_CANNOT_READ, getPath(url));
            }
            if (n == 0)
                break; // Finished

            {
                QByteArray array = QByteArray::fromRawData(buffer, n);
                data(array);
            }

            processed_size += n;
        }

        data(QByteArray());
        ::close(fd);
        processedSize(buff.st_size);

        if (decompressToFile)
            QFile(arcTempDir + file).remove();
        return WorkerResult::pass();
    }
    // send empty buffer to mark EOF
    data(QByteArray());
    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::del(QUrl const &url, bool isFile)
{
    KRFUNC;
    KRDEBUG(getPath(url));

    const auto writeSupportResult = checkWriteSupport();
    if (!writeSupportResult.success())
        return writeSupportResult;

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }
    if (newArchiveURL && !initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }

    if (delCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Deleting files from %1 archives is not supported.", arcType));
    }
    if (!findFileEntry(url)) {
        if ((arcType != "arj" && arcType != "lha") || isFile) {
            return WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, getPath(url));
        }
    }

    QString file = getPath(url).mid(getPath(arcFile->url()).length() + 1);
    if (!isFile && file.right(1) != DIR_SEPARATOR) {
        if (arcType == "zip")
            file = file + DIR_SEPARATOR;
    }
    KrLinecountingProcess proc;
    proc << delCmd << getPath(arcFile->url()) << localeEncodedString(file);
    infoMessage(i18n("Deleting %1...", url.fileName()));

    SET_KRCODEC
    proc.start();
    RESET_KRCODEC

    proc.waitForFinished();
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())) {
        return WorkerResult::fail(ERR_CANNOT_WRITE, getPath(url) + "\n\n" + proc.getErrorMsg());
    }
    //  force a refresh of archive information
    initDirDict(url, true);
    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::stat(const QUrl &url)
{
    KRFUNC;
    KRDEBUG(getPath(url));

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }
    if (newArchiveURL && !initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }

    if (listCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Accessing files is not supported with %1 archives.", arcType));
    }

    QString path = getPath(url, QUrl::StripTrailingSlash);
    QUrl newUrl = url;

    // but treat the archive itself as the archive root
    if (path == getPath(arcFile->url(), QUrl::StripTrailingSlash)) {
        newUrl.setPath(path + DIR_SEPARATOR);
        path = getPath(newUrl);
    }
    // we might be stating a real file
    if (QFileInfo::exists(path)) {
        QT_STATBUF buff;
        QT_STAT(path.toLocal8Bit().constData(), &buff);
        QString mime;
        QMimeDatabase db;
        QMimeType result = db.mimeTypeForFile(path);
        if (result.isValid())
            mime = result.name();
        statEntry(KFileItem(QUrl::fromLocalFile(path), mime, buff.st_mode).entry());
        return WorkerResult::pass();
    }
    UDSEntry *entry = findFileEntry(newUrl);
    if (entry) {
        statEntry(*entry);
        return WorkerResult::pass();
    }
    return WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, path);
}

KIO::WorkerResult kio_krarcProtocol::copy(const QUrl &url, const QUrl &dest, int, KIO::JobFlags flags)
{
    KRDEBUG("source: " << url.path() << " dest: " << dest.path());

    const auto writeSupportResult = checkWriteSupport();
    if (!writeSupportResult.success())
        return writeSupportResult;

    bool overwrite = !!(flags & KIO::Overwrite);

    // KDE HACK: opening the password dlg in copy causes error for the COPY, and further problems
    // that's why encrypted files are not allowed to copy
    if (!encrypted && dest.isLocalFile())
        do {
            if (url.fileName() != dest.fileName())
                break;

            if (QTextCodec::codecForLocale()->name() != codec->name())
                break;

            // the file exists and we don't want to overwrite
            if ((!overwrite) && (QFile(getPath(dest)).exists())) {
                return WorkerResult::fail((int)ERR_FILE_ALREADY_EXIST, QString(QFile::encodeName(getPath(dest))));
            };

            const auto setArcFileResult = setArcFile(url);
            if (!setArcFileResult.success()) {
                return setArcFileResult;
            }
            if (newArchiveURL && !initDirDict(url)) {
                return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
            }

            UDSEntry *entry = findFileEntry(url);
            if (copyCmd.isEmpty() || !entry)
                break;

            QString file = getPath(url).mid(getPath(arcFile->url()).length() + 1);

            QString destDir = getPath(dest, QUrl::StripTrailingSlash);
            if (!QDir(destDir).exists()) {
                qsizetype ndx = destDir.lastIndexOf(DIR_SEPARATOR_CHAR);
                if (ndx != -1)
                    destDir.truncate(ndx + 1);
            }

            QDir::setCurrent(destDir);

            QString escapedFilename = file;
            if (arcType == "zip") {
                // left bracket needs to be escaped
                escapedFilename.replace('[', "[[]");
            }

            KrLinecountingProcess proc;
            proc << copyCmd << getPath(arcFile->url(), QUrl::StripTrailingSlash) << escapedFilename;
            if (arcType == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
                proc.setStandardInputFile("/dev/ptmx");
            proc.setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect

            infoMessage(i18n("Unpacking %1...", url.fileName()));
            proc.start();
            proc.waitForFinished();
            if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())) {
                return WorkerResult::fail(KIO::ERR_CANNOT_WRITE, getPath(dest, QUrl::StripTrailingSlash) + "\n\n" + proc.getErrorMsg());
            }
            if (!QFileInfo::exists(getPath(dest, QUrl::StripTrailingSlash))) {
                return WorkerResult::fail(KIO::ERR_CANNOT_WRITE, getPath(dest, QUrl::StripTrailingSlash));
            }

            processedSize(KFileItem(*entry, url).size());
            QDir::setCurrent(QDir::rootPath()); /* for being able to umount devices after copying*/
            return WorkerResult::pass();
        } while (false);

    if (encrypted)
        KRDEBUG("ERROR: " << url.path() << " is encrypted.");
    if (!dest.isLocalFile())
        KRDEBUG("ERROR: " << dest.path() << " is not a local file.");

    // if the destination is an archive, avoid that get/put is tried instead - which will fail
    if (dest.scheme() == "krarc") {
        return WorkerResult::fail(ERR_UNKNOWN, i18n("Copying from archive to archive is not supported."));
    }
    // CMD_COPY is no more in KF5 - replaced with 74 value (as stated in https://invent.kde.org/frameworks/kio/-/blob/master/src/core/commands_p.h)
    return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString("kio_krarc", 74));
}

KIO::WorkerResult kio_krarcProtocol::rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags)
{
    Q_UNUSED(flags);

    KRDEBUG("renaming from: " << src.path() << " to: " << dest.path());
    KRDEBUG("command: " << arcPath);

    const auto writeSupportResult = checkWriteSupport();
    if (!writeSupportResult.success())
        return writeSupportResult;

    if (renCmd.isEmpty()) {
        return WorkerResult::fail(KIO::ERR_CANNOT_RENAME, src.fileName());
    }

    if (src.fileName() == dest.fileName()) {
        return WorkerResult::pass();
    }

    KrLinecountingProcess proc;
    proc << renCmd << arcPath << src.path().remove(arcPath + '/') << dest.path().remove(arcPath + '/');
    proc.start();
    proc.waitForFinished();

    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode())) {
        return WorkerResult::fail(KIO::ERR_CANNOT_RENAME, src.fileName());
    }

    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::listDir(const QUrl &url)
{
    KRFUNC;
    KRDEBUG(getPath(url));
    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) {
        return setArcFileResult;
    }
    if (listCmd.isEmpty()) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, i18n("Listing folders is not supported for %1 archives.", arcType));
    }
    QString path = getPath(url);
    if (path.right(1) != DIR_SEPARATOR)
        path = path + DIR_SEPARATOR;

    // it might be a real dir !
    if (QFileInfo::exists(path)) {
        if (QFileInfo(path).isDir()) {
            QUrl redir;
            redir.setPath(getPath(url));
            redirection(redir);
            return WorkerResult::pass();
        }
        // maybe it's an archive !
        return WorkerResult::fail(ERR_IS_FILE, path);
    }
    if (!initDirDict(url)) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }

    QString arcDir = path.mid(getPath(arcFile->url()).length());
    arcDir.truncate(arcDir.lastIndexOf(DIR_SEPARATOR));
    if (arcDir.right(1) != DIR_SEPARATOR)
        arcDir = arcDir + DIR_SEPARATOR;

    if (dirDict.find(arcDir) == dirDict.end()) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, getPath(url));
    }
    UDSEntryList *dirList = dirDict[arcDir];
    totalSize(dirList->size());
    listEntries(*dirList);
    return WorkerResult::pass();
}

KIO::WorkerResult kio_krarcProtocol::setArcFile(const QUrl &url)
{
    KRFUNC;
    KRDEBUG(url.fileName());
    QString path = getPath(url);
    time_t currTime = time(nullptr);
    archiveChanged = true;
    newArchiveURL = true;
    // is the file already set ?
    if (arcFile && getPath(arcFile->url(), QUrl::StripTrailingSlash) == path.left(getPath(arcFile->url(), QUrl::StripTrailingSlash).length())) {
        newArchiveURL = false;
        // Has it changed ?
        KFileItem *newArcFile = new KFileItem(arcFile->url(), QString(), arcFile->mode());
        if (metaData("Charset") != currentCharset || !newArcFile->cmp(*arcFile)) {
            currentCharset = metaData("Charset");

            codec = QTextCodec::codecForName(currentCharset.toLatin1());
            if (codec == nullptr)
                codec = QTextCodec::codecForMib(4 /* latin-1 */);

            delete arcFile;
            password.clear();
            extArcReady = false;
            arcFile = newArcFile;
        } else { // same old file
            delete newArcFile;
            archiveChanged = false;
            if (encrypted && password.isNull())
                (void)initArcParameters();
        }
    } else { // it's a new file...
        extArcReady = false;

        // new archive file means new dirDict, too
        dirDict.clear();

        if (arcFile) {
            delete arcFile;
            password.clear();
            arcFile = nullptr;
        }
        QString newPath = path;
        if (newPath.right(1) != DIR_SEPARATOR)
            newPath = newPath + DIR_SEPARATOR;
        for (qsizetype pos = 0; pos >= 0; pos = newPath.indexOf(DIR_SEPARATOR, pos + 1)) {
            QFileInfo qfi(newPath.left(pos));
            if (qfi.exists() && !qfi.isDir()) {
                QT_STATBUF stat_p;
                QT_LSTAT(newPath.left(pos).toLocal8Bit().constData(), &stat_p);
                arcFile = new KFileItem(QUrl::fromLocalFile(newPath.left(pos)), QString(), stat_p.st_mode);
                break;
            }
        }
        if (!arcFile) {
            // KRDEBUG("ERROR: " << path << " does not exist.");
            return WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toString());
        }
        currentCharset = metaData("Charset");

        codec = QTextCodec::codecForName(currentCharset.toLatin1());
        if (codec == nullptr)
            codec = QTextCodec::codecForMib(4 /* latin-1 */);
    }

    /* FIX: file change can only be detected if the timestamp between the two consequent
       changes is more than 1s. If the archive is continuously changing (check: move files
       inside the archive), krarc may erroneously think, that the archive file is unchanged,
       because the timestamp is the same as the previous one. This situation can only occur
       if the modification time equals with the current time. While this condition is true,
       we can say, that the archive is changing, so content reread is always necessary
       during that period. */
    if (archiveChanging)
        archiveChanged = true;
    archiveChanging = (currTime == (time_t)arcFile->time(KFileItem::ModificationTime).toSecsSinceEpoch());

    arcPath = getPath(arcFile->url(), QUrl::StripTrailingSlash);
    arcType = detectArchive(encrypted, arcPath);

    if (arcType == "tbz")
        arcType = "bzip2";
    else if (arcType == "tgz")
        arcType = "gzip";
    else if (arcType == "tlz")
        arcType = "lzma";
    else if (arcType == "txz")
        arcType = "xz";

    if (arcType.isEmpty()) {
        arcType = arcFile->mimetype();
        arcType = getShortTypeFromMime(arcType);
        if (arcType == "jar")
            arcType = "zip";
    }

    return initArcParameters();
}

bool kio_krarcProtocol::initDirDict(const QUrl &url, bool forced)
{
    KRFUNC;
    KRDEBUG(getPath(url));
    // set the archive location
    // if( !setArcFile(getPath(url)) ) return false;
    // no need to rescan the archive if it's not changed
    // KRDEBUG("achiveChanged: " << archiveChanged << " forced: " << forced);
    if (!archiveChanged && !forced) {
        // KRDEBUG("doing nothing.");
        return true;
    }

    extArcReady = false;

    const auto setArcFileResult = setArcFile(url);
    if (!setArcFileResult.success()) { /* if the archive was changed refresh the file information */
        return false;
    }

    // write the temp file
    KrLinecountingProcess proc;
    QTemporaryFile temp;

    // parse the temp file
    if (!temp.open()) {
        return false;
    }

    if (arcType != "bzip2" && arcType != "lzma" && arcType != "xz") {
        if (arcType == "rpm") {
            proc << listCmd << arcPath;
            proc.setStandardOutputFile(temp.fileName());
        } else {
            proc << listCmd << getPath(arcFile->url(), QUrl::StripTrailingSlash);
            proc.setStandardOutputFile(temp.fileName());
        }
        if (arcType == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
            proc.setStandardInputFile("/dev/ptmx");

        proc.setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect
        proc.start();
        proc.waitForFinished();
        if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(proc.exitCode()))
            return false;
    }
    // clear the dir dictionary

    QHashIterator<QString, KIO::UDSEntryList *> lit(dirDict);
    while (lit.hasNext())
        delete lit.next().value();
    dirDict.clear();

    // add the "/" directory
    auto *root = new UDSEntryList();
    dirDict.insert(DIR_SEPARATOR, root);
    // and the "/" UDSEntry
    UDSEntry entry;
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, ".");
    mode_t mode = parsePermString("drwxr-xr-x");
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, mode & S_IFMT); // keep file type only
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, mode & 07777); // keep permissions only

    root->append(entry);

    if (arcType == "bzip2" || arcType == "lzma" || arcType == "xz")
        abort();

    char buf[1000];
    QString line;

    int lineNo = 0;
    bool invalidLine = false;
    // the rar list is started with a ------ line.
    if (arcType == "rar" || arcType == "arj" || arcType == "lha" || arcType == "7z") {
        while (temp.readLine(buf, 1000) != -1) {
            line = decodeString(buf);
            if (line.startsWith(QLatin1String("----------")))
                break;
        }
    }
    while (temp.readLine(buf, 1000) != -1) {
        line = decodeString(buf);
        if (arcType == "rar") {
            // the rar list is ended with a ------ line.
            if (line.startsWith(QLatin1String("----------"))) {
                invalidLine = !invalidLine;
                break;
            }
            if (invalidLine)
                continue;
            else {
                if (line[0] == '*') // encrypted archives starts with '*'
                    line[0] = ' ';
            }
        }
        if (arcType == "ace") {
            // the ace list begins with a number.
            if (!line[0].isDigit())
                continue;
        }
        if (arcType == "arj") {
            // the arj list is ended with a ------ line.
            if (line.startsWith(QLatin1String("----------"))) {
                invalidLine = !invalidLine;
                continue;
            }
            if (invalidLine)
                continue;
            else {
                temp.readLine(buf, 1000);
                line = line + decodeString(buf);
                temp.readLine(buf, 1000);
                line = line + decodeString(buf);
                temp.readLine(buf, 1000);
                line = line + decodeString(buf);
            }
        }
        if (arcType == "lha" || arcType == "7z") {
            // the arj list is ended with a ------ line.
            if (line.startsWith(QLatin1String("----------")))
                break;
        }
        parseLine(lineNo++, line.trimmed());
    }
    // close and delete our file
    temp.close();

    archiveChanged = false;
    // KRDEBUG("done.");
    return true;
}

QString kio_krarcProtocol::findArcDirectory(const QUrl &url)
{
    KRFUNC;
    KRDEBUG(url.fileName());

    QString path = getPath(url);
    if (path.right(1) == DIR_SEPARATOR)
        path.truncate(path.length() - 1);

    if (!initDirDict(url)) {
        return QString();
    }
    QString arcDir = path.mid(getPath(arcFile->url()).length());
    arcDir.truncate(arcDir.lastIndexOf(DIR_SEPARATOR));
    if (arcDir.right(1) != DIR_SEPARATOR)
        arcDir = arcDir + DIR_SEPARATOR;

    return arcDir;
}

UDSEntry *kio_krarcProtocol::findFileEntry(const QUrl &url)
{
    KRFUNC;
    QString arcDir = findArcDirectory(url);
    if (arcDir.isEmpty())
        return nullptr;

    QHash<QString, KIO::UDSEntryList *>::iterator itef = dirDict.find(arcDir);
    if (itef == dirDict.end())
        return nullptr;
    UDSEntryList *dirList = itef.value();

    QString name = getPath(url);
    if (getPath(arcFile->url(), QUrl::StripTrailingSlash) == getPath(url, QUrl::StripTrailingSlash))
        name = '.'; // the '/' case
    else {
        if (name.right(1) == DIR_SEPARATOR)
            name.truncate(name.length() - 1);
        name = name.mid(name.lastIndexOf(DIR_SEPARATOR) + 1);
    }

    UDSEntryList::iterator entry;

    for (entry = dirList->begin(); entry != dirList->end(); ++entry) {
        if ((entry->contains(KIO::UDSEntry::UDS_NAME)) && (entry->stringValue(KIO::UDSEntry::UDS_NAME) == name))
            return &(*entry);
    }
    return nullptr;
}

QString kio_krarcProtocol::nextWord(QString &s, char d)
{
    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    s = s.trimmed();
    qsizetype j = s.indexOf(d, 0);
    QString temp = s.left(j); // find the leftmost word.
    s.remove(0, j);
    return temp;
}

mode_t kio_krarcProtocol::parsePermString(QString perm)
{
    KRFUNC;
    mode_t mode = 0;
    // file type
    if (perm[0] == 'd')
        mode |= S_IFDIR;
#ifndef Q_OS_WIN
    if (perm[0] == 'l')
        mode |= S_IFLNK;
#endif
    if (perm[0] == '-')
        mode |= S_IFREG;
    // owner permissions
    if (perm[1] != '-')
        mode |= S_IRUSR;
    if (perm[2] != '-')
        mode |= S_IWUSR;
    if (perm[3] != '-')
        mode |= S_IXUSR;
#ifndef Q_OS_WIN
    // group permissions
    if (perm[4] != '-')
        mode |= S_IRGRP;
    if (perm[5] != '-')
        mode |= S_IWGRP;
    if (perm[6] != '-')
        mode |= S_IXGRP;
    // other permissions
    if (perm[7] != '-')
        mode |= S_IROTH;
    if (perm[8] != '-')
        mode |= S_IWOTH;
    if (perm[9] != '-')
        mode |= S_IXOTH;
#endif
    return mode;
}

UDSEntryList *kio_krarcProtocol::addNewDir(const QString &path)
{
    KRFUNC;
    UDSEntryList *dir;

    // check if the current dir exists
    QHash<QString, KIO::UDSEntryList *>::iterator itef = dirDict.find(path);
    if (itef != dirDict.end())
        return itef.value();

    // set dir to the parent dir
    dir = addNewDir(path.left(path.lastIndexOf(DIR_SEPARATOR, -2) + 1));

    // add a new entry in the parent dir
    QString name = path.mid(path.lastIndexOf(DIR_SEPARATOR, -2) + 1);
    name = name.left(name.length() - 1);

    if (name == "." || name == "..") { // entries with these names wouldn't be displayed
        // don't translate since this is an internal error
        QString err = QString("Cannot handle path: ") + path;
        KRDEBUG("ERROR: " << err);
        exit();
    }

    UDSEntry entry;
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, name);
    mode_t mode = parsePermString("drwxr-xr-x");
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, mode & S_IFMT); // keep file type only
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, mode & 07777); // keep permissions only
    entry.fastInsert(KIO::UDSEntry::UDS_SIZE, 0);
    entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, arcFile->time(KFileItem::ModificationTime).toSecsSinceEpoch());

    dir->append(entry);

    // create a new directory entry and add it..
    dir = new UDSEntryList();
    dirDict.insert(path, dir);

    return dir;
}

void kio_krarcProtocol::parseLine(int lineNo, QString line)
{
    KRFUNC;
    UDSEntryList *dir;
    UDSEntry entry;

    QString owner;
    QString group;
    QString symlinkDest;
    QString perm;
    mode_t mode = 0666;
    size_t size = 0;
    time_t time = ::time(nullptr);
    QString fullName;

    if (arcType == "zip") {
        // permissions
        perm = nextWord(line);
        // ignore the next 2 fields
        nextWord(line);
        nextWord(line);
        // size
        size = nextWord(line).toLong();
        // ignore the next 2 fields
        nextWord(line);
        nextWord(line);
        // date & time
        QString d = nextWord(line);
        QDate qdate(d.mid(0, 4).toInt(), d.mid(4, 2).toInt(), d.mid(6, 2).toInt());
        QTime qtime(d.mid(9, 2).toInt(), d.mid(11, 2).toInt(), d.mid(13, 2).toInt());
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // full name
        fullName = nextWord(line, '\n');

        if (perm.length() != 10)
            perm = (perm.at(0) == 'd' || fullName.endsWith(DIR_SEPARATOR)) ? "drwxr-xr-x" : "-rw-r--r--";
        mode = parsePermString(perm);
    }
    if (arcType == "rar") {
        // permissions
        perm = nextWord(line);
        // size
        size = nextWord(line).toLong();
        // ignore the next 2 fields : packed size and compression ration
        nextWord(line);
        nextWord(line);
        // date & time
        QString d = nextWord(line);
        QDate qdate(d.left(4).toInt(), d.mid(5, 2).toInt(), d.mid(8, 2).toInt());
        QString t = nextWord(line);
        QTime qtime(t.mid(0, 2).toInt(), t.mid(3, 2).toInt(), 0);
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // checksum : ignored
        nextWord(line);
        // full name
        fullName = nextWord(line, '\n');

        if (perm.length() == 7) { // windows rar permission format
            bool isDir = (perm.at(1).toLower() == 'd');
            bool isReadOnly = (perm.at(2).toLower() == 'r');

            perm = isDir ? "drwxr-xr-x" : "-rw-r--r--";

            if (isReadOnly)
                perm[2] = '-';
        }

        if (perm.length() != 10)
            perm = (perm.at(0) == 'd') ? "drwxr-xr-x" : "-rw-r--r--";
        mode = parsePermString(perm);
    }
    if (arcType == "arj") {
        nextWord(line);
        // full name
        fullName = nextWord(line, '\n');
        // ignore the next 2 fields
        nextWord(line);
        nextWord(line);
        // size
        size = nextWord(line).toLong();
        // ignore the next 2 fields
        nextWord(line);
        nextWord(line);
        // date & time
        QString d = nextWord(line);
        int year = 1900 + d.mid(0, 2).toInt();
        if (year < 1930)
            year += 100;
        QDate qdate(year, d.mid(3, 2).toInt(), d.mid(6, 2).toInt());
        QString t = nextWord(line);
        QTime qtime(t.mid(0, 2).toInt(), t.mid(3, 2).toInt(), 0);
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // permissions
        perm = nextWord(line);
        if (perm.length() != 10)
            perm = (perm.at(0) == 'd') ? "drwxr-xr-x" : "-rw-r--r--";
        mode = parsePermString(perm);
    }
    if (arcType == "rpm") {
        // full name
        fullName = nextWord(line);
        // size
        size = nextWord(line).toULong();
        // date & time
        time = nextWord(line).toULong();
        // next field is md5sum, ignore it
        nextWord(line);
        // permissions
        mode = nextWord(line).toUInt(nullptr, 8);
        // Owner & Group
        owner = nextWord(line);
        group = nextWord(line);
        // symlink destination
#ifndef Q_OS_WIN
        if (S_ISLNK(mode)) {
            // ignore the next 3 fields
            nextWord(line);
            nextWord(line);
            nextWord(line);
            symlinkDest = nextWord(line);
        }
#endif
    }
    if (arcType == "gzip") {
        if (!lineNo)
            return; // ignore the first line
        // first field is uncompressed size - ignore it
        nextWord(line);
        // size
        size = nextWord(line).toULong();
        // ignore the next field
        nextWord(line);
        // full name
        fullName = nextWord(line);
        fullName = fullName.mid(fullName.lastIndexOf(DIR_SEPARATOR) + 1);
    }
    if (arcType == "lzma") {
        fullName = arcFile->name();
        if (fullName.endsWith(QLatin1String("lzma"))) {
            fullName.truncate(fullName.length() - 5);
        }
        mode = arcFile->mode();
        size = arcFile->size();
    }
    if (arcType == "xz") {
        fullName = arcFile->name();
        if (fullName.endsWith(QLatin1String("xz"))) {
            fullName.truncate(fullName.length() - 3);
        }
        mode = arcFile->mode();
        size = arcFile->size();
    }
    if (arcType == "bzip2") {
        // There is no way to list bzip2 files, so we take our information from
        // the archive itself...
        fullName = arcFile->name();
        if (fullName.endsWith(QLatin1String("bz2"))) {
            fullName.truncate(fullName.length() - 4);
        }
        mode = arcFile->mode();
        size = arcFile->size();
    }
    if (arcType == "lha") {
        // permissions
        perm = nextWord(line);
        if (perm.length() != 10)
            perm = (perm.at(0) == 'd') ? "drwxr-xr-x" : "-rw-r--r--";
        mode = parsePermString(perm);
        // ignore the next field
        nextWord(line);
        // size
        size = nextWord(line).toLong();
        // ignore the next field
        nextWord(line);
        // date & time
        int month = static_cast<int>((QString("Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec").split(',')).indexOf(nextWord(line))) + 1;
        int day = nextWord(line).toInt();
        int year = QDate::currentDate().year();
        QString third = nextWord(line);
        QTime qtime;

        if (third.contains(":"))
            qtime = QTime::fromString(third);
        else
            year = third.toInt();

        QDate qdate(year, month, day);

        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // full name
        fullName = nextWord(line, '\n');
    }
    if (arcType == "ace") {
        // date & time
        QString d = nextWord(line);
        int year = 1900 + d.mid(6, 2).toInt();
        if (year < 1930)
            year += 100;
        QDate qdate(year, d.mid(3, 2).toInt(), d.mid(0, 2).toInt());
        QString t = nextWord(line);
        QTime qtime(t.mid(0, 2).toInt(), t.mid(3, 2).toInt(), 0);
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // ignore the next field
        nextWord(line);
        // size
        size = nextWord(line).toLong();
        // ignore the next field
        nextWord(line);
        // full name
        fullName = nextWord(line, '\n');
        if (fullName[0] == '*') // encrypted archives starts with '*'
            fullName = fullName.mid(1);
    }
    if (arcType == "deb") {
        // permissions
        perm = nextWord(line);
        mode = parsePermString(perm);
        // Owner & Group
        owner = nextWord(line, DIR_SEPARATOR_CHAR);
        group = nextWord(line).mid(1);
        // size
        size = nextWord(line).toLong();
        // date & time
        QString d = nextWord(line);
        QDate qdate(d.mid(0, 4).toInt(), d.mid(5, 2).toInt(), d.mid(8, 2).toInt());
        QString t = nextWord(line);
        QTime qtime(t.mid(0, 2).toInt(), t.mid(3, 2).toInt(), 0);
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();
        // full name
        fullName = nextWord(line, '\n').mid(1);
        // if ( fullName.right( 1 ) == "/" ) return;
        if (fullName.contains("->")) {
            symlinkDest = fullName.mid(fullName.indexOf("->") + 2);
            fullName = fullName.left(fullName.indexOf("->") - 1);
        }
    }
    if (arcType == "7z") {
        // date & time
        QString d = nextWord(line);
        QDate qdate(d.mid(0, 4).toInt(), d.mid(5, 2).toInt(), d.mid(8, 2).toInt());
        QString t = nextWord(line);
        QTime qtime(t.mid(0, 2).toInt(), t.mid(3, 2).toInt(), t.mid(6, 2).toInt());
        time = QDateTime(qdate, qtime).toSecsSinceEpoch();

        // permissions
        perm = nextWord(line);
        bool isDir = (perm.at(0).toLower() == 'd');
        bool isReadOnly = (perm.at(1).toLower() == 'r');
        perm = isDir ? "drwxr-xr-x" : "-rw-r--r--";
        if (isReadOnly)
            perm[2] = '-';

        mode = parsePermString(perm);

        // size
        size = nextWord(line).toLong();

        // ignore the next 15 characters
        line = line.mid(15);

        // full name
        fullName = nextWord(line, '\n');
    }

    if (fullName.right(1) == DIR_SEPARATOR)
        fullName = fullName.left(fullName.length() - 1);
    if (!fullName.startsWith(DIR_SEPARATOR))
        fullName = DIR_SEPARATOR + fullName;
    QString path = fullName.left(fullName.lastIndexOf(DIR_SEPARATOR) + 1);
    // set/create the directory UDSEntryList
    QHash<QString, KIO::UDSEntryList *>::iterator itef = dirDict.find(path);
    if (itef == dirDict.end())
        dir = addNewDir(path);
    else
        dir = itef.value();

    QString name = fullName.mid(fullName.lastIndexOf(DIR_SEPARATOR) + 1);
    // file name
    entry.fastInsert(KIO::UDSEntry::UDS_NAME, name);
    // file type
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, mode & S_IFMT); // keep file type only
    // file permissions
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, mode & 07777); // keep permissions only
    // file size
    entry.fastInsert(KIO::UDSEntry::UDS_SIZE, size);
    // modification time
    entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, time);
    // link destination
    if (!symlinkDest.isEmpty()) {
        entry.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, symlinkDest);
    }
    if (S_ISDIR(mode)) {
        fullName = fullName + DIR_SEPARATOR;
        if (dirDict.find(fullName) == dirDict.end())
            dirDict.insert(fullName, new UDSEntryList());
        else {
            // try to overwrite an existing entry
            UDSEntryList::iterator entryIt;

            for (entryIt = dir->begin(); entryIt != dir->end(); ++entryIt) {
                if (entryIt->contains(KIO::UDSEntry::UDS_NAME) && entryIt->stringValue(KIO::UDSEntry::UDS_NAME) == name) {
                    entryIt->fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, time);
                    entryIt->fastInsert(KIO::UDSEntry::UDS_ACCESS, mode);
                    return;
                }
            }
            return; // there is already an entry for this directory
        }
    }

    // multi volume archives can add a file twice, use only one
    UDSEntryList::iterator dirEntryIt;

    for (dirEntryIt = dir->begin(); dirEntryIt != dir->end(); ++dirEntryIt)
        if (dirEntryIt->contains(KIO::UDSEntry::UDS_NAME) && dirEntryIt->stringValue(KIO::UDSEntry::UDS_NAME) == name)
            return;

    dir->append(entry);
}

KIO::WorkerResult kio_krarcProtocol::initArcParameters()
{
    KRFUNC;
    KRDEBUG("arcType: " << arcType);

    noencoding = false;

    cmd.clear();
    listCmd = QStringList();
    getCmd = QStringList();
    copyCmd = QStringList();
    delCmd = QStringList();
    putCmd = QStringList();
    renCmd = QStringList();

    if (arcType == "zip") {
        noencoding = true;
        cmd = fullPathName("unzip");
        listCmd << fullPathName("unzip") << "-ZTs-z-t-h";
        getCmd << fullPathName("unzip") << "-p";
        copyCmd << fullPathName("unzip") << "-jo";

        if (QStandardPaths::findExecutable(QStringLiteral("zip")).isEmpty()) {
            delCmd = QStringList();
            putCmd = QStringList();
        } else {
            delCmd << fullPathName("zip") << "-d";
            putCmd << fullPathName("zip") << "-ry";
        }

        QString a7zExecutable = find7zExecutable();
        if (!a7zExecutable.isEmpty()) {
            renCmd << a7zExecutable << "rn";
        }

        if (!getPassword().isEmpty()) {
            getCmd << "-P" << password;
            copyCmd << "-P" << password;
            putCmd << "-P" << password;
        }
    } else if (arcType == "rar") {
        noencoding = true;
        if (QStandardPaths::findExecutable(QStringLiteral("rar")).isEmpty()) {
            cmd = fullPathName("unrar");
            listCmd << fullPathName("unrar") << "-c-"
                    << "-v"
                    << "v";
            getCmd << fullPathName("unrar") << "p"
                   << "-ierr"
                   << "-idp"
                   << "-c-"
                   << "-y";
            copyCmd << fullPathName("unrar") << "e"
                    << "-y";
            delCmd = QStringList();
            putCmd = QStringList();
        } else {
            cmd = fullPathName("rar");
            listCmd << fullPathName("rar") << "-c-"
                    << "-v"
                    << "v";
            getCmd << fullPathName("rar") << "p"
                   << "-ierr"
                   << "-idp"
                   << "-c-"
                   << "-y";
            copyCmd << fullPathName("rar") << "e"
                    << "-y";
            delCmd << fullPathName("rar") << "d";
            putCmd << fullPathName("rar") << "-r"
                   << "a";
        }
        if (!getPassword().isEmpty()) {
            getCmd << QString("-p%1").arg(password);
            listCmd << QString("-p%1").arg(password);
            copyCmd << QString("-p%1").arg(password);
            if (!putCmd.isEmpty()) {
                putCmd << QString("-p%1").arg(password);
                delCmd << QString("-p%1").arg(password);
            }
        }
    } else if (arcType == "rpm") {
        cmd = fullPathName("rpm");
        listCmd << fullPathName("rpm") << "--dump"
                << "-lpq";
        getCmd << fullPathName("cpio") << "--force-local"
               << "--no-absolute-filenames"
               << "-iuvdF";
        delCmd = QStringList();
        putCmd = QStringList();
        copyCmd = QStringList();
    } else if (arcType == "gzip") {
        cmd = fullPathName("gzip");
        listCmd << fullPathName("gzip") << "-l";
        getCmd << fullPathName("gzip") << "-dc";
        copyCmd = QStringList();
        delCmd = QStringList();
        putCmd = QStringList();
    } else if (arcType == "bzip2") {
        cmd = fullPathName("bzip2");
        listCmd << fullPathName("bzip2");
        getCmd << fullPathName("bzip2") << "-dc";
        copyCmd = QStringList();
        delCmd = QStringList();
        putCmd = QStringList();
    } else if (arcType == "lzma") {
        cmd = fullPathName("lzma");
        listCmd << fullPathName("lzma");
        getCmd << fullPathName("lzma") << "-dc";
        copyCmd = QStringList();
        delCmd = QStringList();
        putCmd = QStringList();
    } else if (arcType == "xz") {
        cmd = fullPathName("xz");
        listCmd << fullPathName("xz");
        getCmd << fullPathName("xz") << "-dc";
        copyCmd = QStringList();
        delCmd = QStringList();
        putCmd = QStringList();
    } else if (arcType == "arj") {
        cmd = fullPathName("arj");
        listCmd << fullPathName("arj") << "v"
                << "-y"
                << "-v";
        getCmd << fullPathName("arj") << "-jyov"
               << "-v"
               << "e";
        copyCmd << fullPathName("arj") << "-jyov"
                << "-v"
                << "e";
        delCmd << fullPathName("arj") << "d";
        putCmd << fullPathName("arj") << "-r"
               << "a";
        if (!getPassword().isEmpty()) {
            getCmd << QString("-g%1").arg(password);
            copyCmd << QString("-g%1").arg(password);
            putCmd << QString("-g%1").arg(password);
        }
    } else if (arcType == "lha") {
        cmd = fullPathName("lha");
        listCmd << fullPathName("lha") << "l";
        getCmd << fullPathName("lha") << "pq";
        copyCmd << fullPathName("lha") << "eif";
        delCmd << fullPathName("lha") << "d";
        putCmd << fullPathName("lha") << "a";
    } else if (arcType == "ace") {
        cmd = fullPathName("unace");
        listCmd << fullPathName("unace") << "v";
        getCmd << fullPathName("unace") << "e"
               << "-o";
        copyCmd << fullPathName("unace") << "e"
                << "-o";
        delCmd = QStringList();
        putCmd = QStringList();
        if (!getPassword().isEmpty()) {
            getCmd << QString("-p%1").arg(password);
            copyCmd << QString("-p%1").arg(password);
        }
    } else if (arcType == "deb") {
        cmd = fullPathName("dpkg");
        listCmd << fullPathName("dpkg") << "-c";
        getCmd << fullPathName("tar") << "xvf";
        copyCmd = QStringList();
        delCmd = QStringList();
        putCmd = QStringList();
    } else if (arcType == "7z") {
        noencoding = true;
        cmd = find7zExecutable();
        if (cmd.isEmpty()) {
            return WorkerResult::fail(KIO::ERR_CANNOT_LAUNCH_PROCESS, {});
        }

        listCmd << cmd << "l"
                << "-y";
        getCmd << cmd << "e"
               << "-y";
        copyCmd << cmd << "e"
                << "-y";
        delCmd << cmd << "d"
               << "-y";
        putCmd << cmd << "a"
               << "-y";
        renCmd << cmd << "rn";
        if (!getPassword().isEmpty()) {
            getCmd << QString("-p%1").arg(password);
            listCmd << QString("-p%1").arg(password);
            copyCmd << QString("-p%1").arg(password);
            if (!putCmd.isEmpty()) {
                putCmd << QString("-p%1").arg(password);
                delCmd << QString("-p%1").arg(password);
            }
        }
    }
    // checking if it's an absolute path
#ifdef Q_OS_WIN
    if (cmd.length() > 2 && cmd[0].isLetter() && cmd[1] == ':')
        return WorkerResult::pass();

#else
    if (cmd.startsWith(DIR_SEPARATOR))
        return WorkerResult::pass();

#endif
    if (QStandardPaths::findExecutable(cmd).isEmpty()) {
        KRDEBUG("Failed to find cmd: " << cmd);
        return WorkerResult::fail(KIO::ERR_CANNOT_LAUNCH_PROCESS, cmd + i18n("\nMake sure that the %1 binary is installed properly on your system.", cmd));
    }
    return WorkerResult::pass();
}

bool kio_krarcProtocol::checkStatus(int exitCode)
{
    KRFUNC;
    KRDEBUG(exitCode);
    return KrArcBaseManager::checkStatus(arcType, exitCode);
}

void kio_krarcProtocol::checkIf7zIsEncrypted(bool &encrypted, QString fileName)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `KrArcHandler::checkIf7zIsEncrypted()`

    KRFUNC;
    if (encryptedArchPath == fileName)
        encrypted = true;
    else { // we try to find whether the 7z archive is encrypted
        // this is hard as the headers are also compressed
        QString a7zExecutable = find7zExecutable();
        if (a7zExecutable.isEmpty()) {
            return;
        }

        lastData = encryptedArchPath = "";

        KrLinecountingProcess proc;
        // Note: That command uses information given in a comment from
        // https://stackoverflow.com/questions/5248572/how-do-i-know-if-7zip-used-aes256
        proc << a7zExecutable << "l"
             << "-slt" << fileName;
        connect(&proc, &KrLinecountingProcess::newOutputData, this, &kio_krarcProtocol::check7zOutputForPassword);
        proc.start();
        proc.waitForFinished();
        encrypted = this->encrypted;

        if (encrypted)
            encryptedArchPath = fileName;
    }
}

void kio_krarcProtocol::check7zOutputForPassword(KProcess *proc, QByteArray &buf)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `Kr7zEncryptionChecker::receivedOutput()`

    KRFUNC;
    QString data = QString(buf);

    QString checkable = lastData + data;

    QStringList lines = checkable.split('\n');
    lastData = lines[lines.count() - 1];
    for (int i = 0; i != lines.count(); i++) {
        QString line = lines[i].trimmed().toLower();
        qsizetype ndx = line.indexOf("listing"); // Reminder: Lower-case letters are used
        if (ndx >= 0)
            line.truncate(ndx);
        if (line.isEmpty())
            continue;

        if ((line.contains("password") && line.contains("enter")) || line == QStringLiteral("encrypted = +")) {
            KRDEBUG("Encrypted 7z archive found!");
            encrypted = true;
            proc->kill();
            return;
        }
    }
}

void kio_krarcProtocol::invalidatePassword()
{
    KRFUNC;
    KRDEBUG(getPath(arcFile->url(), QUrl::StripTrailingSlash) + DIR_SEPARATOR);

    if (!encrypted)
        return;

    KIO::AuthInfo authInfo;
    authInfo.caption = i18n("Krarc Password Dialog");
    authInfo.username = "archive";
    authInfo.readOnly = true;
    authInfo.keepPassword = true;
    authInfo.verifyPath = true;
    QString fileName = getPath(arcFile->url(), QUrl::StripTrailingSlash);
    authInfo.url = QUrl::fromLocalFile(ROOT_DIR);
    authInfo.url.setHost(fileName /*.replace('/','_')*/);
    authInfo.url.setScheme("krarc");

    password.clear();

    cacheAuthentication(authInfo);
}

QString kio_krarcProtocol::getPassword()
{
    KRFUNC;
    KRDEBUG("Encrypted: " << encrypted);

    if (!password.isNull())
        return password;
    if (!encrypted)
        return (password = "");

    KIO::AuthInfo authInfo;
    authInfo.caption = i18n("Krarc Password Dialog");
    authInfo.username = "archive";
    authInfo.readOnly = true;
    authInfo.keepPassword = true;
    authInfo.verifyPath = true;
    QString fileName = getPath(arcFile->url(), QUrl::StripTrailingSlash);
    authInfo.url = QUrl::fromLocalFile(ROOT_DIR);
    authInfo.url.setHost(fileName /*.replace('/','_')*/);
    authInfo.url.setScheme("krarc");

    if (checkCachedAuthentication(authInfo) && !authInfo.password.isNull()) {
        // KRDEBUG(authInfo.password);
        return (password = authInfo.password);
    }

    authInfo.password.clear();

    int errCode = openPasswordDialog(authInfo, i18n("Accessing the file requires a password."));
    if (!errCode && !authInfo.password.isNull()) {
        // KRDEBUG(authInfo.password);
        return (password = authInfo.password);
    } else {
        password.clear();
    }

    // KRDEBUG(password);
    return password;
}

QString kio_krarcProtocol::localeEncodedString(QString str)
{
    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    if (noencoding)
        return str;

    QByteArray array = codec->fromUnicode(str);

    // encoding the byte array to QString, mapping 0x0000-0x00FF to 0xE000-0xE0FF
    // see KrArcCodec for more explanation
    qsizetype size = array.size();
    QString result;

    const char *data = array.constData();
    for (int i = 0; i != size; i++) {
        unsigned int ch = (((int)data[i]) & 0xFF) + 0xE000; // user defined character
        result.append(QChar(ch));
    }
    return result;
}

QByteArray kio_krarcProtocol::encodeString(const QString &str)
{
    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    if (noencoding)
        return QTextCodec::codecForLocale()->fromUnicode(str);
    return codec->fromUnicode(str);
}

QString kio_krarcProtocol::decodeString(char *buf)
{
    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    if (noencoding)
        return QTextCodec::codecForLocale()->toUnicode(buf);
    return codec->toUnicode(buf);
}

QString kio_krarcProtocol::getPath(const QUrl &url, QUrl::FormattingOptions options)
{
    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    QString path = url.adjusted(options).path();
    REPLACE_DIR_SEP2(path);

#ifdef Q_OS_WIN
    if (path.startsWith(DIR_SEPARATOR)) {
        int p = 1;
        while (p < path.length() && path[p] == DIR_SEPARATOR_CHAR)
            p++;
        /* /C:/Folder */
        if (p + 2 <= path.length() && path[p].isLetter() && path[p + 1] == ':') {
            path = path.mid(p);
        }
    }
#endif
    return path;
}

#endif // KRARC_ENABLED

#include "krarc.moc"
