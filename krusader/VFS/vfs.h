/***************************************************************************
	  	                        vfs.h
  	                    -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
  -------------------------------------------------------------------------
   the vfs class is an extendable class which by itself does (almost)
   nothing. other VFSs like the normal_vfs inherits from this class and
   make it possible to use a consistent API for all types of VFSs.

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

#ifndef VFS_H
#define VFS_H

// QT includes
#include <qstring.h>
#include <qlist.h>
#include <qobject.h>
// KDE includes
#include <kurl.h>
#include <kio/jobclasses.h>
// Krusader includes
#include "vfile.h"

class vfs: public QObject{
	Q_OBJECT
public: 	
	// service functions
  inline void vfs_addToList(vfile *data){ vfs_filesP->append(data); }
  inline void vfs_removeFromList(vfile *data){ vfs_filesP->remove(data); }
	QString 		dateTime2QString(const QDateTime& datetime);
  QString 		round(int i);
	QString     month2Qstring(QString month);
  // create a vfs from origin
	vfs(QObject* panel, bool quiet=false);

	virtual			~vfs(){}
	// copy a file to the vfs (physical)
	virtual void vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QWidget* toNotify,QString dir = "")=0;	
	// remove a file from the vfs (physical)
	virtual void vfs_delFiles(QStringList *fileNames)=0;	
	// return a path to the file
	virtual KURL::List* vfs_getFiles(QStringList* names)=0;
	virtual QString vfs_getFile(QString name)=0;
	// make dir
	virtual void vfs_mkdir(QString name)=0;
	// rename file
	virtual void vfs_rename(QString fileName,QString newName)=0;
	// calculate space
	virtual void vfs_calcSpace(QString name ,long long *totalSize,long *totalFiles, long *totalDirs)=0;
	// return the working dir
	virtual QString vfs_workingDir()=0;
	virtual void blockSignals(bool block){ QObject::blockSignals(block); }	
	// check the write permission
  virtual bool vfs_isWritable() { return isWritable; }

	long vfs_totalSize();             // the total size of FILES in the vfs,
	vfile* vfs_search(QString name);  // return vfile* or 0 if not found
	
	inline   int  vfs_noOfFiles() 	 	{ return vfs_filesP->count(); 	}
	virtual  inline	 QString vfs_getOrigin() 	{ return vfs_origin;    }	
	inline 	 QString vfs_getType()	 	{ return vfs_type; 						  }
  inline   bool vfs_error()         { return error;                 }

	inline   vfile* vfs_getFirstFile(){
	                    if(vfs_filesP->isEmpty()) return 0;
	                    else return (vfs_filesP->first());	}
	inline   vfile* vfs_getNextFile()	{	return (vfs_filesP->next());  }

public slots:
	// actually reads files and stats
	virtual bool vfs_refresh(QString origin)=0;
	virtual bool vfs_refresh(KIO::Job* job=0);

signals: 	
  void startUpdate();
  void endUpdate();

protected: // allows derived classes to use these fields
	QString		  	vfs_type;			// the vfs type;
	QString      vfs_origin;	 	// the path or file the VFS originates from
 	QList<vfile> *vfs_filesP;
	bool error;
	bool quietMode;   // if true the vfs won't display error messages or emit signals
	bool isWritable;	// true if it's writable
};

#endif
