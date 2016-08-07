/***************************************************************************
                          ftp_vfs.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  A vfs class that handels "ftp/samba" directory enteris
 inherits: vfs
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
#ifndef FTP_VFS_H
#define FTP_VFS_H

#include <KIOWidgets/KDirLister>

#include "vfs.h"

class ftp_vfs : public vfs
{
    Q_OBJECT
public:
    // the constructor simply uses the inherited constructor
    ftp_vfs(QObject* panel);
    ~ftp_vfs();

    /// Copy a file to the vfs (physical).
    virtual void vfs_addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode mode, QObject* toNotify, QString dir = "", PreserveMode pmode = PM_DEFAULT) Q_DECL_OVERRIDE;
    /// Handle file drop
    virtual void vfs_drop(const QUrl &destination, QDropEvent *event) Q_DECL_OVERRIDE;
    /// Remove a file from the vfs (physical)
    virtual void vfs_delFiles(const QStringList &fileNames, bool reallyDelete = false) Q_DECL_OVERRIDE;
    /// Return a list of URLs for multiple files
    virtual QList<QUrl> vfs_getFiles(const QStringList &names) Q_DECL_OVERRIDE;
    /// Return a URL to a single file
    virtual QUrl vfs_getFile(const QString& name) Q_DECL_OVERRIDE;
    /// Create a new directory
    virtual void vfs_mkdir(const QString& name) Q_DECL_OVERRIDE;
    /// Rename file
    virtual void vfs_rename(const QString& fileName, const QString& newName) Q_DECL_OVERRIDE;
    /// Return the VFS working dir
    QString vfs_workingDir() Q_DECL_OVERRIDE;

public slots:
    /// Handles new files from the dir lister
    void slotAddFiles(KIO::Job * job, const KIO::UDSEntryList& entries);
    /// Redirection signal handlers
    void slotRedirection(KIO::Job *, const QUrl &url);
    void slotPermanentRedirection(KIO::Job*, const QUrl &, const QUrl& newUrl);
    /// Called when the dir listing job is finished (for better or worst)
    void slotListResult(KJob *job);
    /// Active the dir listing job
    virtual bool populateVfsList(const QUrl &origin, bool showHidden) Q_DECL_OVERRIDE;

protected:
    QUrl origin_backup;         //< used to backup the old URL when refreshing to a new one,
    bool busy;
    bool listError;
};

#endif
