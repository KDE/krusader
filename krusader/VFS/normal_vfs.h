/***************************************************************************
                          normal_vfs.h
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

#ifndef NORMAL_VFS_H
#define NORMAL_VFS_H

#include <QtCore/QString>
#include <QTimer>

#include <kfileitem.h>
#include <kdirwatch.h>
#include <kurl.h>

#include "vfs.h"

/**
 * The normal_vfs class is Kruasder implemention for local directories.
 * As this is the most common case, we try to make it as fast and efficient
 * as possible.
 */
class normal_vfs : public vfs
{
    Q_OBJECT
public:
    // the constructor simply uses the inherited constructor
    normal_vfs(QObject* panel);
    ~normal_vfs() {
        if (watcher) delete watcher;
    }

    /// Copy a file to the vfs (physical).
    virtual void vfs_addFiles(KUrl::List *fileUrls, KIO::CopyJob::CopyMode mode, QObject* toNotify, QString dir = "", PreserveMode pmode = PM_DEFAULT);
    /// Remove a file from the vfs (physical)
    virtual void vfs_delFiles(QStringList *fileNames, bool reallyDelete = false);
    /// Return a list of URLs for multiple files
    virtual KUrl::List* vfs_getFiles(QStringList* names);
    /// Return a URL to a single file
    virtual KUrl vfs_getFile(const QString& name);
    /// Create a new directory
    virtual void vfs_mkdir(const QString& name);
    /// Rename file
    virtual void vfs_rename(const QString& fileName, const QString& newName);

    /// return the VFS working dir
    virtual QString vfs_workingDir() {
#ifdef Q_OS_WIN
        
        QString path = vfs_origin.toLocalFile();
        if(path.endsWith('/')) path.chop(1);
        return path;
#else
        return vfs_origin.path(KUrl::RemoveTrailingSlash);
#endif
    }

    /// Get ACL permissions
    static void getACL(vfile *file, QString &acl, QString &defAcl);

public slots:
    void vfs_slotRefresh();
    void vfs_slotDirty(const QString& path);
    void vfs_slotCreated(const QString& path);
    void vfs_slotDeleted(const QString& path);

protected:
    /// Re-reads files and stats and fills the vfile list
    virtual bool populateVfsList(const KUrl& origin, bool showHidden);

    QTimer refreshTimer;         //< Timer to exclude sudden refreshes
    KDirWatch *watcher;          //< The internal dir watcher - use to detect changes in directories
    vfile*   vfileFromName(const QString& name, char * d_name);

private:
    bool burstRefresh(const QString &path);
    static QString getACL(const QString & path, int type);
};

#endif
