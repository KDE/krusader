/***************************************************************************
                       default_vfs.cpp
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

#include "default_vfs.h"

// QtCore
#include <QEventLoop>
#include <QByteArray>
#include <QDir>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KIO/DeleteJob>
#include <KIO/DropJob>
#include <KIO/JobUiDelegate>
#include <KIOCore/KFileItem>
#include <KIOCore/KProtocolManager>

#include "krpermhandler.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../MountMan/kmountman.h"

default_vfs::default_vfs(QObject* parent): vfs(parent), watcher(0)
{
    vfs_type = VFS_NORMAL;
}

void default_vfs::vfs_addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                               QObject *toNotify, QString dir)
{
    //if (watcher) watcher->stopScan();

    QUrl destination(vfs_origin);
    if (!dir.isEmpty()) {
        destination.setPath(QDir::cleanPath(destination.path() + '/' + dir));

        if (destination.scheme() == "tar" || destination.scheme() == "zip" || destination.scheme() == "krarc") {
            if (QDir(destination.adjusted(QUrl::StripTrailingSlash).path()).exists())
                destination.setScheme("file"); // if we get out from the archive change the protocol
        }
    }

    KIO::Job *job = 0;
    switch (mode) {
    case KIO::CopyJob::Copy:
        job = KIO::copy(fileUrls, destination);
        break;
    case KIO::CopyJob::Move:
        job = KIO::move(fileUrls, destination);
        break;
    case KIO::CopyJob::Link:
        job = KIO::link(fileUrls, destination);
        break;
    }

    connect(job, SIGNAL(result(KJob *)), this, SLOT(slotJobResult(KJob *)));
    if (toNotify && mode == KIO::CopyJob::Move) { // notify the other panel
        connect(job, SIGNAL(result(KJob *)), toNotify, SLOT(vfs_refresh(KJob *)));
    }
}

void default_vfs::vfs_drop(const QUrl &destination, QDropEvent *event)
{
    //if (watcher) watcher->stopScan();

    KIO::DropJob *job = KIO::drop(event, destination);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobResult(KJob *)));
    QObject *dropSource = event->source();
    if (dropSource) { // refresh source because files may have been removed
        connect(job, SIGNAL(result(KJob*)), dropSource, SLOT(vfs_refresh(KJob*)));
    }
}

void default_vfs::vfs_delFiles(const QStringList &fileNames, bool reallyDelete)
{
    //if (watcher) watcher->stopScan();

    // get absolute URLs for file names
    QList<QUrl> filesUrls = vfs_getFiles(fileNames);

    // create job: delete of move to trash ?
    KIO::Job *job;
    KConfigGroup group(krConfig, "General");
    if (!reallyDelete && group.readEntry("Move To Trash", _MoveToTrash)) {
        job = KIO::trash(filesUrls);
        emit trashJobStarted(job);
    } else {
        job = KIO::del(filesUrls);
    }
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobResult(KJob*)));
}

QUrl default_vfs::vfs_getFile(const QString& name)
{
    QUrl absoluteUrl(vfs_origin);
    absoluteUrl.setPath(absoluteUrl.path() + "/" + name);
    return absoluteUrl;
}

QList<QUrl> default_vfs::vfs_getFiles(const QStringList &names)
{
    QList<QUrl> urls;
    foreach (const QString &name, names) {
        urls.append(vfs_getFile(name));
    }
    return urls;
}

void default_vfs::vfs_mkdir(const QString& name)
{
    KIO::SimpleJob* job = KIO::mkdir(vfs_getFile(name));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotJobResult(KJob*)));
}

void default_vfs::vfs_rename(const QString& oldName, const QString& newName)
{
    QUrl oldUrl = vfs_getFile(oldName);
    QUrl newUrl = vfs_getFile(newName);
    KIO::Job *job = KIO::moveAs(oldUrl, newUrl, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob *)), this, SLOT(slotJobResult(KJob *)));
}

// ==== protected ====

bool default_vfs::populateVfsList(const QUrl &origin, bool showHidden)
{
    delete watcher; // stop watching the old dir

    QString errorMsg;
    if (!origin.isValid()) {
        errorMsg = i18n("Malformed URL:\n%1", origin.url());
    } else if (!KProtocolManager::supportsListing(origin)) {
        errorMsg = i18n("Protocol not supported by Krusader:\n%1", origin.url());
    }
    if (!errorMsg.isEmpty()) {
        if (!quietMode) {
            emit error(errorMsg);
        }
        return false;
    }

    if (origin.isLocalFile()) {
        QString path = KrServices::urlToLocalPath(origin);

#ifdef Q_WS_WIN
        if (!path.contains("/")) { // change C: to C:/
            path = path + QString("/");
        }
#endif

        // check if the new origin exists
        if (!QDir(path).exists()) {
            if (!quietMode)
                emit error(i18n("The folder %1 does not exist.", path));
            return false;
        }

        KConfigGroup group(krConfig, "Advanced");
        if (group.readEntry("AutoMount", _AutoMount) && !mountMan.isNull())
            mountMan->autoMount(path);

        // set the origin...
        vfs_origin = origin;
        vfs_origin.setPath(path);
        vfs_origin.setPath(QDir::cleanPath(vfs_origin.path()));
    } else {
        vfs_origin = origin.adjusted(QUrl::StripTrailingSlash);
    }

    // start the listing job
    KIO::ListJob *job = KIO::listDir(vfs_origin, KIO::HideProgressInfo, showHidden);
    connect(job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)), this,
            SLOT(slotAddFiles(KIO::Job *, const KIO::UDSEntryList &)));
    connect(job, &KIO::ListJob::redirection,
            [=](KIO::Job *, const QUrl &url) { slotRedirection(url); });
    connect(job, &KIO::ListJob::permanentRedirection,
            [=](KIO::Job *, const QUrl &, const QUrl &url) { slotRedirection(url); });
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotListResult(KJob*)));

    if(!parentWindow.isNull()) {
        KIO::JobUiDelegate *ui = static_cast<KIO::JobUiDelegate*>(job->uiDelegate());
        ui->setWindow(parentWindow);
    }

    if (!quietMode) {
        emit startJob(job);
    }

    listError = false;
    // ugly: we have to wait here until the list job is finished
    QEventLoop eventLoop;
    connect(job, SIGNAL(result(KJob*)), &eventLoop, SLOT(quit()));
    eventLoop.exec(); // blocking until quit()

    if (panelConnected && vfs_origin.isLocalFile()) {
        // start watching the new dir for file changes
        watcher = new KDirWatch(this);
        connect(watcher, SIGNAL(dirty(const QString&)), this, SLOT(slotWatcherDirty(const QString&)));
        connect(watcher, SIGNAL(created(const QString&)), this, SLOT(slotWatcherCreated(const QString&)));
        connect(watcher, SIGNAL(deleted(const QString&)), this, SLOT(slotWatcherDeleted(const QString&)));
        watcher->addDir(vfs_getOrigin().toLocalFile(), KDirWatch::WatchFiles);
        watcher->startScan(false);
    }

    return !listError;
}

// ==== slots ====

void default_vfs::slotJobResult(KJob *job)
{
    if (job->error() && job->uiDelegate()) {
        job->uiDelegate()->showErrorMessage();
    }

    vfs_refresh();

    return;
}

void default_vfs::slotAddFiles(KIO::Job *, const KIO::UDSEntryList& entries)
{
    int rwx = -1;

    QString prot = vfs_origin.scheme();
    if (prot == "krarc" || prot == "tar" || prot == "zip")
        rwx = PERM_ALL;

    // add all files to the vfs
    for (const KIO::UDSEntry entry : entries) {
        KFileItem kfi(entry, vfs_origin, true, true);
        vfile *temp;

        // get file statistics
        QString name = kfi.text();
        // ignore un-needed entries
        // TODO do not filter ".." here
        if (name.isEmpty() || name == "." || name == "..")
            continue;

        KIO::filesize_t size = kfi.size();
        time_t mtime = kfi.time(KFileItem::ModificationTime).toTime_t();
        bool symLink = kfi.isLink();
        mode_t mode = kfi.mode() | kfi.permissions();
        QString perm = KRpermHandler::mode2QString(mode);
        // set the mimetype
        QString mime = kfi.mimetype();
        QString symDest = "";
        if (symLink) {
            symDest = kfi.linkDest();
            if (kfi.isDir())
                perm[0] = 'd';
        }

        // create a new virtual file object
        if (kfi.user().isEmpty())
            temp = new vfile(name, size, perm, mtime, symLink, false, getuid(), getgid(), mime,
                             symDest, mode, rwx);
        else {
            QString currentUser = vfs_origin.userName();
            if (currentUser.contains("@")) /* remove the FTP proxy tags from the username */
                currentUser.truncate(currentUser.indexOf('@'));
            if (currentUser.isEmpty()) {
                if (vfs_origin.host().isEmpty()) {
                    currentUser = KRpermHandler::uid2user(getuid());
                } else {
                    currentUser = ""; // empty, but not QString()
                }
            }
            temp = new vfile(name, size, perm, mtime, symLink, false, kfi.user(), kfi.group(),
                             currentUser, mime, symDest, mode, rwx, kfi.ACL().asString(),
                             kfi.defaultACL().asString());
        }

        if (!kfi.localPath().isEmpty()) {
            temp->vfile_setUrl(QUrl::fromLocalFile(kfi.localPath()));
        } else {
            temp->vfile_setUrl(kfi.url());
        }
        temp->vfile_setIcon(kfi.iconName());
        foundVfile(temp);
    }
}

void default_vfs::slotRedirection(const QUrl &url)
{
    // update the origin
    vfs_origin = url.adjusted(QUrl::StripTrailingSlash);
}

void default_vfs::slotListResult(KJob *job)
{
    if (job && job->error()) {
        // we failed to refresh
        listError = true;
        // display error message
        if (!quietMode) {
            emit error(job->errorString());
        }
    }
}

void default_vfs::slotWatcherDirty(const QString& path)
{
    // TODO
}

void default_vfs::slotWatcherCreated(const QString& path)
{
    // TODO
}

void default_vfs::slotWatcherDeleted(const QString& path)
{
    // TODO
}
