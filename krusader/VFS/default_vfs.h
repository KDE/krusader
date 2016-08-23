/***************************************************************************
                          default_vfs.h
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

#ifndef DEFAULT_VFS_H
#define DEFAULT_VFS_H

#include "vfs.h"

#include <QFileSystemWatcher>

#include <KCoreAddons/KDirWatch>

/**
 * @brief Default filesystem implementation supporting all KIO protocols
 *
 * This vfs implementation allows file operations and listing for all supported KIO protocols (local
 * and remote/network).
 *
 * Refreshing local directories is optimized for performance.
 */
class default_vfs : public vfs {
    Q_OBJECT
public:
    default_vfs();
    ~default_vfs() {}

    virtual void copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                           KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                           bool showProgressInfo = true) Q_DECL_OVERRIDE;
    virtual void dropFiles(const QUrl &destination, QDropEvent *event) Q_DECL_OVERRIDE;

    virtual void addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                              QString dir = "") Q_DECL_OVERRIDE;
    virtual void deleteFiles(const QStringList &fileNames,
                             bool forceDeletion = false) Q_DECL_OVERRIDE;
    virtual void mkDir(const QString &name) Q_DECL_OVERRIDE;
    virtual void rename(const QString &fileName, const QString &newName) Q_DECL_OVERRIDE;
    /// Return URL for file name - even if file does not exist.
    virtual QUrl getUrl(const QString &name) Q_DECL_OVERRIDE;

protected:
    virtual bool refreshInternal(const QUrl &origin, bool showHidden) Q_DECL_OVERRIDE;
    virtual bool ignoreRefresh() Q_DECL_OVERRIDE;

protected slots:
    /// Handle result after dir listing job is finished
    void slotListResult(KJob *job);
    /// Fill directory file list with new files from the dir lister
    void slotAddFiles(KIO::Job *job, const KIO::UDSEntryList &entries);
    /// URL redirection signal from dir lister
    void slotRedirection(const QUrl &url);
    // React to filesystem changes nofified by watcher
    // NOTE: the path parameter can be the directory itself or files in this directory
    void slotWatcherDirty(const QString &path);
    void slotWatcherDeleted(const QString &path);

private:
    void connectSourceVFS(KIO::Job *job, const QList<QUrl> urls);

    bool refreshLocal(const QUrl &directory); // NOTE: this is very fast
    vfile *createLocalVFile(const QString &name);

    QPointer<KDirWatch> _watcher; // dir watcher used to detect changes in the current dir
    bool _listError; // for async operation, return list job result
};

#endif
