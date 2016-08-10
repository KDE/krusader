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

#include <KCoreAddons/KDirWatch>

#include "preservingcopyjob.h"

/**
 * @brief Default filesystem implementation supporting all KIO protocols
 *
 * This vfs implementation allows file operations and listing of all supported KIO protocls (local,
 * virtual and remote/network).
 *
 */
class default_vfs : public vfs {
    Q_OBJECT
public:
    default_vfs(QObject *parent);
    ~default_vfs() {}

    virtual void vfs_addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode,
                              QObject *toNotify, QString dir = "", PreserveMode pmode = PM_DEFAULT) Q_DECL_OVERRIDE;
    virtual void vfs_drop(const QUrl &destination, QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void vfs_delFiles(const QStringList &fileNames,
                              bool reallyDelete = false) Q_DECL_OVERRIDE;
    virtual QList<QUrl> vfs_getFiles(const QStringList &names) Q_DECL_OVERRIDE;
    virtual QUrl vfs_getFile(const QString &name) Q_DECL_OVERRIDE;
    virtual void vfs_mkdir(const QString &name) Q_DECL_OVERRIDE;
    virtual void vfs_rename(const QString &fileName, const QString &newName) Q_DECL_OVERRIDE;
    virtual QString vfs_workingDir() Q_DECL_OVERRIDE {
        return vfs_origin.path();
    }

protected:
    virtual bool populateVfsList(const QUrl &origin, bool showHidden) Q_DECL_OVERRIDE;

protected slots:
    // handle result after job (except listing job!) finished
    void slotJobResult(KJob *job);
    // fill directory file list with new files from the dir lister
    void slotAddFiles(KIO::Job *job, const KIO::UDSEntryList &entries);
    // redirection signal from dir lister
    void slotRedirection(const QUrl &url);
    // handle result after dir listing job is finished
    void slotListResult(KJob *job);
    // react to filesystem changes nofified by watcher
    void slotWatcherDirty(const QString &path);
    void slotWatcherCreated(const QString &path);
    void slotWatcherDeleted(const QString &path);

private:
    KDirWatch *watcher; // dir watcher used to detect changes in the current dir
    bool listError; // for async operation, return list job result
};

#endif
