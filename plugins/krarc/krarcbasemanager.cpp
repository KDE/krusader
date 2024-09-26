/*
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krarcbasemanager.h"
#include "../../app/krdebuglogger.h"

#include <KTar>

#include <QtCore/QStandardPaths>

KrArcBaseManager::AutoDetectParams KrArcBaseManager::autoDetectParams[] = {
    {"zip", 0, "PK\x03\x04"},
    {"rar", 0, "Rar!\x1a"},
    {"arj", 0, "\x60\xea"},
    {"rpm", 0, "\xed\xab\xee\xdb"},
    {"ace", 7, "**ACE**"},
    {"bzip2", 0, "\x42\x5a\x68\x39\x31"},
    {"gzip", 0, "\x1f\x8b"},
    {"deb", 0, "!<arch>\ndebian-binary   "},
    {"7z", 0, "7z\xbc\xaf\x27\x1c"} /*,
  {"xz",   0, "\xfd\x37\x7a\x58\x5a\x00"}*/
};

int KrArcBaseManager::autoDetectElems = sizeof(autoDetectParams) / sizeof(AutoDetectParams);
const int KrArcBaseManager::maxLenType = 5;

KrArcBaseManager::KrArcBaseManager()
    : krConf("krusaderrc")
    , dependGrp(&krConf, "Dependencies")
{
}

QString KrArcBaseManager::fullPathName(const QString &name)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `KrServices::fullPathName()`
    // and `KrServices::cmdExist()`

    // Note: KRFUNC was not used here in order to avoid filling the log with too much information
    KRDEBUG(name);

    QString supposedName = dependGrp.readEntry(name, QString());
    if (QFileInfo::exists(supposedName))
        return supposedName;

    if ((supposedName = QStandardPaths::findExecutable(name)).isEmpty())
        return QString();

    // Because an executable file has been found, its path is remembered
    // in order to avoid being searched next time
    dependGrp.writeEntry(name, supposedName);

    return supposedName;
}

QString KrArcBaseManager::find7zExecutable()
{
    KRFUNC;
    QString program = fullPathName("7z");
    if (program.isEmpty()) {
        KRDEBUG("A 7z program was not found");
        program = fullPathName("7za");
        if (program.isEmpty()) {
            KRDEBUG("A 7za program was not found");
        }
    }

    return program;
}

//! Checks if a returned status ("exit code") of an archiving-related process is OK
/*!
    \param arcType A short QString which contains an identifier of the type of the archive.
    \param exitCode The returned status ("exit code") of an archiving-related process.
    \return If the exit code means that the archiving-related process ended without errors.
*/
bool KrArcBaseManager::checkStatus(const QString &arcType, int exitCode)
{
    if (arcType == "zip" || arcType == "rar" || arcType == "7z")
        return exitCode == 0 || exitCode == 1;
    else if (arcType == "ace" || arcType == "bzip2" || arcType == "lha" || arcType == "rpm" || arcType == "cpio" || arcType == "tar" || arcType == "tarz"
             || arcType == "tbz" || arcType == "tgz" || arcType == "arj" || arcType == "deb" || arcType == "tlz" || arcType == "txz")
        return exitCode == 0;
    else if (arcType == "gzip" || arcType == "lzma" || arcType == "xz")
        return exitCode == 0 || exitCode == 2;
    else
        return exitCode == 0;
}

QString KrArcBaseManager::detectArchive(bool &encrypted, const QString &fileName, bool check7zEncrypted, bool fast)
{
    encrypted = false;

    QFile arcFile(fileName);
    if (arcFile.open(QIODevice::ReadOnly)) {
        char buffer[1024];
        long sizeMax = arcFile.read(buffer, sizeof(buffer));
        arcFile.close();

        for (int i = 0; i < autoDetectElems; i++) {
            QByteArray detectionString = autoDetectParams[i].detectionString;
            int location = autoDetectParams[i].location;

            qsizetype endPtr = detectionString.length() + location;
            if (endPtr > sizeMax)
                continue;

            int j = 0;
            for (; j != detectionString.length(); j++) {
                if (detectionString[j] == '?')
                    continue;
                if (buffer[location + j] != detectionString[j])
                    break;
            }

            if (j == detectionString.length()) {
                QString type = autoDetectParams[i].type;
                if (type == "bzip2" || type == "gzip") {
                    if (fast) {
                        if (fileName.endsWith(QLatin1String(".tar.gz")))
                            type = "tgz";
                        else if (fileName.endsWith(QLatin1String(".tar.bz2")))
                            type = "tbz";
                    } else {
                        KTar tapeArchive(fileName);
                        if (tapeArchive.open(QIODevice::ReadOnly)) {
                            tapeArchive.close();
                            if (type == "gzip")
                                type = "tgz";
                            else if (type == "bzip2")
                                type = "tbz";
                        }
                    }
                } else if (type == "zip")
                    encrypted = (buffer[6] & 1);
                else if (type == "arj") {
                    if (sizeMax > 4) {
                        long headerSize = ((unsigned char *)buffer)[2] + 256 * ((unsigned char *)buffer)[3];
                        long fileHeader = headerSize + 10;
                        if (fileHeader + 9 < sizeMax && buffer[fileHeader] == (char)0x60 && buffer[fileHeader + 1] == (char)0xea)
                            encrypted = (buffer[fileHeader + 8] & 1);
                    }
                } else if (type == "rar") {
                    if (sizeMax > 13 && buffer[9] == (char)0x73) {
                        if (buffer[10] & 0x80) { // the header is encrypted?
                            encrypted = true;
                        } else {
                            long offset = 7;
                            long mainHeaderSize = ((unsigned char *)buffer)[offset + 5] + 256 * ((unsigned char *)buffer)[offset + 6];
                            offset += mainHeaderSize;
                            while (offset + 10 < sizeMax) {
                                long headerSize = ((unsigned char *)buffer)[offset + 5] + 256 * ((unsigned char *)buffer)[offset + 6];
                                bool isDir = (buffer[offset + 7] == '\0') && (buffer[offset + 8] == '\0') && (buffer[offset + 9] == '\0')
                                    && (buffer[offset + 10] == '\0');

                                if (buffer[offset + 2] != (char)0x74)
                                    break;
                                if (!isDir) {
                                    encrypted = (buffer[offset + 3] & 4) != 0;
                                    break;
                                }
                                offset += headerSize;
                            }
                        }
                    }
                } else if (type == "ace") {
                    long offset = 0;
                    long mainHeaderSize = ((unsigned char *)buffer)[offset + 2] + 256 * ((unsigned char *)buffer)[offset + 3] + 4;
                    offset += mainHeaderSize;
                    while (offset + 10 < sizeMax) {
                        long headerSize = ((unsigned char *)buffer)[offset + 2] + 256 * ((unsigned char *)buffer)[offset + 3] + 4;
                        bool isDir =
                            (buffer[offset + 11] == '\0') && (buffer[offset + 12] == '\0') && (buffer[offset + 13] == '\0') && (buffer[offset + 14] == '\0');

                        if (buffer[offset + 4] != (char)0x01)
                            break;
                        if (!isDir) {
                            encrypted = (buffer[offset + 6] & 64) != 0;
                            break;
                        }
                        offset += headerSize;
                    }
                } else if (type == "7z") {
                    // encryption check is expensive, check only if it's necessary
                    if (check7zEncrypted)
                        checkIf7zIsEncrypted(encrypted, fileName);
                }
                return type;
            }
        }

        if (sizeMax >= 512) {
            /* checking if it's a tar file */
            unsigned checksum = 32 * 8;
            char chksum[9];
            for (int i = 0; i != 512; i++)
                checksum += ((unsigned char *)buffer)[i];
            for (int i = 148; i != 156; i++)
                checksum -= ((unsigned char *)buffer)[i];
            sprintf(chksum, "0%o", checksum);
            if (!memcmp(buffer + 148, chksum, strlen(chksum))) {
                auto k = strlen(chksum);
                for (; k < 8; k++)
                    if (buffer[148 + k] != 0 && buffer[148 + k] != 32)
                        break;
                if (k == 8)
                    return "tar";
            }
        }
    }

    if (fileName.endsWith(QLatin1String(".tar.lzma")) || fileName.endsWith(QLatin1String(".tlz"))) {
        return "tlz";
    }
    if (fileName.endsWith(QLatin1String(".lzma"))) {
        return "lzma";
    }

    if (fileName.endsWith(QLatin1String(".tar.xz")) || fileName.endsWith(QLatin1String(".txz"))) {
        return "txz";
    }
    if (fileName.endsWith(QLatin1String(".xz"))) {
        return "xz";
    }

    return QString();
}

//! Returns a short identifier of the type of a file, obtained from the mime type of the file
/*!
    \param mime The mime type of the file.
    \return A short QString which contains an identifier of the type of the file.
*/
QString KrArcBaseManager::getShortTypeFromMime(const QString &mime)
{
    // Reminder: If a mime type is added/removed/modified in that
    // member function, it's important to research if the type has to
    // be added/removed/modified in the `krarc.protocol` file, or
    // in `KrArcHandler::KrArcHandler()`

    // 7zip files are a not a normal case because their mimetype does not
    // follow the norm of other types: zip, tar, lha, ace, arj, etc.
    if (mime == "application/x-7z-compressed")
        return "7z";

    // If it's a rar file but its mimetype isn't "application/x-rar"
    if (mime == "application/x-rar-compressed" || mime == "application/vnd.rar")
        return "rar";

    // If it's a cbr file but its mimetype isn't "application/x-cbr"
    if (mime == "application/vnd.comicbook-rar")
        return "cbr";

    // The short type that will be returned
    QString sType = mime;

    qsizetype lastHyphen = sType.lastIndexOf('-');
    if (lastHyphen != -1)
        sType = sType.mid(lastHyphen + 1);
    else {
        qsizetype lastSlash = sType.lastIndexOf('/');
        if (lastSlash != -1)
            sType = sType.mid(lastSlash + 1);
    }
    // The identifier kept short
    if (sType.length() > maxLenType)
        sType = sType.right(maxLenType);

    return sType;
}
