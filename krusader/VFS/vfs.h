/***************************************************************************
	  	                        vfs.h
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

#ifndef VFS_H
#define VFS_H

// QT includes
#include <qstring.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qdict.h>
// KDE includes
#include <kurl.h>
#include <kio/jobclasses.h>
// Krusader includes
#include "vfile.h"

/**
 * The vfs class is an extendable class which by itself does (almost)
 * nothing. other VFSs like the normal_vfs inherits from this class and
 * make it possible to use a consistent API for all types of VFSs.
 */
class vfs: public QObject{
	Q_OBJECT
public:
	enum VFS_TYPE{ERROR=0,NORMAL,FTP,TEMP,VIRT};

	/**
	 * Creates a vfs.
	 * @param panel the panel father. the VFS will connect it's signals to this object.
	 * @param quiet if true, the VFS will not display error messages or emit signals
	 */
	vfs(QObject* panel, bool quiet=false);
	virtual			~vfs(){}

	/// Copy a file to the vfs (physical).
	virtual void vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "")=0;	
	/// Remove a file from the vfs (physical)
	virtual void vfs_delFiles(QStringList *fileNames)=0;	
	/// Return a list of URLs for multiple files	
	virtual KURL::List* vfs_getFiles(QStringList* names)=0;
	/// Return a URL to a single file	
	virtual KURL vfs_getFile(const QString& name)=0;
	/// Create a new directory
	virtual void vfs_mkdir(const QString& name)=0;
	/// Rename file
	virtual void vfs_rename(const QString& fileName,const QString& newName)=0;
	/// Calculate the amount of space occupied by a file or directory (recursive).
	virtual void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop = 0)=0;

	/// Return the VFS working dir
	virtual QString vfs_workingDir()=0;
	/// Return true if the VFS url is writable
	virtual bool vfs_isWritable() { return isWritable; }
	/// Return vfile* or 0 if not found
	inline vfile* vfs_search(const QString& name){ return (*vfs_filesP)[name]; } 

	/// The total size of all the files in the VFS,
	KIO::filesize_t vfs_totalSize();
	/// The number of files in the VFS
	inline unsigned long vfs_noOfFiles() { return vfs_filesP->count(); }
	/// Returns the VFS url.
	inline KURL vfs_getOrigin()          { return vfs_origin;          }
	// Return the VFS type.
	inline VFS_TYPE vfs_getType()        { return vfs_type;            }
	/// Return the first file in the VFS and set the internal iterator to the beginning of the list.
	inline vfile* vfs_getFirstFile(){ return (vfileIterator ? vfileIterator->toFirst() : 0); }
	/// Return the the next file in the list and advance the iterator.
	inline vfile* vfs_getNextFile() { return (vfileIterator ? ++(*vfileIterator) : 0);  }
    // KDE FTP proxy bug correction
    static KURL fromPathOrURL( const QString &originIn );


public slots:
	/// Re-reads files and stats and fills the vfile list
	virtual bool vfs_refresh(const KURL& origin)=0;
	/// Used to refresh the VFS when a job finishs. it calls the refresh() slot
  /// or display a error message if the job fails
	virtual bool vfs_refresh(KIO::Job* job);
	//virtual bool vfs_refresh(KIO::Job* job);
	virtual bool vfs_refresh(){ return vfs_refresh(vfs_getOrigin()); }

signals: 	
	void startUpdate(); //< emitted when the VFS starts to refresh its list of vfiles.

protected:
	/// Set the vfile list pointer
	void setVfsFilesP(QDict<vfile>* dict);
	/// clear and delete all current vfiles
	inline void clear(){ vfs_filesP->clear(); }
	/// Add a new vfile to the list.
	inline void addToList(vfile *data){ vfs_filesP->insert(data->vfile_getName(),data); }
	/// Deletes a vfile from the list.
	inline void removeFromList(vfile *data){ vfs_filesP->remove(data->vfile_getName()); }

	VFS_TYPE      vfs_type;     //< the vfs type.
	KURL          vfs_origin;   //< the path or file the VFS originates from.
	bool quietMode;             //< if true the vfs won't display error messages or emit signals
	bool isWritable;            //< true if it's writable

private:
	QDict<vfile>*  vfs_filesP;    //< Point to a lists of virtual files (vfile).
	QDictIterator<vfile>* vfileIterator; //< Point to a dictionary of virtual files (vfile).
};

#endif
