/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

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
                           JobMan::StartMode startMode = JobMan::Default) override;
    void dropFiles(const QUrl &destination, QDropEvent *event) override;

    void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                  const QString &dir = "") override;
    void mkDir(const QString &name) override;
    void rename(const QString &fileName, const QString &newName) override;
    /// Return URL for file name - even if file does not exist.
    QUrl getUrl(const QString &name) const override;
    bool canMoveToTrash(const QStringList &) const override { return isLocal(); }

    QString mountPoint() const override { return _mountPoint; }
    bool hasAutoUpdate() const override { return !_watcher.isNull(); }
    void updateFilesystemInfo() override;

protected:
    bool refreshInternal(const QUrl &origin, bool onlyScan) override;

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
