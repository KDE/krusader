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
#ifndef FTP_VFS
#define FTP_VFS

// KDE includes
#include <kdirlister.h>
// Krusader includes
#include "vfs.h"

class ftp_vfs : public vfs{
	Q_OBJECT
public:
	// the constructor simply uses the inherited constructor
	ftp_vfs(QObject* panel);
 ~ftp_vfs(){}

	/// Copy a file to the vfs (physical).
	virtual void vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "");
	/// Remove a file from the vfs (physical)
	virtual void vfs_delFiles(QStringList *fileNames);
	/// Return a list of URLs for multiple files
	virtual KURL::List* vfs_getFiles(QStringList* names);
	/// Return a URL to a single file
	virtual KURL vfs_getFile(const QString& name);
	/// Create a new directory
	virtual void vfs_mkdir(const QString& name);
	/// Rename file
	virtual void vfs_rename(const QString& fileName,const QString& newName);
	/// Calculate the amount of space occupied by a file or directory (recursive).
  /// Not implemted (yet) in ftp_vfs
	virtual void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop = 0);

	/// Return the VFS working dir	
	QString vfs_workingDir();
	
public slots:
	/// Handles new files from the dir lister
	void slotAddFiles(KIO::Job * job, const KIO::UDSEntryList& entries);
	/// Redirection signal handler
	void slotRedirection(KIO::Job *, const KURL &url);
	/// Called when the dir listing job is finished (for better or worst)
	void slotListResult(KIO::Job *job);
	/// Active the dir listing job
	bool vfs_refresh(const KURL& origin);

  void startLister();

protected:
	QList<vfile>  vfs_files;    //< List of pointers to vfile.
  //QList<vfile>  vfs_files2;   //< The second list of pointers to vfiles.
  //QList<vfile>  *vfs_filesP2; //< pointer to the second list
  KURL origin_backup;         //< used to backup the old URL when refreshing to a new one,

  bool busy;
};

#endif
