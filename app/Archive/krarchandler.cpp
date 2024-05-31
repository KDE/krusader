/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krarchandler.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QFile>
// QtWidgets
#include <QApplication>

#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>
#include <KProtocolManager>
#include <KSharedConfig>
#include <KWallet>
#include <utility>

#include "../../plugins/krarc/krlinecountingprocess.h"
#include "../Dialogs/krpleasewait.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "kr7zencryptionchecker.h"

static QStringList arcProtocols = QString("tar;bzip;bzip2;lzma;xz;gzip;krarc;zip").split(';');

QMap<QString, QString> *KrArcHandler::slaveMap = nullptr;
KWallet::Wallet *KrArcHandler::wallet = nullptr;

KrArcHandler::KrArcHandler(QObject *parent)
    : QObject(parent)
{
    // Reminder: If a mime type is added/removed/modified in that
    // member function, it's important to research if the type has to
    // be added/removed/modified in the `krarc.protocol` file, or
    // in `KrArcBaseManager::getShortTypeFromMime(const QString &mime)`

    // Hard-code these proven mimetypes openable by krarc protocol.
    // They cannot be listed in krarc.protocol itself
    // because it would baffle other file managers (like Dolphin).
    krarcArchiveMimetypes = {
        QStringLiteral("application/x-deb"),
        QStringLiteral("application/x-debian-package"),
        QStringLiteral("application/vnd.debian.binary-package"),
        QStringLiteral("application/x-java-archive"),
        QStringLiteral("application/x-rpm"),
        QStringLiteral("application/x-source-rpm"),
        QStringLiteral("application/vnd.oasis.opendocument.chart"),
        QStringLiteral("application/vnd.oasis.opendocument.database"),
        QStringLiteral("application/vnd.oasis.opendocument.formula"),
        QStringLiteral("application/vnd.oasis.opendocument.graphics"),
        QStringLiteral("application/vnd.oasis.opendocument.presentation"),
        QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"),
        QStringLiteral("application/vnd.oasis.opendocument.text"),
        QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
        QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
        QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
        QStringLiteral("application/x-cbz"),
        QStringLiteral("application/vnd.comicbook+zip"),
        QStringLiteral("application/x-cbr"),
        QStringLiteral("application/vnd.comicbook-rar"),
        QStringLiteral("application/epub+zip"),
        QStringLiteral("application/x-webarchive"),
        QStringLiteral("application/x-plasma"),
        QStringLiteral("application/vnd.rar"),
    };

#ifdef KRARC_QUERY_ENABLED
    const auto mimetypes = KProtocolInfo::archiveMimetypes("krarc");
    for (const auto &mimetype : mimetypes)
        krarcArchiveMimetypes.insert(mimetype);
#endif
}

QStringList KrArcHandler::supportedPackers()
{
    QStringList packers;

    // we will simply try to find the packers here..
    if (KrServices::cmdExist("tar"))
        packers.append("tar");
    if (KrServices::cmdExist("gzip"))
        packers.append("gzip");
    if (KrServices::cmdExist("bzip2"))
        packers.append("bzip2");
    if (KrServices::cmdExist("lzma"))
        packers.append("lzma");
    if (KrServices::cmdExist("xz"))
        packers.append("xz");
    if (KrServices::cmdExist("unzip"))
        packers.append("unzip");
    if (KrServices::cmdExist("zip"))
        packers.append("zip");
    if (KrServices::cmdExist("zip"))
        packers.append("cbz");
    if (KrServices::cmdExist("lha"))
        packers.append("lha");
    if (KrServices::cmdExist("cpio"))
        packers.append("cpio");
    if (KrServices::cmdExist("unrar"))
        packers.append("unrar");
    if (KrServices::cmdExist("rar"))
        packers.append("rar");
    if (KrServices::cmdExist("rar"))
        packers.append("cbr");
    if (KrServices::cmdExist("arj"))
        packers.append("arj");
    if (KrServices::cmdExist("unarj"))
        packers.append("unarj");
    if (KrServices::cmdExist("unace"))
        packers.append("unace");
    if (KrServices::cmdExist("dpkg"))
        packers.append("dpkg");
    if (KrServices::cmdExist("7z") || KrServices::cmdExist("7za"))
        packers.append("7z");
    if (KrServices::cmdExist("rpm") && KrServices::cmdExist("rpm2cpio"))
        packers.append("rpm");
    // qDebug() << "Supported Packers:";
    // QStringList::Iterator it;
    // for( it = packers.begin(); it != packers.end(); ++it )
    // qDebug() << *it;

    return packers;
}

bool KrArcHandler::arcSupported(QString type)
{
    // lst will contain the supported unpacker list...
    const KConfigGroup group(krConfig, "Archives");
    const QStringList lst = group.readEntry("Supported Packers", QStringList());

    // Let's notice that in some cases the QString `type` that arrives here
    // represents a mimetype, and in some other cases it represents
    // a short identifier.
    // If `type` is not a short identifier then it's supposed that `type` is a mime type
    if (type.length() > maxLenType) {
        type = getShortTypeFromMime(type);
    }

    // clang-format off
    return (type == "zip" && lst.contains("unzip"))
           || (type == "tar" && lst.contains("tar"))
           || (type == "tbz" && lst.contains("tar"))
           || (type == "tgz" && lst.contains("tar"))
           || (type == "tlz" && lst.contains("tar"))
           || (type == "txz" && lst.contains("tar"))
           || (type == "tarz" && lst.contains("tar"))
           || (type == "gzip" && lst.contains("gzip"))
           || (type == "bzip2" && lst.contains("bzip2"))
           || (type == "lzma" && lst.contains("lzma"))
           || (type == "xz" && lst.contains("xz"))
           || (type == "lha" && lst.contains("lha"))
           || (type == "ace" && lst.contains("unace"))
           || (type == "rpm" && lst.contains("cpio"))
           || (type == "cpio" && lst.contains("cpio"))
           || (type == "rar" && (lst.contains("unrar") || lst.contains("rar")))
           || (type == "arj" && (lst.contains("unarj") || lst.contains("arj")))
           || (type == "deb" && (lst.contains("dpkg") && lst.contains("tar")))
           || (type == "7z" && lst.contains("7z"));
    // clang-format on
}

qulonglong KrArcHandler::arcFileCount(const QString &archive, const QString &type, const QString &password, KrArcObserver *observer)
{
    int divideWith = 1;

    // first check if supported
    if (!arcSupported(type))
        return 0;

    // bzip2, gzip, etc. archives contain only one file
    if (type == "bzip2" || type == "gzip" || type == "lzma" || type == "xz")
        return 1L;

    // set the right lister to do the job
    QStringList lister;

    if (type == "zip")
        lister << KrServices::fullPathName("unzip") << "-ZTs";
    else if (type == "tar")
        lister << KrServices::fullPathName("tar") << "-tvf";
    else if (type == "tgz")
        lister << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tarz")
        lister << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tbz")
        lister << KrServices::fullPathName("tar") << "-tjvf";
    else if (type == "tlz")
        lister << KrServices::fullPathName("tar") << "--lzma"
               << "-tvf";
    else if (type == "txz")
        lister << KrServices::fullPathName("tar") << "--xz"
               << "-tvf";
    else if (type == "lha")
        lister << KrServices::fullPathName("lha") << "l";
    else if (type == "rar")
        lister << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "l"
               << "-v";
    else if (type == "ace")
        lister << KrServices::fullPathName("unace") << "l";
    else if (type == "arj") {
        if (KrServices::cmdExist("arj")) {
            lister << KrServices::fullPathName("arj") << "v"
                   << "-y"
                   << "-v";
            divideWith = 4;
        } else {
            lister << KrServices::fullPathName("unarj") << "l";
        }
    } else if (type == "rpm")
        lister << KrServices::fullPathName("rpm") << "--dump"
               << "-lpq";
    else if (type == "deb")
        lister << KrServices::fullPathName("dpkg") << "-c";
    else if (type == "7z")
        lister << find7zExecutable() << "-y"
               << "l";
    else
        return 0L;

    if (!password.isNull()) {
        if (type == "arj")
            lister << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            lister << QString("-p%1").arg(password);
    }

    // tell the user to wait
    observer->subJobStarted(i18n("Counting files in archive"), 0);

    // count the number of files in the archive
    qulonglong count = 1;
    KProcess list;
    list << lister << archive;
    if (type == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
        list.setStandardInputFile("/dev/ptmx");
    list.setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect
    list.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (list.waitForStarted())
        while (list.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                list.kill();
        }; // busy wait - need to find something better...

    observer->subJobStopped();

    if (list.exitStatus() != QProcess::NormalExit || !checkStatus(type, list.exitCode())) {
        observer->detailedError(i18n("Failed to list the content of the archive (%1).", archive), QString::fromLocal8Bit(list.readAllStandardError()));
        return 0;
    }

    count = list.readAllStandardOutput().count('\n');

    // make sure you call stopWait after this function return...
    //   observer->subJobStopped();

    return count / divideWith;
}

bool KrArcHandler::unpack(QString archive, const QString &type, const QString &password, const QString &dest, KrArcObserver *observer)
{
    KConfigGroup group(krConfig, "Archives");
    if (group.readEntry("Test Before Unpack", _TestBeforeUnpack)) {
        // test first - or be sorry later...
        if (type != "rpm" && type != "deb" && !test(archive, type, password, observer, 0)) {
            observer->error(i18n("Failed to unpack %1.", archive));
            return false;
        }
    }

    // count the files in the archive
    qulonglong count = arcFileCount(archive, type, password, observer);
    if (count == 0)
        return false; // not supported
    if (count == 1)
        count = 0;

    // choose the right packer for the job
    QString cpioName;
    QStringList packer;

    // set the right packer to do the job
    if (type == "zip")
        packer << KrServices::fullPathName("unzip") << "-o";
    else if (type == "tar")
        packer << KrServices::fullPathName("tar") << "-xvf";
    else if (type == "tgz")
        packer << KrServices::fullPathName("tar") << "-xvzf";
    else if (type == "tarz")
        packer << KrServices::fullPathName("tar") << "-xvzf";
    else if (type == "tbz")
        packer << KrServices::fullPathName("tar") << "-xjvf";
    else if (type == "tlz")
        packer << KrServices::fullPathName("tar") << "--lzma"
               << "-xvf";
    else if (type == "txz")
        packer << KrServices::fullPathName("tar") << "--xz"
               << "-xvf";
    else if (type == "gzip")
        packer << KrServices::fullPathName("gzip") << "-cd";
    else if (type == "bzip2")
        packer << KrServices::fullPathName("bzip2") << "-cdk";
    else if (type == "lzma")
        packer << KrServices::fullPathName("lzma") << "-cdk";
    else if (type == "xz")
        packer << KrServices::fullPathName("xz") << "-cdk";
    else if (type == "lha")
        packer << KrServices::fullPathName("lha") << "xf";
    else if (type == "rar")
        packer << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "-y"
               << "x";
    else if (type == "ace")
        packer << KrServices::fullPathName("unace") << "x";
    else if (type == "arj") {
        if (KrServices::cmdExist("arj"))
            packer << KrServices::fullPathName("arj") << "-y"
                   << "-v"
                   << "x";
        else
            packer << KrServices::fullPathName("unarj") << "x";
    } else if (type == "7z")
        packer << find7zExecutable() << "-y"
               << "x";
    else if (type == "rpm") {
        // TODO use QTemporaryFile (setAutoRemove(false) when asynchrone)
        cpioName = QDir::tempPath() + QStringLiteral("/contents.cpio");

        KrLinecountingProcess cpio;
        cpio << KrServices::fullPathName("rpm2cpio") << archive;
        cpio.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
        cpio.start();
        if (!cpio.waitForFinished() || cpio.exitStatus() != QProcess::NormalExit || !checkStatus("cpio", cpio.exitCode())) {
            observer->detailedError(i18n("Failed to convert rpm (%1) to cpio.", archive), cpio.getErrorMsg());
            return 0;
        }

        archive = cpioName;
        packer << KrServices::fullPathName("cpio") << "--force-local"
               << "--no-absolute-filenames"
               << "-iuvdF";
    } else if (type == "deb") {
        // TODO use QTemporaryFile (setAutoRemove(false) when asynchrone)
        cpioName = QDir::tempPath() + QStringLiteral("/contents.tar");

        KrLinecountingProcess dpkg;
        dpkg << KrServices::fullPathName("dpkg") << "--fsys-tarfile" << archive;
        dpkg.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
        dpkg.start();
        if (!dpkg.waitForFinished() || dpkg.exitStatus() != QProcess::NormalExit || !checkStatus("deb", dpkg.exitCode())) {
            observer->detailedError(i18n("Failed to convert deb (%1) to tar.", archive), dpkg.getErrorMsg());
            return 0;
        }

        archive = cpioName;
        packer << KrServices::fullPathName("tar") << "xvf";
    } else
        return false;

    if (!password.isNull()) {
        if (type == "zip")
            packer << "-P" << password;
        if (type == "arj")
            packer << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            packer << QString("-p%1").arg(password);
    }

    // unpack the files
    KrLinecountingProcess proc;
    proc << packer << archive;
    if (type == "bzip2" || type == "gzip" || type == "lzma" || type == "xz") {
        QString arcname = archive.mid(archive.lastIndexOf("/") + 1);
        if (arcname.contains("."))
            arcname = arcname.left(arcname.lastIndexOf("."));
        proc.setStandardOutputFile(dest + '/' + arcname);
    }
    if (type == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
        proc.setStandardInputFile("/dev/ptmx");

    proc.setWorkingDirectory(dest);

    // tell the user to wait
    observer->subJobStarted(i18n("Unpacking File(s)"), count);
    if (count != 0) {
        connect(&proc, &KrLinecountingProcess::newOutputLines, observer, &KrArcObserver::incrementProgress);
        if (type == "rpm")
            connect(&proc, &KrLinecountingProcess::newErrorLines, observer, &KrArcObserver::incrementProgress);
    }

    // start the unpacking process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }; // busy wait - need to find something better...
    observer->subJobStopped();

    if (!cpioName.isEmpty())
        QFile(cpioName).remove(); /* remove the cpio file */

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode())) {
        observer->detailedError(i18n("Failed to unpack %1.", archive), observer->wasCancelled() ? i18n("User cancelled.") : proc.getErrorMsg());
        return false;
    }
    return true; // SUCCESS
}

bool KrArcHandler::test(const QString &archive, const QString &type, const QString &password, KrArcObserver *observer, qulonglong count)
{
    // choose the right packer for the job
    QStringList packer;

    // set the right packer to do the job
    if (type == "zip")
        packer << KrServices::fullPathName("unzip") << "-t";
    else if (type == "tar")
        packer << KrServices::fullPathName("tar") << "-tvf";
    else if (type == "tgz")
        packer << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tarz")
        packer << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tbz")
        packer << KrServices::fullPathName("tar") << "-tjvf";
    else if (type == "tlz")
        packer << KrServices::fullPathName("tar") << "--lzma"
               << "-tvf";
    else if (type == "txz")
        packer << KrServices::fullPathName("tar") << "--xz"
               << "-tvf";
    else if (type == "gzip")
        packer << KrServices::fullPathName("gzip") << "-tv";
    else if (type == "bzip2")
        packer << KrServices::fullPathName("bzip2") << "-tv";
    else if (type == "lzma")
        packer << KrServices::fullPathName("lzma") << "-tv";
    else if (type == "xz")
        packer << KrServices::fullPathName("xz") << "-tv";
    else if (type == "rar")
        packer << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "t";
    else if (type == "ace")
        packer << KrServices::fullPathName("unace") << "t";
    else if (type == "lha")
        packer << KrServices::fullPathName("lha") << "t";
    else if (type == "arj")
        packer << KrServices::fullPathName(KrServices::cmdExist("arj") ? "arj" : "unarj") << "t";
    else if (type == "cpio")
        packer << KrServices::fullPathName("cpio") << "--only-verify-crc"
               << "-tvF";
    else if (type == "7z")
        packer << find7zExecutable() << "-y"
               << "t";
    else
        return false;

    if (!password.isNull()) {
        if (type == "zip")
            packer << "-P" << password;
        if (type == "arj")
            packer << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            packer << QString("-p%1").arg(password);
    }

    // unpack the files
    KrLinecountingProcess proc;
    proc << packer << archive;

    if (type == "ace" && QFile("/dev/ptmx").exists()) // Don't remove, unace crashes if missing!!!
        proc.setStandardInputFile("/dev/ptmx");

    // tell the user to wait
    observer->subJobStarted(i18n("Testing Archive"), count);
    if (count != 0)
        connect(&proc, &KrLinecountingProcess::newOutputLines, observer, &KrArcObserver::incrementProgress);

    // start the unpacking process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }; // busy wait - need to find something better...
    observer->subJobStopped();

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode()))
        return false;

    return true; // SUCCESS
}

bool KrArcHandler::pack(QStringList fileNames, QString type, const QString &dest, qulonglong count, QMap<QString, QString> extraProps, KrArcObserver *observer)
{
    // set the right packer to do the job
    QStringList packer;

    if (type == "zip") {
        packer << KrServices::fullPathName("zip") << "-ry";
    } else if (type == "cbz") {
        packer << KrServices::fullPathName("zip") << "-ry";
        type = "zip";
    } else if (type == "tar") {
        packer << KrServices::fullPathName("tar") << "-cvf";
    } else if (type == "tar.gz") {
        packer << KrServices::fullPathName("tar") << "-cvzf";
        type = "tgz";
    } else if (type == "tar.bz2") {
        packer << KrServices::fullPathName("tar") << "-cvjf";
        type = "tbz";
    } else if (type == "tar.lzma") {
        packer << KrServices::fullPathName("tar") << "--lzma"
               << "-cvf";
        type = "tlz";
    } else if (type == "tar.xz") {
        packer << KrServices::fullPathName("tar") << "--xz"
               << "-cvf";
        type = "txz";
    } else if (type == "rar") {
        packer << KrServices::fullPathName("rar") << "-r"
               << "a";
    } else if (type == "cbr") {
        packer << KrServices::fullPathName("rar") << "-r"
               << "a";
        type = "rar";
    } else if (type == "lha") {
        packer << KrServices::fullPathName("lha") << "a";
    } else if (type == "arj") {
        packer << KrServices::fullPathName("arj") << "-r"
               << "-y"
               << "a";
    } else if (type == "7z") {
        packer << find7zExecutable() << "-y"
               << "a";
    } else
        return false;

    QString password;

    if (extraProps.count("Password") > 0) {
        password = extraProps["Password"];

        if (!password.isNull()) {
            if (type == "zip")
                packer << "-P" << password;
            else if (type == "arj")
                packer << QString("-g%1").arg(password);
            else if (type == "ace" || type == "7z")
                packer << QString("-p%1").arg(password);
            else if (type == "rar") {
                if (extraProps.count("EncryptHeaders") > 0)
                    packer << QString("-hp%1").arg(password);
                else
                    packer << QString("-p%1").arg(password);
            } else
                password.clear();
        }
    }

    if (extraProps.count("VolumeSize") > 0) {
        QString sizeStr = extraProps["VolumeSize"];
        KIO::filesize_t size = sizeStr.toLongLong();

        if (size >= 10000) {
            if (type == "arj" || type == "rar")
                packer << QString("-v%1b").arg(sizeStr);
        }
    }

    if (extraProps.count("CompressionLevel") > 0) {
        int level = extraProps["CompressionLevel"].toInt() - 1;
        if (level < 0)
            level = 0;
        if (level > 8)
            level = 8;

        if (type == "rar") {
            static const int rarLevels[] = {0, 1, 2, 2, 3, 3, 4, 4, 5};
            packer << QString("-m%1").arg(rarLevels[level]);
        } else if (type == "arj") {
            static const int arjLevels[] = {0, 4, 4, 3, 3, 2, 2, 1, 1};
            packer << QString("-m%1").arg(arjLevels[level]);
        } else if (type == "zip") {
            static const int zipLevels[] = {0, 1, 2, 4, 5, 6, 7, 8, 9};
            packer << QString("-%1").arg(zipLevels[level]);
        } else if (type == "7z") {
            static const int sevenZipLevels[] = {0, 1, 2, 4, 5, 6, 7, 8, 9};
            packer << QString("-mx%1").arg(sevenZipLevels[level]);
        }
    }

    if (extraProps.count("CommandLineSwitches") > 0)
        packer << QString("%1").arg(extraProps["CommandLineSwitches"]);

    // prepare to pack
    KrLinecountingProcess proc;
    proc << packer << dest;

    for (auto &fileName : fileNames) {
        proc << fileName;
    }

    // tell the user to wait
    observer->subJobStarted(i18n("Packing File(s)"), count);
    if (count != 0)
        connect(&proc, &KrLinecountingProcess::newOutputLines, observer, &KrArcObserver::incrementProgress);

    // start the packing process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }; // busy wait - need to find something better...
    observer->subJobStopped();

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode())) {
        observer->detailedError(i18n("Failed to pack %1.", dest), observer->wasCancelled() ? i18n("User cancelled.") : proc.getErrorMsg());
        return false;
    }

    KConfigGroup group(krConfig, "Archives");
    if (group.readEntry("Test Archives", _TestArchives) && !test(dest, type, password, observer, count)) {
        observer->error(i18n("Failed to pack %1.", dest));
        return false;
    }
    return true; // SUCCESS
}

bool KrArcHandler::openWallet()
{
    if (!wallet) {
        // find a suitable parent window
        QWidget *actWindow = QApplication::activeWindow();
        WId w = 0;
        if (actWindow)
            w = actWindow->effectiveWinId();
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), w);

    }
    return (wallet != nullptr);
}

QString KrArcHandler::getPassword(const QString &path)
{
    QString password;

    QString key = "krarc-" + path;

    if (!KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(), KWallet::Wallet::PasswordFolder(), key)) {
        if (!KWallet::Wallet::isOpen(KWallet::Wallet::NetworkWallet()) && wallet != nullptr) {
            delete wallet;
            wallet = nullptr;
        }
        if (openWallet() && wallet->hasFolder(KWallet::Wallet::PasswordFolder())) {
            wallet->setFolder(KWallet::Wallet::PasswordFolder());
            QMap<QString, QString> map;
            if (wallet->readMap(key, map) == 0) {
                QMap<QString, QString>::const_iterator it = map.constFind("password");
                if (it != map.constEnd())
                    password = it.value();
            }
        }
    }

    bool keep = true;
    QString user = "archive";
    QPointer<KPasswordDialog> passDlg = new KPasswordDialog(nullptr, KPasswordDialog::ShowKeepPassword);
    passDlg->setPrompt(i18n("This archive is encrypted, please supply the password:")), passDlg->setUsername(user);
    passDlg->setPassword(password);
    if (passDlg->exec() == KPasswordDialog::Accepted) {
        password = passDlg->password();
        if (keep) {
            if (!KWallet::Wallet::isOpen(KWallet::Wallet::NetworkWallet()) && wallet != nullptr) {
                delete wallet;
                wallet = nullptr;
            }
            if (openWallet()) {
                bool ok = true;
                if (!wallet->hasFolder(KWallet::Wallet::PasswordFolder()))
                    ok = wallet->createFolder(KWallet::Wallet::PasswordFolder());
                if (ok) {
                    wallet->setFolder(KWallet::Wallet::PasswordFolder());
                    QMap<QString, QString> map;
                    map.insert("login", "archive");
                    map.insert("password", password);
                    wallet->writeMap(key, map);
                }
            }
        }
        delete passDlg;
        return password;
    }
    delete passDlg;

    return "";
}

bool KrArcHandler::isArchive(const QUrl &url)
{
    QString protocol = url.scheme();
    if (arcProtocols.indexOf(protocol) != -1)
        return true;
    else
        return false;
}

QString KrArcHandler::getType(bool &encrypted, QString fileName, const QString &mime, bool check7zEncrypted, bool fast)
{
    QString result = detectArchive(encrypted, std::move(fileName), check7zEncrypted, fast);
    if (result.isNull()) {
        // Then the type is based on the mime type
        return getShortTypeFromMime(mime);
    }
    return result;
}

bool KrArcHandler::checkStatus(const QString &type, int exitCode)
{
    return KrArcBaseManager::checkStatus(type, exitCode);
}

void KrArcHandler::checkIf7zIsEncrypted(bool &encrypted, QString fileName)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `kio_krarcProtocol::checkIf7zIsEncrypted()`

    Kr7zEncryptionChecker proc;
    // TODO incorporate all this in Kr7zEncryptionChecker
    // Note: That command uses information given in a comment from
    // https://stackoverflow.com/questions/5248572/how-do-i-know-if-7zip-used-aes256
    proc << find7zExecutable() << "l"
         << "-slt" << fileName;
    proc.start();
    proc.waitForFinished();
    encrypted = proc.isEncrypted();
}

QString KrArcHandler::registeredProtocol(const QString &mimetype)
{
    if (slaveMap == nullptr) {
        slaveMap = new QMap<QString, QString>();

        KConfigGroup group(krConfig, "Protocols");
        QStringList protList = group.readEntry("Handled Protocols", QStringList());
        for (auto &it : protList) {
            QStringList mimes = group.readEntry(QString("Mimes For %1").arg(it), QStringList());
            for (auto &mime : mimes)
                (*slaveMap)[mime] = it;
        }
    }
    QString protocol = (*slaveMap)[mimetype];
    if (protocol.isEmpty()) {
        if (krarcArchiveMimetypes.contains(mimetype)) {
            return QStringLiteral("krarc");
        }
        protocol = KProtocolManager::protocolForArchiveMimetype(mimetype);
    }
    return protocol;
}

void KrArcHandler::clearProtocolCache()
{
    if (slaveMap)
        delete slaveMap;
    slaveMap = nullptr;
}
