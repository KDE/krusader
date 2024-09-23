/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filesystem.h"

#include <memory>

// QtCore
#include <QDebug>
#include <QDir>
#include <QList>
// QtWidgets
#include <qplatformdefs.h>

#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KSharedConfig>

#include "../JobMan/jobman.h"
#include "../JobMan/krjob.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "fileitem.h"
#include "krpermhandler.h"

FileSystem::FileSystem()
    : DirListerInterface(nullptr)
    , _isRefreshing(false)
{
}

FileSystem::~FileSystem()
{
    clear(_fileItems);
    // please don't remove this line. This informs the view about deleting the file items.
    emit cleared();
}

QList<QUrl> FileSystem::getUrls(const QStringList &names) const
{
    QList<QUrl> urls;
    for (const QString &name : names) {
        urls.append(getUrl(name));
    }
    return urls;
}

FileItem *FileSystem::getFileItem(const QString &name) const
{
    return _fileItems.contains(name) ? _fileItems.value(name) : nullptr;
}

KIO::filesize_t FileSystem::totalSize() const
{
    KIO::filesize_t temp = 0;
    for (FileItem *item : _fileItems.values()) {
        if (!item->isDir() && item->getName() != "." && item->getName() != "..") {
            temp += item->getSize();
        }
    }

    return temp;
}

QUrl FileSystem::ensureTrailingSlash(const QUrl &url)
{
    if (url.path().endsWith('/')) {
        return url;
    }

    QUrl adjustedUrl(url);
    adjustedUrl.setPath(adjustedUrl.path() + '/');
    return adjustedUrl;
}

QUrl FileSystem::preferLocalUrl(const QUrl &url)
{
    if (url.isEmpty() || !url.scheme().isEmpty())
        return url;

    QUrl adjustedUrl = url;
    adjustedUrl.setScheme("file");
    return adjustedUrl;
}

bool FileSystem::scanOrRefresh(const QUrl &directory, bool onlyScan)
{
    qDebug() << "from current dir=" << _currentDirectory.toDisplayString() << "; to=" << directory.toDisplayString();
    if (_isRefreshing) {
        // NOTE: this does not happen (unless async)";
        return false;
    }

    // workaround for krarc: find out if transition to local fs is wanted and adjust URL manually
    QUrl url = directory;
    if (_currentDirectory.scheme() == "krarc" && url.scheme() == "krarc" && QDir(url.path()).exists()) {
        url.setScheme("file");
    }

    const bool dirChange = !url.isEmpty() && cleanUrl(url) != _currentDirectory;

    const QUrl toRefresh = dirChange ? url.adjusted(QUrl::NormalizePathSegments) : _currentDirectory;
    if (!toRefresh.isValid()) {
        emit error(i18n("Malformed URL:\n%1", toRefresh.toDisplayString()));
        return false;
    }

    _isRefreshing = true;

    FileItemDict tempFileItems(_fileItems); // old file items are still used during refresh
    _fileItems.clear();
    if (dirChange)
        // show an empty directory while loading the new one and clear selection
        emit cleared();

    const bool refreshed = refreshInternal(toRefresh, onlyScan);
    _isRefreshing = false;

    if (!refreshed) {
        // cleanup and abort
        if (!dirChange)
            emit cleared();
        clear(tempFileItems);
        return false;
    }

    emit scanDone(dirChange);

    clear(tempFileItems);

    updateFilesystemInfo();

    return true;
}

void FileSystem::deleteFiles(const QList<QUrl> &urls, bool moveToTrash)
{
    KrJob *krJob = KrJob::createDeleteJob(urls, moveToTrash);
    connect(krJob, &KrJob::started, this, [=](KIO::Job *job) {
        connectJobToSources(job, urls);
    });

    if (moveToTrash) {
        // update destination: the trash bin (in case a panel/tab is showing it)
        connect(krJob, &KrJob::started, this, [=](KIO::Job *job) {
            // Note: the "trash" protocol should always have only one "/" after the "scheme:" part
            connect(job, &KIO::Job::result, this, [=]() {
                emit fileSystemChanged(QUrl("trash:/"), false);
            });
        });
    }

    krJobMan->manageJob(krJob);
}

void FileSystem::connectJobToSources(KJob *job, const QList<QUrl> &urls)
{
    if (!urls.isEmpty()) {
        // TODO we assume that all files were in the same directory and only emit one signal for
        // the directory of the first file URL (all subdirectories of parent are notified)
        const QUrl url = urls.first().adjusted(QUrl::RemoveFilename);
        connect(job, &KIO::Job::result, this, [=]() {
            emit fileSystemChanged(url, true);
        });
    }
}

void FileSystem::connectJobToDestination(KJob *job, const QUrl &destination)
{
    connect(job, &KIO::Job::result, this, [=]() {
        emit fileSystemChanged(destination, false);
    });
    // (additional) direct refresh if on local fs because watcher is too slow
    const bool refresh = cleanUrl(destination) == _currentDirectory && isLocal();
    connect(job, &KIO::Job::result, this, [=](KJob *job) {
        slotJobResult(job, refresh);
    });
}

bool FileSystem::showHiddenFiles()
{
    const KConfigGroup gl(krConfig, "Look&Feel");
    return gl.readEntry("Show Hidden", _ShowHidden);
}

void FileSystem::addFileItem(FileItem *item)
{
    _fileItems.insert(item->getName(), item);
}

FileItem *FileSystem::createLocalFileItem(const QString &name, const QString &directory, bool virt)
{
    const QDir dir = QDir(directory);
    const QString path = dir.filePath(name);
    const QByteArray pathByteArray = path.toLocal8Bit();
    const QString fileItemName = virt ? path : name;
    const QUrl fileItemUrl = QUrl::fromLocalFile(path);

    // read file status; in case of error create a "broken" file item
    QT_STATBUF stat_p;
    memset(&stat_p, 0, sizeof(stat_p));
    if (QT_LSTAT(pathByteArray.data(), &stat_p) < 0)
        return FileItem::createBroken(fileItemName, fileItemUrl);

    const KIO::filesize_t size = stat_p.st_size;
    bool isDir = S_ISDIR(stat_p.st_mode);
    const bool isLink = S_ISLNK(stat_p.st_mode);

    // for links, read link destination and determine whether it's broken or not
    QString linkDestination;
    bool brokenLink = false;
    if (isLink) {
        linkDestination = readLinkSafely(pathByteArray.data());

        if (linkDestination.isNull()) {
            brokenLink = true;
        } else {
            const QFileInfo linkFile(dir, linkDestination);
            if (!linkFile.exists())
                brokenLink = true;
            else if (linkFile.isDir())
                isDir = true;
        }
    }

    // TODO use statx available in glibc >= 2.28 supporting creation time (btime) and more

    // create normal file item
    return new FileItem(fileItemName,
                        fileItemUrl,
                        isDir,
                        size,
                        stat_p.st_mode,
                        stat_p.st_mtime,
                        stat_p.st_ctime,
                        stat_p.st_atime,
                        -1,
                        stat_p.st_uid,
                        stat_p.st_gid,
                        QString(),
                        QString(),
                        isLink,
                        linkDestination,
                        brokenLink);
}

QString FileSystem::readLinkSafely(const char *path)
{
    // inspired by the areadlink_with_size function from gnulib, which is used for coreutils
    // idea: start with a small buffer and gradually increase it as we discover it wasn't enough

    QT_OFF_T bufferSize = 1024; // start with 1 KiB
    QT_OFF_T maxBufferSize = std::numeric_limits<QT_OFF_T>::max();

    while (true) {
        // try to read the link
        std::unique_ptr<char[]> buffer(new char[bufferSize]);
        auto nBytesRead = readlink(path, buffer.get(), bufferSize);

        // should never happen, asserted by the readlink
        if (nBytesRead > bufferSize) {
            return QString();
        }

        // read failure
        if (nBytesRead < 0) {
            qDebug() << "Failed to read the link " << path;
            return QString();
        }

        // read success
        if (nBytesRead < bufferSize || nBytesRead == maxBufferSize) {
            return QString::fromLocal8Bit(buffer.get(), static_cast<int>(nBytesRead));
        }

        // increase the buffer and retry again
        // bufferSize < maxBufferSize is implied from previous checks
        if (bufferSize <= maxBufferSize / 2) {
            bufferSize *= 2;
        } else {
            bufferSize = maxBufferSize;
        }
    }
}

FileItem *FileSystem::createFileItemFromKIO(const KIO::UDSEntry &entry, const QUrl &directory, bool virt)
{
    const KFileItem kfi(entry, directory, true, true);

    const QString name = kfi.name();
    // ignore un-needed entries
    if (name.isEmpty() || name == "." || name == "..") {
        return nullptr;
    }

    const QString localPath = kfi.localPath();
    const QUrl url = !localPath.isEmpty() ? QUrl::fromLocalFile(localPath) : kfi.url();
    const QString fname = virt ? url.toDisplayString() : name;

    // get file statistics...
    const time_t mtime = kfi.time(KFileItem::ModificationTime).toSecsSinceEpoch();
    const time_t atime = kfi.time(KFileItem::AccessTime).toSecsSinceEpoch();
    const mode_t mode = kfi.mode() | kfi.permissions();
    const QDateTime creationTime = kfi.time(KFileItem::CreationTime);
    const time_t btime = creationTime.isValid() ? creationTime.toSecsSinceEpoch() : (time_t)-1;

    // NOTE: we could get the mimetype (and file icon) from the kfileitem here but this is very
    // slow. Instead, the file item class has it's own (faster) way to determine the file type.

    // NOTE: "broken link" flag is always false, checking link destination existence is
    // considered to be too expensive
    return new FileItem(fname,
                        url,
                        kfi.isDir(),
                        kfi.size(),
                        mode,
                        mtime,
                        -1,
                        atime,
                        btime,
                        (uid_t)-1,
                        (gid_t)-1,
                        kfi.user(),
                        kfi.group(),
                        kfi.isLink(),
                        kfi.linkDest(),
                        false,
                        kfi.ACL().asString(),
                        kfi.defaultACL().asString());
}

void FileSystem::slotJobResult(KJob *job, bool refresh)
{
    if (job->error() && job->uiDelegate()) {
        // show errors for modifying operations as popup (works always)
        job->uiDelegate()->showErrorMessage();
    }

    if (refresh) {
        FileSystem::refresh();
    }
}

void FileSystem::clear(FileItemDict &fileItems)
{
    QHashIterator<QString, FileItem *> lit(fileItems);
    while (lit.hasNext()) {
        delete lit.next().value();
    }
    fileItems.clear();
}
