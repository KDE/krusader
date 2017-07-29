/***************************************************************************
                       defaultfilesystem.cpp
                   -------------------
    copyright            : (C) 2000 by Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------

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

#include "defaultfilesystem.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QEventLoop>

#include <KConfigCore/KSharedConfig>
#include <KCoreAddons/KUrlMimeData>
#include <KI18n/KLocalizedString>
#include <KIO/DropJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/ListJob>
#include <KIOCore/KDiskFreeSpaceInfo>
#include <KIOCore/KFileItem>
#include <KIOCore/KMountPoint>
#include <KIOCore/KProtocolManager>
#include <kio_version.h>

#include "fileitem.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../JobMan/krjob.h"

DefaultFileSystem::DefaultFileSystem(): FileSystem(), _watcher()
{
    _type = FS_DEFAULT;
}

void DefaultFileSystem::copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                            KIO::CopyJob::CopyMode mode, bool showProgressInfo,
                            JobMan::StartMode startMode)
{
    // resolve relative path before resolving symlinks
    const QUrl dest = resolveRelativePath(destination);

    KIO::JobFlags flags = showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;

    KrJob *krJob = KrJob::createCopyJob(mode, urls, dest, flags);
    // destination can be a full path with filename when copying/moving a single file
    const QUrl destDir = dest.adjusted(QUrl::RemoveFilename);
    connect(krJob, &KrJob::started, this, [=](KIO::Job *job) { connectJobToDestination(job, destDir); });
    if (mode == KIO::CopyJob::Move) { // notify source about removed files
        connect(krJob, &KrJob::started, this, [=](KIO::Job *job) { connectJobToSources(job, urls); });
    }

    krJobMan->manageJob(krJob, startMode);
}

void DefaultFileSystem::dropFiles(const QUrl &destination, QDropEvent *event)
{
    qDebug() << "destination=" << destination;

    // resolve relative path before resolving symlinks
    const QUrl dest = resolveRelativePath(destination);

    KIO::DropJob *job = KIO::drop(event, dest);
#if KIO_VERSION >= QT_VERSION_CHECK(5, 30, 0)
    // NOTE: a DropJob "starts" with showing a menu. If the operation is choosen (copy/move/link)
    // the actual CopyJob starts automatically - we cannot manage the start of the CopyJob (see
    // documentation for KrJob)
    connect(job, &KIO::DropJob::copyJobStarted, this, [=](KIO::CopyJob *kJob) {
        connectJobToDestination(job, dest); // now we have to refresh the destination

        KrJob *krJob = KrJob::createDropJob(job, kJob);
        krJobMan->manageStartedJob(krJob, kJob);
        if (kJob->operationMode() == KIO::CopyJob::Move) { // notify source about removed files
            connectJobToSources(kJob, kJob->srcUrls());
        }
    });
#else
    // NOTE: DropJob does not provide information about the actual user choice
    // (move/copy/link/abort). We have to assume the worst (move)
    connectJob(job, dest);
    connectSourceFileSystem(job, KUrlMimeData::urlsFromMimeData(event->mimeData()));
#endif
}

void DefaultFileSystem::addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                                 const QString &dir)
{
    QUrl destination(_currentDirectory);
    if (!dir.isEmpty()) {
        destination.setPath(QDir::cleanPath(destination.path() + '/' + dir));
        const QString scheme = destination.scheme();
        if (scheme == "tar" || scheme == "zip" || scheme == "krarc") {
            if (QDir(destination.path()).exists())
                // if we get out from the archive change the protocol
                destination.setScheme("file");
        }
    }

    destination = ensureTrailingSlash(destination); // destination is always a directory
    copyFiles(fileUrls, destination, mode);
}

void DefaultFileSystem::mkDir(const QString &name)
{
    KIO::SimpleJob* job = KIO::mkdir(getUrl(name));
    connectJobToDestination(job, currentDirectory());
}

void DefaultFileSystem::rename(const QString &oldName, const QString &newName)
{
    const QUrl oldUrl = getUrl(oldName);
    const QUrl newUrl = getUrl(newName);
    KIO::Job *job = KIO::moveAs(oldUrl, newUrl, KIO::HideProgressInfo);
    connectJobToDestination(job, currentDirectory());

    KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Rename, {oldUrl}, newUrl, job);
}

QUrl DefaultFileSystem::getUrl(const QString& name) const
{
    // NOTE: on non-local fs file URL does not have to be path + name!
    FileItem *fileItem = getFileItem(name);
    if (fileItem)
        return fileItem->getUrl();

    QUrl absoluteUrl(_currentDirectory);
    absoluteUrl.setPath(absoluteUrl.path() + '/' + name);
    return absoluteUrl;
}

void DefaultFileSystem::updateFilesystemInfo()
{
    if (!KConfigGroup(krConfig, "Look&Feel").readEntry("ShowSpaceInformation", true)) {
        _mountPoint = "";
        emit fileSystemInfoChanged(i18n("Space information disabled"), "", 0, 0);
        return;
    }

    // TODO get space info for trash:/ with KIO spaceInfo job
    if (!_currentDirectory.isLocalFile()) {
        _mountPoint = "";
        emit fileSystemInfoChanged(i18n("No space information on non-local filesystems"), "", 0, 0);
        return;
    }

    const QString path = _currentDirectory.path();
    const KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(path);
    if (!info.isValid()) {
        _mountPoint = "";
        emit fileSystemInfoChanged(i18n("Space information unavailable"), "", 0, 0);
        return;
    }
    _mountPoint = info.mountPoint();

    const KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByPath(path);
    const QString fsType = mountPoint ? mountPoint->mountType() : "";

    emit fileSystemInfoChanged("", fsType, info.size(), info.available());
}

// ==== protected ====

bool DefaultFileSystem::refreshInternal(const QUrl &directory, bool onlyScan)
{
    qDebug() << "refresh internal to URL=" << directory.toDisplayString();
    if (!KProtocolManager::supportsListing(directory)) {
        emit error(i18n("Protocol not supported by Krusader:\n%1", directory.url()));
        return false;
    }

    delete _watcher; // stop watching the old dir

    if (directory.isLocalFile()) {
        qDebug() << "start local refresh to URL=" << directory.toDisplayString();
        // we could read local directories with KIO but using Qt is a lot faster!
        return refreshLocal(directory, onlyScan);
    }

    _currentDirectory = cleanUrl(directory);

    // start the listing job
    KIO::ListJob *job = KIO::listDir(_currentDirectory, KIO::HideProgressInfo, showHiddenFiles());
    connect(job, &KIO::ListJob::entries, this, &DefaultFileSystem::slotAddFiles);
    connect(job, &KIO::ListJob::redirection, this, &DefaultFileSystem::slotRedirection);
    connect(job, &KIO::ListJob::permanentRedirection, this, &DefaultFileSystem::slotRedirection);
    connect(job, &KIO::Job::result, this, &DefaultFileSystem::slotListResult);

    // ensure connection credentials are asked only once
    if(!parentWindow.isNull()) {
        KIO::JobUiDelegate *ui = static_cast<KIO::JobUiDelegate*>(job->uiDelegate());
        ui->setWindow(parentWindow);
    }

    emit refreshJobStarted(job);

    _listError = false;
    // ugly: we have to wait here until the list job is finished
    QEventLoop eventLoop;
    connect(job, &KJob::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); // blocking until quit()

    return !_listError;
}

// ==== protected slots ====

void DefaultFileSystem::slotListResult(KJob *job)
{
    qDebug() << "got list result";
    if (job && job->error()) {
        // we failed to refresh
        _listError = true;
        qDebug() << "error=" << job->errorString() << "; text=" << job->errorText();
        emit error(job->errorString()); // display error message (in panel)
    }
}

void DefaultFileSystem::slotAddFiles(KIO::Job *, const KIO::UDSEntryList& entries)
{
    for (const KIO::UDSEntry entry : entries) {
        FileItem *fileItem = FileSystem::createFileItemFromKIO(entry, _currentDirectory);
        if (fileItem) {
            addFileItem(fileItem);
        }
    }
}

void DefaultFileSystem::slotRedirection(KIO::Job *job, const QUrl &url)
{
   qDebug() << "redirection to URL=" << url.toDisplayString();

   // some protocols (zip, tar) send redirect to local URL without scheme
   const QUrl newUrl = preferLocalUrl(url);

   if (newUrl.scheme() != _currentDirectory.scheme()) {
       // abort and start over again,
       // some protocols (iso, zip, tar) do this on transition to local fs
       job->kill();
       _isRefreshing = false;
       refresh(newUrl);
       return;
   }

    _currentDirectory = cleanUrl(newUrl);
}

void DefaultFileSystem::slotWatcherCreated(const QString& path)
{
    qDebug() << "path created (doing nothing): " << path;
}

void DefaultFileSystem::slotWatcherDirty(const QString& path)
{
    qDebug() << "path dirty: " << path;
    if (path == realPath()) {
        // this happens
        //   1. if a directory was created/deleted/renamed inside this directory.
        //   2. during and after a file operation (create/delete/rename/touch) inside this directory
        // KDirWatcher doesn't reveal the name of changed directories and we have to refresh.
        // (QFileSystemWatcher in Qt5.7 can't help here either)
        refresh();
        return;
    }

    const QString name = QUrl::fromLocalFile(path).fileName();

    FileItem *fileItem = getFileItem(name);
    if (!fileItem) {
        qWarning() << "file not found (unexpected), path=" << path;
        // this happens at least for cifs mounted filesystems: when a new file is created, a dirty
        // signal with its file path but no other signals are sent (buggy behaviour of KDirWatch)
        refresh();
        return;
    }

    // we have an updated file..
    FileItem *newFileItem = createLocalFileItem(name);
    addFileItem(newFileItem);
    emit updatedFileItem(newFileItem);

    delete fileItem;
}

void DefaultFileSystem::slotWatcherDeleted(const QString& path)
{
    qDebug() << "path deleted: " << path;
    if (path != realPath()) {
        // ignore deletion of files here, a 'dirty' signal will be send anyway
        return;
    }

    // the current directory was deleted. Try a refresh, which will fail. An error message will
    // be emitted and the empty (non-existing) directory remains.
    refresh();
}

bool DefaultFileSystem::refreshLocal(const QUrl &directory, bool onlyScan) {
    const QString path = KrServices::urlToLocalPath(directory);

#ifdef Q_WS_WIN
    if (!path.contains("/")) { // change C: to C:/
        path = path + QString("/");
    }
#endif

    // check if the new directory exists
    if (!QDir(path).exists()) {
        emit error(i18n("The folder %1 does not exist.", path));
        return false;
    }

    // mount if needed
    emit aboutToOpenDir(path);

    // set the current directory...
    _currentDirectory = directory;
    _currentDirectory.setPath(QDir::cleanPath(_currentDirectory.path()));

    // Note: we are using low-level Qt functions here.
    // It's around twice as fast as using the QDir class.

    QT_DIR* dir = QT_OPENDIR(path.toLocal8Bit());
    if (!dir) {
        emit error(i18n("Cannot open the folder %1.", path));
        return false;
    }

    // change directory to the new directory
    const QString savedDir = QDir::currentPath();
    if (!QDir::setCurrent(path)) {
        emit error(i18nc("%1=folder path", "Access to %1 denied", path));
        QT_CLOSEDIR(dir);
        return false;
    }

    QT_DIRENT* dirEnt;
    QString name;
    const bool showHidden = showHiddenFiles();
    while ((dirEnt = QT_READDIR(dir)) != NULL) {
        name = QString::fromLocal8Bit(dirEnt->d_name);

        // show hidden files?
        if (!showHidden && name.left(1) == ".") continue;
        // we don't need the "." and ".." entries
        if (name == "." || name == "..") continue;

        FileItem* temp = createLocalFileItem(name);
        addFileItem(temp);
    }
    // clean up
    QT_CLOSEDIR(dir);
    QDir::setCurrent(savedDir);

    if (!onlyScan) {
        // start watching the new dir for file changes
        _watcher = new KDirWatch(this);
        // if the current dir is a link path the watcher needs to watch the real path - and signal
        // parameters will be the real path
        _watcher->addDir(realPath(), KDirWatch::WatchFiles);
        connect(_watcher.data(), &KDirWatch::dirty, this, &DefaultFileSystem::slotWatcherDirty);
        // NOTE: not connecting 'created' signal. A 'dirty' is send after that anyway
        //connect(_watcher, SIGNAL(created(QString)), this, SLOT(slotWatcherCreated(QString)));
        connect(_watcher.data(), &KDirWatch::deleted, this, &DefaultFileSystem::slotWatcherDeleted);
        _watcher->startScan(false);
    }

    return true;
}

FileItem *DefaultFileSystem::createLocalFileItem(const QString &name)
{
    return FileSystem::createLocalFileItem(name, _currentDirectory.path());
}

QString DefaultFileSystem::DefaultFileSystem::realPath()
{
    return QDir(_currentDirectory.toLocalFile()).canonicalPath();
}

QUrl DefaultFileSystem::resolveRelativePath(const QUrl &url)
{
    // if e.g. "/tmp/bin" is a link to "/bin",
    // resolve "/tmp/bin/.." to "/tmp" and not "/"
    return url.adjusted(QUrl::NormalizePathSegments);
}
