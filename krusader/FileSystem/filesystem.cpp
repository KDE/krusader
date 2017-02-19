/***************************************************************************
                            filesystem.cpp
                       -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
  ------------------------------------------------------------------------
   the filesystem class is an extendable class which by itself does (almost)
   nothing. other filesystems like the normal_filesystem inherits from this class and
   make it possible to use a consistent API for all types of filesystems.

 ***************************************************************************

   A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filesystem.h"

// QtCore
#include <QDir>
#include <QEventLoop>
#include <QList>
// QtWidgets
#include <QApplication>
#include <qplatformdefs.h>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KIO/DirectorySizeJob>
#include <KIO/JobUiDelegate>

#include "../defaults.h"
#include "../krglobal.h"
#include "../JobMan/jobman.h"
#include "../JobMan/krjob.h"
#include "krpermhandler.h"

FileSystem::FileSystem() : DirListerInterface(0), _isRefreshing(false) {}

FileSystem::~FileSystem()
{
    clear(_fileItems);
    emit cleared(); // please don't remove this line. This informs the view about deleting the references
}

QList<QUrl> FileSystem::getUrls(const QStringList &names)
{
    QList<QUrl> urls;
    for (const QString name : names) {
        urls.append(getUrl(name));
    }
    return urls;
}

FileItem *FileSystem::getFileItem(const QString &name)
{
    return _fileItems.contains(name) ? _fileItems.value(name) : 0;
}

QList<FileItem *> FileSystem::searchFileItems(const KRQuery &filter)
{
    QList<FileItem *> result;
    for (FileItem *item : _fileItems.values()) {
        if (filter.match(item)) {
            result.append(item);
        }
    }

    return result;
}

KIO::filesize_t FileSystem::totalSize()
{
    KIO::filesize_t temp = 0;
    for (FileItem *item : _fileItems.values()) {
        if (!item->isDir() && item->getName() != "." && item->getName() != "..") {
            temp += item->getSize();
        }
    }

    return temp;
}

void FileSystem::calcSpace(const QString &name, KIO::filesize_t *totalSize,
                            unsigned long *totalFiles, unsigned long *totalDirs, bool *stop)
{
    const QUrl url = getUrl(name);
    if (url.isEmpty()) {
        krOut << "item for calculating space not found: " << name;
        return;
    }

    calcSpace(url, totalSize, totalFiles, totalDirs, stop);
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

QUrl FileSystem::preferLocalUrl(const QUrl &url){
    if (url.isEmpty() || !url.scheme().isEmpty())
        return url;

    QUrl adjustedUrl = url;
    adjustedUrl.setScheme("file");
    return adjustedUrl;
}

bool FileSystem::refresh(const QUrl &directory)
{

    if (_isRefreshing) {
        // NOTE: this does not happen (unless async)";
        return false;
    }

    // workaround for krarc: find out if transition to local fs is wanted and adjust URL manually
    QUrl url = directory;
    if (_currentDirectory.scheme() == "krarc" && url.scheme() == "krarc" &&
        QDir(url.path()).exists()) {
        url.setScheme("file");
    }

    const bool dirChange = !url.isEmpty() && cleanUrl(url) != _currentDirectory;

    const QUrl toRefresh =
            dirChange ? url.adjusted(QUrl::NormalizePathSegments) : _currentDirectory;
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

    const bool res = refreshInternal(toRefresh, showHiddenFiles());
    _isRefreshing = false;

    if (!res) {
        // cleanup and abort
        if (!dirChange)
            emit cleared();
        clear(tempFileItems);
        return false;
    }

    emit refreshDone(dirChange);

    clear(tempFileItems);

    updateFilesystemInfo();

    return true;
}

void FileSystem::deleteFiles(const QStringList &fileNames, bool moveToTrash)
{
    // get absolute URLs for file names
    const QList<QUrl> fileUrls = getUrls(fileNames);

    KrJob *krJob = KrJob::createDeleteJob(fileUrls, moveToTrash);
    connect(krJob, &KrJob::started, [=](KIO::Job *job) { connectJob(job, currentDirectory()); });
    if (moveToTrash) {
        // update destination: the trash bin (in case a panel/tab is showing it)
        connect(krJob, &KrJob::started, [=](KIO::Job *job) {
            // Note: the "trash" protocal should always have only one "/" after the "scheme:" part
            connect(job, &KIO::Job::result, [=]() { emit fileSystemChanged(QUrl("trash:/")); });
        });
    }

    krJobMan->manageJob(krJob);
}

void FileSystem::connectJob(KJob *job, const QUrl &destination)
{
    // (additional) direct refresh if on local fs because watcher is too slow
    const bool refresh = cleanUrl(destination) == _currentDirectory && isLocal();
    connect(job, &KIO::Job::result, this, [=](KJob* job) { slotJobResult(job, refresh); });
    connect(job, &KIO::Job::result, [=]() { emit fileSystemChanged(destination); });
}

bool FileSystem::showHiddenFiles()
{
    const KConfigGroup gl(krConfig, "Look&Feel");
    return gl.readEntry("Show Hidden", _ShowHidden);
}

void FileSystem::calcSpace(const QUrl &url, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                    unsigned long *totalDirs, bool *stop)
{
    if (url.isLocalFile()) {
        calcSpaceLocal(cleanUrl(url).path(), totalSize, totalFiles, totalDirs, stop);
    } else {
        calcSpaceKIO(url, totalSize, totalFiles, totalDirs, stop);
    }
}

void FileSystem::calcSpaceLocal(const QString &path, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                         unsigned long *totalDirs, bool *stop)
{
    if (stop && *stop)
        return;

    if (path == "/proc")
        return;

    QT_STATBUF stat_p; // KDE lstat is necessary as QFileInfo and KFileItem
    // if the name is wrongly encoded, then we zero the size out
    stat_p.st_size = 0;
    stat_p.st_mode = 0;
    QT_LSTAT(path.toLocal8Bit(), &stat_p); // reports wrong size for a symbolic link

    if (S_ISLNK(stat_p.st_mode) || !S_ISDIR(stat_p.st_mode)) { // single files are easy : )
        ++(*totalFiles);
        (*totalSize) += stat_p.st_size;
    } else { // handle directories avoid a nasty crash on un-readable dirs
        bool readable = ::access(path.toLocal8Bit(), R_OK | X_OK) == 0;
        if (!readable)
            return;

        QDir dir(path);
        if (!dir.exists())
            return;

        ++(*totalDirs);
        dir.setFilter(QDir::TypeMask | QDir::System | QDir::Hidden);
        dir.setSorting(QDir::Name | QDir::DirsFirst);

        // recurse on all the files in the directory
        QFileInfoList fileList = dir.entryInfoList();
        for (int k = 0; k != fileList.size(); k++) {
            if (*stop)
                return;
            QFileInfo qfiP = fileList[k];
            if (qfiP.fileName() != "." && qfiP.fileName() != "..")
                calcSpaceLocal(path + '/' + qfiP.fileName(), totalSize, totalFiles, totalDirs,
                               stop);
        }
    }
}

// TODO called from another thread, creating KIO jobs does not work here
void FileSystem::calcSpaceKIO(const QUrl &url, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                       unsigned long *totalDirs, bool *stop)
{
    return;

    if (stop && *stop)
        return;

    _calcKdsBusy = stop;
    _calcKdsTotalSize = totalSize;
    _calcKdsTotalFiles = totalFiles;
    _calcKdsTotalDirs = totalDirs;

    _calcStatBusy = true;
    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo); // thread problem here
    connect(statJob, &KIO::Job::result, this, &FileSystem::slotCalcStatResult);

    while (!(*stop) && _calcStatBusy) {
        usleep(1000);
    }

    if (_calcEntry.count() == 0)
        return; // statJob failed

    const KFileItem kfi(_calcEntry, url, true);
    if (kfi.isFile() || kfi.isLink()) {
        (*totalFiles)++;
        *totalSize += kfi.size();
        return;
    }

    KIO::DirectorySizeJob *directorySizeJob = KIO::directorySize(url);
    connect(directorySizeJob, &KIO::Job::result, this, &FileSystem::slotCalcKdsResult);

    while (!(*stop)) {
        // we are in a separate thread - so sleeping is OK
        usleep(1000);
    }
}

FileItem *FileSystem::createLocalFileItem(const QString &name, const QString &directory, bool virt)
{
    const QDir dir = QDir(directory);
    const QString path = dir.filePath(name);
    const QByteArray filePath = path.toLocal8Bit();

    QT_STATBUF stat_p;
    stat_p.st_size = 0;
    stat_p.st_mode = 0;
    stat_p.st_mtime = 0;
    stat_p.st_uid = 0;
    stat_p.st_gid = 0;
    QT_LSTAT(filePath.data(), &stat_p);
    const KIO::filesize_t size = stat_p.st_size;

    bool isDir = S_ISDIR(stat_p.st_mode);
    const bool isLink = S_ISLNK(stat_p.st_mode);

    QString linkDestination;
    bool brokenLink = false;
    if (isLink) {
        // find where the link is pointing to
        // the path of the symlink target cannot be longer than the file size of the symlink
        char buffer[stat_p.st_size];
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = readlink(filePath.data(), buffer, sizeof(buffer));
        if (bytesRead != -1) {
            linkDestination = QString::fromLocal8Bit(buffer, bytesRead); // absolute or relative
            const QFileInfo linkFile(dir, linkDestination);
            if (!linkFile.exists())
                brokenLink = true;
            else if (linkFile.isDir())
                isDir = true;
        } else {
            krOut << "Failed to read link: " << path;
        }
    }

    return new FileItem(virt ? path : name, QUrl::fromLocalFile(path), isDir,
                     size, stat_p.st_mode, stat_p.st_mtime,
                     stat_p.st_uid, stat_p.st_gid, QString(), QString(),
                     isLink, linkDestination, brokenLink);
}

FileItem *FileSystem::createFileItemFromKIO(const KIO::UDSEntry &entry, const QUrl &directory, bool virt)
{
    const KFileItem kfi(entry, directory, true, true);

    const QString name = kfi.text();
    // ignore un-needed entries
    if (name.isEmpty() || name == "." || name == "..") {
        return 0;
    }

    const QString localPath = kfi.localPath();
    const QUrl url = !localPath.isEmpty() ? QUrl::fromLocalFile(localPath) : kfi.url();
    const QString fname = virt ? url.toDisplayString() : name;

    // get file statistics...
    const time_t mtime = kfi.time(KFileItem::ModificationTime).toTime_t();
    const mode_t mode = kfi.mode() | kfi.permissions();
    // NOTE: we could get the mimetype (and file icon) from the kfileitem here but this is very
    // slow. Instead, the file item class has it's own (faster) way to determine the file type.

    // NOTE: "broken link" flag is always false, checking link destination existence is
    // considered to be too expensive
    return new FileItem(fname, url, kfi.isDir(),
                     kfi.size(), mode, mtime,
                     (uid_t) -1, (gid_t) -1, kfi.user(), kfi.group(),
                     kfi.isLink(), kfi.linkDest(), false,
                     kfi.ACL().asString(), kfi.defaultACL().asString());
}

// ==== protected slots ====

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

/// to be implemented
void FileSystem::slotCalcKdsResult(KJob *job)
{
    if (!job->error()) {
        KIO::DirectorySizeJob *kds = static_cast<KIO::DirectorySizeJob *>(job);
        *_calcKdsTotalSize += kds->totalSize();
        *_calcKdsTotalFiles += kds->totalFiles();
        *_calcKdsTotalDirs += kds->totalSubdirs();
    }
    *_calcKdsBusy = true;
}

void FileSystem::slotCalcStatResult(KJob *job)
{
    _calcEntry = job->error() ? KIO::UDSEntry() : static_cast<KIO::StatJob *>(job)->statResult();
    _calcStatBusy = false;
}

// ==== private ====

void FileSystem::clear(FileItemDict &fileItems)
{
    QHashIterator<QString, FileItem *> lit(fileItems);
    while (lit.hasNext()) {
        delete lit.next().value();
    }
    fileItems.clear();
}
