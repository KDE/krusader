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
	ftp_vfs(QString origin,QWidget* panel);
 ~ftp_vfs(){}
	
	// copy a file to the vfs (physical)
	void    vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "");
	// remove a file from the vfs (physical)
	void 		vfs_delFiles(QStringList *fileNames);	
	// return a path to the file
	QString vfs_getFile(QString name);
	KURL::List* vfs_getFiles(QStringList* names);
	// make dir
	void vfs_mkdir(QString name);
	// rename file
	void vfs_rename(QString fileName,QString newName);
	QString vfs_workingDir();
	// not implemented for ftp !
	void vfs_calcSpace(QString ,long long *,long *, long *){}
	
public slots:
	// recieve KDirLister  job.
	void slotAddFiles(KIO::Job * job, const KIO::UDSEntryList& entries);
  void slotRedirection(KIO::Job *, const KURL &url);
  void slotListResult(KIO::Job *job);
	// actually reads files and stats
	bool vfs_refresh(QString origin);

protected:
	QList<vfile>  vfs_files;    // list of pointers to vfile
  QList<vfile>  vfs_files2;   // list of pointers to vfile
  QList<vfile>  *vfs_filesP2;
  QString origin_backup;

  KURL vfs_url;	
  KURL vfs_url_backup;

  bool notConnected;

  QString loginName;
	QString password;
	QString hostName;
	int port;  		
};

#endif
