/***************************************************************************
                          virt_vfs.h  -  description
                             -------------------
    begin                : Fri Dec 5 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VIRT_VFS_H
#define VIRT_VFS_H

#include <kconfig.h>
#include <qhash.h>

#include "vfs.h"

/**
  *@author Shie Erlich & Rafi Yanai
  */

class virt_vfs : public vfs  {
Q_OBJECT
public: 
	virt_vfs(QObject* panel, bool quiet=false);
	~virt_vfs();
	
	/// Copy a file to the vfs (physical).
	void vfs_addFiles(KUrl::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "",  PreserveMode pmode = PM_DEFAULT );	
	/// Remove a file from the vfs (physical)
	void vfs_delFiles(QStringList *fileNames, bool reallyDelete = true);	
	/// Remove a file from the collection (only its link, not the file)
	void vfs_removeFiles(QStringList *fileNames);	
	/// Return a list of URLs for multiple files	
	KUrl::List* vfs_getFiles(QStringList* names);
	/// Return a URL to a single file	
	KUrl vfs_getFile(const QString& name);
	/// Create a new directory
	void vfs_mkdir(const QString& name);
	/// Rename file
	void vfs_rename(const QString& fileName,const QString& newName);
	/// Calculate the amount of space occupied by a file or directory (recursive).
	virtual void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop);
	
	/// Return the VFS working dir
	QString vfs_workingDir(){ return QString(); }
	
protected slots:
	void slotStatResult(KJob *job);

protected:
	/// Save the dictionary to file
	bool save();
	/// Restore the dictionary from file
	bool restore();	
	/// return the URLs DB
	KConfig*  getVirtDB();

	bool populateVfsList(const KUrl& origin, bool showHidden);
	vfile* stat(const KUrl& url);
	
	static QHash<QString, KUrl::List *> virtVfsDict;
	static KConfig* virt_vfs_db;
	bool busy;
	QString path;
	KIO::UDSEntry entry;
};

#endif
