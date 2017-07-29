/***************************************************************************
                          defaultfilesystem.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net

 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DEFAULTFILESYSTEM_H
#define DEFAULTFILESYSTEM_H

#include "filesystem.h"

#include <QFileSystemWatcher>

#include <KCoreAddons/KDirWatch>


/**
 * @brief Default filesystem implementation supporting all KIO protocols
 *
 * This filesystem implementation allows file operations and listing for all supported KIO protocols
 *  (local and remote/network).
 *
 * Refreshing local directories is optimized for performance.
 *
 * NOTE: For detecting local file changes a filesystem watcher is used. It cannot be used for
 * refreshing the view after own file operations are performed because the detection is to slow
 * (~500ms delay between operation finished and watcher emits signals).
 *
 */
class DefaultFileSystem : public FileSystem {
    Q_OBJECT
public:
    DefaultFileSystem();

    void copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                           KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                           bool showProgressInfo = true,
                           JobMan::StartMode startMode = JobMan::Default) Q_DECL_OVERRIDE;
    void dropFiles(const QUrl &destination, QDropEvent *event) Q_DECL_OVERRIDE;

    void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                  const QString &dir = "") Q_DECL_OVERRIDE;
    void mkDir(const QString &name) Q_DECL_OVERRIDE;
    void rename(const QString &fileName, const QString &newName) Q_DECL_OVERRIDE;
    /// Return URL for file name - even if file does not exist.
    QUrl getUrl(const QString &name) const Q_DECL_OVERRIDE;
    bool canMoveToTrash(const QStringList &) const Q_DECL_OVERRIDE { return isLocal(); }

    QString mountPoint() const { return _mountPoint; }
    bool hasAutoUpdate() const Q_DECL_OVERRIDE { return !_watcher.isNull(); }
    void updateFilesystemInfo() Q_DECL_OVERRIDE;

protected:
    bool refreshInternal(const QUrl &origin, bool onlyScan) Q_DECL_OVERRIDE;

protected slots:
    /// Handle result after dir listing job is finished
    void slotListResult(KJob *job);
    /// Fill directory file list with new files from the dir lister
    void slotAddFiles(KIO::Job *job, const KIO::UDSEntryList &entries);
    /// URL redirection signal from dir lister
    void slotRedirection(KIO::Job *job, const QUrl &url);
    // React to filesystem changes nofified by watcher
    // NOTE: the path parameter can be the directory itself or files in this directory
    void slotWatcherCreated(const QString &path);
    void slotWatcherDirty(const QString &path);
    void slotWatcherDeleted(const QString &path);

private:
    bool refreshLocal(const QUrl &directory, bool onlyScan); // NOTE: this is very fast
    FileItem *createLocalFileItem(const QString &name);
    /// Returns the current path with symbolic links resolved
    QString realPath();
    static QUrl resolveRelativePath(const QUrl &url);

    QPointer<KDirWatch> _watcher; // dir watcher used to detect changes in the current dir
    bool _listError;              // for async operation, return list job result
    QString _mountPoint;          // the mount point of the current dir
};

#endif
