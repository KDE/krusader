/***************************************************************************
                          normal_vfs.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  A vfs class that handels "normal" directory enterys
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

#ifndef NORMAL_VFS
#define NORMAL_VFS

// QT includes
#include <qstring.h>
// KDE includes
#include <kfileitem.h>
#include <kdirwatch.h>
// Krusader includes
#include "vfs.h"
//#include "krdirwatch.h"

class normal_vfs : public vfs{
	Q_OBJECT
public:
	// the constructor simply uses the inherited constructor
	normal_vfs(QString origin,QWidget* panel);
 ~normal_vfs(){}
	
	// copy a file to the vfs (physical)
	void    vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify=0,QString dir="");	
	// remove a file from the vfs (physical)
	void 		vfs_delFiles(QStringList *fileNames);	
	// return a path to the file
	QString vfs_getFile(QString name);
	KURL::List* vfs_getFiles(QStringList* names);
	// make dir
	void vfs_mkdir(QString name);
	// rename file
	void vfs_rename(QString fileName,QString newName);
	// calculate space
	void vfs_calcSpace(QString name ,long long *totalSize,long *totalFiles, long *totalDirs);
	// return the working dir
	inline virtual QString vfs_workingDir() { return vfs_origin; }
	void blockSignals(bool block){ block? watcher.stopScan() : watcher.startScan() ; }

public slots:
	// actually reads files and stats
	bool vfs_refresh(QString origin);

protected:
	QList<vfile>  vfs_files;    // list of pointers to vfile	
	KDirWatch watcher;
};

#endif
