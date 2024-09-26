/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    This file is heavily based on ktar from kdelibs

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iso.h"

#include <zlib.h>

// QtCore
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <qplatformdefs.h>

#include "../../app/compat.h"
#include "kiso.h"
#include "kisodirectory.h"
#include "kisofile.h"
#include "libisofs/iso_fs.h"

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.iso" FILE "iso.json")
};

using namespace KIO;
extern "C" {

int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    // qDebug()   << "Starting " << getpid() << endl;

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_iso protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    kio_isoProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    // qDebug()   << "Done" << endl;
    return 0;
}

} // extern "C"

typedef struct {
    char magic[8];
    char uncompressed_len[4];
    unsigned char header_size;
    unsigned char block_size;
    char reserved[2]; /* Reserved for future use, MBZ */
} compressed_file_header;

static const unsigned char zisofs_magic[8] = {0x37, 0xE4, 0x53, 0x96, 0xC9, 0xDB, 0xD6, 0x07};

kio_isoProtocol::kio_isoProtocol(const QByteArray &pool, const QByteArray &app)
    : WorkerBase("iso", pool, app)
{
    // qDebug() << "kio_isoProtocol::kio_isoProtocol" << endl;
    m_isoFile = nullptr;
}

kio_isoProtocol::~kio_isoProtocol()
{
    delete m_isoFile;
}

bool kio_isoProtocol::checkNewFile(QString fullPath, QString &path, int startsec)
{
    // qDebug()   << "kio_isoProtocol::checkNewFile " << fullPath << " startsec: " <<
    // startsec << endl;

    // Are we already looking at that file ?
    if (m_isoFile && startsec == m_isoFile->startSec() && m_isoFile->fileName() == fullPath.left(m_isoFile->fileName().length())) {
        // Has it changed ?
        QT_STATBUF statbuf;
        if (QT_STAT(QFile::encodeName(m_isoFile->fileName()).data(), &statbuf) == 0) {
            if (m_mtime == statbuf.st_mtime) {
                path = fullPath.mid(m_isoFile->fileName().length());
                // qDebug()   << "kio_isoProtocol::checkNewFile returning " << path << endl;
                if (path.endsWith(DIR_SEPARATOR_CHAR)) {
                    path.chop(1);
                }
                if (path.isEmpty()) {
                    path = DIR_SEPARATOR_CHAR;
                }
                return true;
            }
        }
    }
    // qDebug() << "Need to open a new file" << endl;

    // Close previous file
    if (m_isoFile) {
        m_isoFile->close();
        delete m_isoFile;
        m_isoFile = nullptr;
    }

    // Find where the iso file is in the full path
    qsizetype pos = 0;
    QString isoFile;
    path.clear();

    qsizetype len = fullPath.length();
    if (len != 0 && fullPath[len - 1] != DIR_SEPARATOR_CHAR)
        fullPath += DIR_SEPARATOR_CHAR;

    // qDebug()   << "the full path is " << fullPath << endl;
    while ((pos = fullPath.indexOf(DIR_SEPARATOR_CHAR, pos + 1)) != -1) {
        QString tryPath = fullPath.left(pos);
        // qDebug()   << fullPath << "  trying " << tryPath << endl;

        QT_STATBUF statbuf;
        if (QT_LSTAT(QFile::encodeName(tryPath).data(), &statbuf) == 0 && !S_ISDIR(statbuf.st_mode)) {
            bool isFile = true;
            if (S_ISLNK(statbuf.st_mode)) {
                char symDest[256];
                memset(symDest, 0, 256);
                ssize_t endOfName = readlink(QFile::encodeName(tryPath).data(), symDest, 256);
                if (endOfName != -1) {
                    if (QDir(QString::fromLocal8Bit(symDest)).exists())
                        isFile = false;
                }
            }

            if (isFile) {
                isoFile = tryPath;
                m_mtime = statbuf.st_mtime;
                m_mode = statbuf.st_mode;
                path = fullPath.mid(pos + 1);
                // qDebug()   << "fullPath=" << fullPath << " path=" << path << endl;
                if (path.endsWith(DIR_SEPARATOR_CHAR)) {
                    path.chop(1);
                }
                if (path.isEmpty()) {
                    path = DIR_SEPARATOR_CHAR;
                }
                // qDebug()   << "Found. isoFile=" << isoFile << " path=" << path << endl;
                break;
            }
        }
    }
    if (isoFile.isEmpty()) {
        // qDebug()   << "kio_isoProtocol::checkNewFile: not found" << endl;
        return false;
    }

    // Open new file
    // qDebug() << "Opening KIso on " << isoFile << endl;
    m_isoFile = new KIso(isoFile);
    m_isoFile->setStartSec(startsec);
    if (!m_isoFile->open(QIODevice::ReadOnly)) {
        // qDebug()   << "Opening " << isoFile << " failed." << endl;
        delete m_isoFile;
        m_isoFile = nullptr;
        return false;
    }

    return true;
}

void kio_isoProtocol::createUDSEntry(const KArchiveEntry *isoEntry, UDSEntry &entry)
{
    entry.clear();
    entry.fastInsert(UDSEntry::UDS_NAME, isoEntry->name());
    entry.fastInsert(UDSEntry::UDS_FILE_TYPE, isoEntry->permissions() & S_IFMT); // keep file type only
    entry.fastInsert(UDSEntry::UDS_ACCESS, isoEntry->permissions() & 07777); // keep permissions only

    if (isoEntry->isFile()) {
        long long si = (dynamic_cast<const KIsoFile *>(isoEntry))->realsize();
        if (!si)
            si = (dynamic_cast<const KIsoFile *>(isoEntry))->size();
        entry.fastInsert(UDSEntry::UDS_SIZE, si);
    } else {
        entry.fastInsert(UDSEntry::UDS_SIZE, 0L);
    }

    entry.fastInsert(UDSEntry::UDS_USER, isoEntry->user());
    entry.fastInsert(UDSEntry::UDS_GROUP, isoEntry->group());
    entry.fastInsert((uint)UDSEntry::UDS_MODIFICATION_TIME, isoEntry->date().toSecsSinceEpoch());
    entry.fastInsert(UDSEntry::UDS_ACCESS_TIME,
                     isoEntry->isFile() ? (dynamic_cast<const KIsoFile *>(isoEntry))->adate() : (dynamic_cast<const KIsoDirectory *>(isoEntry))->adate());

    entry.fastInsert(UDSEntry::UDS_CREATION_TIME,
                     isoEntry->isFile() ? (dynamic_cast<const KIsoFile *>(isoEntry))->cdate() : (dynamic_cast<const KIsoDirectory *>(isoEntry))->cdate());

    entry.fastInsert(UDSEntry::UDS_LINK_DEST, isoEntry->symLinkTarget());
}

WorkerResult kio_isoProtocol::listDir(const QUrl &url)
{
    // qDebug() << "kio_isoProtocol::listDir " << url.url() << endl;

    QString path;
    if (!checkNewFile(getPath(url), path, url.hasFragment() ? url.fragment(QUrl::FullyDecoded).toInt() : -1)) {
        QByteArray _path(QFile::encodeName(getPath(url)));
        // qDebug()  << "Checking (stat) on " << _path << endl;
        QT_STATBUF buff;
        if (QT_STAT(_path.data(), &buff) == -1 || !S_ISDIR(buff.st_mode)) {
            return WorkerResult::fail(ERR_DOES_NOT_EXIST, getPath(url));
        }
        // It's a real dir -> redirect
        QUrl redir;
        redir.setPath(getPath(url));
        if (url.hasFragment())
            redir.setFragment(url.fragment(QUrl::FullyDecoded));
        // qDebug()  << "Ok, redirection to " << redir.url() << endl;
        redir.setScheme("file");
        redirection(redir);
        // And let go of the iso file - for people who want to unmount a cdrom after that
        delete m_isoFile;
        m_isoFile = nullptr;
        return WorkerResult::pass();
    }

    if (path.isEmpty()) {
        QUrl redir(QStringLiteral("iso:/"));
        // qDebug() << "url.path()==" << getPath(url) << endl;
        if (url.hasFragment())
            redir.setFragment(url.fragment(QUrl::FullyDecoded));
        redir.setPath(getPath(url) + QString::fromLatin1(DIR_SEPARATOR));
        // qDebug() << "kio_isoProtocol::listDir: redirection " << redir.url() << endl;
        redir.setScheme("file");
        redirection(redir);
        return WorkerResult::pass();
    }

    // qDebug()  << "checkNewFile done" << endl;
    const KArchiveDirectory *root = m_isoFile->directory();
    const KArchiveDirectory *dir;
    if (!path.isEmpty() && path != DIR_SEPARATOR) {
        // qDebug()   << QString("Looking for entry %1").arg(path) << endl;
        const KArchiveEntry *e = root->entry(path);
        if (!e) {
            return WorkerResult::fail(ERR_DOES_NOT_EXIST, path);
        }
        if (!e->isDirectory()) {
            return WorkerResult::fail(ERR_IS_FILE, path);
        }
        dir = dynamic_cast<const KArchiveDirectory *>(e);
    } else {
        dir = root;
    }

    QStringList l = dir->entries();
    totalSize(l.count());

    UDSEntry entry;
    QStringList::Iterator it = l.begin();
    for (; it != l.end(); ++it) {
        // qDebug()   << (*it) << endl;
        const KArchiveEntry *isoEntry = dir->entry((*it));

        createUDSEntry(isoEntry, entry);

        listEntry(entry);
    }

    return WorkerResult::pass();
    // qDebug()  << "kio_isoProtocol::listDir done" << endl;
}

WorkerResult kio_isoProtocol::stat(const QUrl &url)
{
    QString path;
    UDSEntry entry;

    // qDebug() << "kio_isoProtocol::stat " << url.url() << endl;
    if (!checkNewFile(getPath(url), path, url.hasFragment() ? url.fragment(QUrl::FullyDecoded).toInt() : -1)) {
        // We may be looking at a real directory - this happens
        // when pressing up after being in the root of an archive
        QByteArray _path(QFile::encodeName(getPath(url)));
        // qDebug()  << "kio_isoProtocol::stat (stat) on " << _path << endl;
        QT_STATBUF buff;
        if (QT_STAT(_path.data(), &buff) == -1 || !S_ISDIR(buff.st_mode)) {
            // qDebug() << "isdir=" << S_ISDIR(buff.st_mode) << "  errno=" << strerror(errno) << endl;
            return WorkerResult::fail(ERR_DOES_NOT_EXIST, getPath(url));
        }
        // Real directory. Return just enough information for KRun to work
        entry.fastInsert(UDSEntry::UDS_NAME, url.fileName());
        // qDebug()  << "kio_isoProtocol::stat returning name=" << url.fileName() << endl;

        entry.fastInsert(UDSEntry::UDS_FILE_TYPE, buff.st_mode & S_IFMT);

        statEntry(entry);

        // And let go of the iso file - for people who want to unmount a cdrom after that
        delete m_isoFile;
        m_isoFile = nullptr;
        return WorkerResult::pass();
    }

    const KArchiveDirectory *root = m_isoFile->directory();
    const KArchiveEntry *isoEntry;
    if (path.isEmpty()) {
        path = QString::fromLatin1(DIR_SEPARATOR);
        isoEntry = root;
    } else {
        isoEntry = root->entry(path);
    }
    if (!isoEntry) {
        return WorkerResult::fail(ERR_DOES_NOT_EXIST, path);
    }
    createUDSEntry(isoEntry, entry);
    statEntry(entry);
    return WorkerResult::pass();
}

WorkerResult kio_isoProtocol::getFile(const KIsoFile *isoFileEntry, const QString &path)
{
    unsigned long long size, pos = 0;
    bool mime = false, zlib = false;
    QByteArray fileData, pointer_block, inbuf, outbuf;
    char *pptr = nullptr;
    compressed_file_header *hdr;
    int block_shift;
    unsigned long nblocks;
    unsigned long fullsize = 0, block_size = 0, block_size2 = 0;
    size_t ptrblock_bytes;
    unsigned long cstart, cend, csize;
    uLong bytes;

    size = isoFileEntry->realsize();
    if (size >= sizeof(compressed_file_header))
        zlib = true;
    if (!size)
        size = isoFileEntry->size();
    totalSize(size);
    if (!size)
        mimeType("application/x-zerosize");

    if (size && !m_isoFile->device()->isOpen()) {
        m_isoFile->device()->open(QIODevice::ReadOnly);

        // seek(0) ensures integrity with the QIODevice's built-in buffer
        // see bug #372023 for details
        m_isoFile->device()->seek(0);
    }

    if (zlib) {
        fileData = isoFileEntry->dataAt(0, sizeof(compressed_file_header));
        if (fileData.size() == sizeof(compressed_file_header) && !memcmp(fileData.data(), zisofs_magic, sizeof(zisofs_magic))) {
            hdr = (compressed_file_header *)fileData.data();
            block_shift = hdr->block_size;
            block_size = 1UL << block_shift;
            block_size2 = block_size << 1;
            fullsize = isonum_731(hdr->uncompressed_len);
            nblocks = (fullsize + block_size - 1) >> block_shift;
            ptrblock_bytes = (nblocks + 1) * 4;
            pointer_block = isoFileEntry->dataAt(hdr->header_size << 2, ptrblock_bytes);
            if ((unsigned long)pointer_block.size() == ptrblock_bytes) {
                inbuf.resize(static_cast<int>(block_size2));
                if (inbuf.size()) {
                    outbuf.resize(static_cast<int>(block_size));

                    if (outbuf.size())
                        pptr = pointer_block.data();
                    else {
                        return WorkerResult::fail(ERR_CANNOT_READ, path);
                    }
                } else {
                    return WorkerResult::fail(ERR_CANNOT_READ, path);
                }
            } else {
                return WorkerResult::fail(ERR_CANNOT_READ, path);
            }
        } else {
            zlib = false;
        }
    }

    while (pos < size) {
        if (zlib) {
            cstart = isonum_731(pptr);
            pptr += 4;
            cend = isonum_731(pptr);

            csize = cend - cstart;

            if (csize == 0) {
                outbuf.fill(0, -1);
            } else {
                if (csize > block_size2) {
                    // err = EX_DATAERR;
                    break;
                }

                inbuf = isoFileEntry->dataAt(cstart, csize);
                if ((unsigned long)inbuf.size() != csize) {
                    break;
                }

                bytes = block_size; // Max output buffer size
                if ((uncompress((Bytef *)outbuf.data(), &bytes, (Bytef *)inbuf.data(), csize)) != Z_OK) {
                    break;
                }
            }

            if (((fullsize > block_size) && (bytes != block_size)) || ((fullsize <= block_size) && (bytes < fullsize))) {
                break;
            }

            if (bytes > fullsize)
                bytes = fullsize;
            fileData = outbuf;
            fileData.resize(static_cast<int>(bytes));
            fullsize -= bytes;
        } else {
            fileData = isoFileEntry->dataAt(pos, 65536);
            if (fileData.size() == 0)
                break;
        }
        if (!mime) {
            QMimeDatabase db;
            QMimeType mt = db.mimeTypeForFileNameAndData(path, fileData);
            if (mt.isValid()) {
                // qDebug() << "Emitting mimetype " << mt.name() << endl;
                mimeType(mt.name());
                mime = true;
            }
        }
        data(fileData);
        pos += fileData.size();
        processedSize(pos);
    }

    if (pos != size) {
        return WorkerResult::fail(ERR_CANNOT_READ, path);
    }

    fileData.resize(0);
    data(fileData);
    processedSize(pos);
    return WorkerResult::pass();
}

WorkerResult kio_isoProtocol::get(const QUrl &url)
{
    // qDebug()  << "kio_isoProtocol::get" << url.url() << endl;

    QString path;
    if (!checkNewFile(getPath(url), path, url.hasFragment() ? url.fragment(QUrl::FullyDecoded).toInt() : -1)) {
        return WorkerResult::fail(ERR_DOES_NOT_EXIST, getPath(url));
    }

    const KArchiveDirectory *root = m_isoFile->directory();
    const KArchiveEntry *isoEntry = root->entry(path);

    if (!isoEntry) {
        return WorkerResult::fail(ERR_DOES_NOT_EXIST, path);
    }
    if (isoEntry->isDirectory()) {
        return WorkerResult::fail(ERR_IS_DIRECTORY, path);
    }

    const auto *isoFileEntry = dynamic_cast<const KIsoFile *>(isoEntry);
    if (!isoEntry->symLinkTarget().isEmpty()) {
        // qDebug() << "Redirection to " << isoEntry->symLinkTarget() << endl;
        QUrl realURL = QUrl(url).resolved(QUrl(isoEntry->symLinkTarget()));
        // qDebug() << "realURL= " << realURL.url() << endl;
        realURL.setScheme("file");
        redirection(realURL);
        return WorkerResult::pass();
    }
    auto result = getFile(isoFileEntry, path);
    if (m_isoFile->device()->isOpen())
        m_isoFile->device()->close();
    return result;
}

QString kio_isoProtocol::getPath(const QUrl &url)
{
    QString path = url.path();
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

#include "iso.moc"
