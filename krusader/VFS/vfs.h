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
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QHash>
// KDE includes
#include <kurl.h>
#include <kio/jobclasses.h>
// Krusader includes
#include "vfile.h"
#include "preservingcopyjob.h"
#include "krquery.h"

/**
 * The vfs class is an extendable class which by itself does (almost)
 * nothing. other VFSs like the normal_vfs inherits from this class and
 * make it possible to use a consistent API for all types of VFSs.
 */
class vfs: public QObject{
	Q_OBJECT
public:
	typedef QHash<QString, vfile *> vfileDict;	
	enum VFS_TYPE{VFS_ERROR=0,VFS_NORMAL,VFS_FTP,VFS_VIRT};

	/**
	 * Creates a vfs.
	 * @param panel the panel father. the VFS will connect it's signals to this object.
	 * @param quiet if true, the VFS will not display error messages
	 */
	vfs(QObject* panel, bool quiet=false);
	virtual			~vfs();

	/// Copy a file to the vfs (physical).
	virtual void vfs_addFiles(KUrl::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir = "", PreserveMode pmode = PM_DEFAULT)=0;	
	/// Remove a file from the vfs (physical)
	virtual void vfs_delFiles(QStringList *fileNames, bool reallyDelete=false)=0;	
	/// Return a list of URLs for multiple files	
	virtual KUrl::List* vfs_getFiles(QStringList* names)=0;
	/// Return a URL to a single file	
	virtual KUrl vfs_getFile(const QString& name)=0;
	/// Create a new directory
	virtual void vfs_mkdir(const QString& name)=0;
	/// Rename file
	virtual void vfs_rename(const QString& fileName,const QString& newName)=0;
	/// Calculate the amount of space occupied by a file or directory (recursive).
	virtual void vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop);
	/// Calculate the amount of space occupied by a local file or directory (recursive).
	virtual void vfs_calcSpaceLocal(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop);

	/// Return the VFS working dir
	virtual QString vfs_workingDir()=0;
	/// Return true if the VFS url is writable
	virtual bool vfs_isWritable() { return isWritable; }
	/// Return vfile* or 0 if not found
	inline vfile* vfs_search(const QString& name){ return (*vfs_filesP)[name]; } 
	/// Return an empty vfile* list if not found
	QList<vfile*> vfs_search(const KRQuery& filter);
	/// The total size of all the files in the VFS,
	KIO::filesize_t vfs_totalSize();
	/// The number of files in the VFS
	inline unsigned long vfs_noOfFiles() { return vfs_filesP->count(); }
	/// Returns the VFS url.
	inline KUrl vfs_getOrigin()          { return vfs_origin;          }
	/// Return the VFS type.
	inline VFS_TYPE vfs_getType()        { return vfs_type;            }
	/// Returns true if vfs is busy
	inline bool vfs_isBusy()             { return vfs_busy;            }
	/// Returns true if hidden files has to be shown
	       bool vfs_showHidden();
	/// Return the first file in the VFS and set the internal iterator to the beginning of the list.
	inline vfile* vfs_getFirstFile(){ vfileIterator = vfs_filesP->begin(); return (vfileIterator == vfs_filesP->end() ? 0 : *vfileIterator ); }
	/// Return the the next file in the list and advance the iterator.
	inline vfile* vfs_getNextFile() { if( vfileIterator == vfs_filesP->end() || ++vfileIterator == vfs_filesP->end() ) return 0; else return *vfileIterator;  }
	/// returns true if the vfs can be deleted without crash        
	virtual bool vfs_canDelete() { return deletePossible; } 
	/// process the application events               
	virtual bool vfs_processEvents();        
	/// process the application events               
	virtual void vfs_requestDelete();        
	/// process the application events               
	virtual bool vfs_isDeleting()    { return deleteRequested; }
   // KDE FTP proxy bug correction
   static QString pathOrUrl( const KUrl &originIn, KUrl::AdjustPathOption trailingSlash = KUrl::LeaveTrailingSlash );


public slots:
	/// Re-reads files and stats and fills the vfile list
	bool vfs_refresh(const KUrl& origin);
	/// Used to refresh the VFS when a job finishs. it calls the refresh() slot
	/// or display a error message if the job fails
	bool vfs_refresh(KJob * job);
	bool vfs_refresh();
	void vfs_setQuiet(bool beQuiet){ quietMode=beQuiet; }
	bool vfs_enableRefresh(bool enable);        
	void vfs_invalidate() { invalidated = true; }          

signals:
	void startUpdate(); //< emitted when the VFS starts to refresh its list of vfiles.
	void startJob(KIO::Job* job);
	void incrementalRefreshFinished( const KUrl& ); //< emitted when the incremental refresh was finished
	void addedVfile(vfile* vf);
	void deletedVfile(const QString& name);
	void updatedVfile(vfile* vf);
	void cleared();
	void deleteAllowed();                

protected:
	/// Feel the vfs dictionary with vfiles, must be implemented for each vfs
	virtual bool populateVfsList(const KUrl& origin, bool showHidden) = 0;
	/// Called by populateVfsList for each file
	void foundVfile( vfile *vf ) { vfs_tempFilesP->insert(vf->vfile_getName(),vf); }
	/// Set the vfile list pointer
	void setVfsFilesP(vfileDict* dict);
	/// clear and delete all current vfiles
	inline void clear();
	/// Add a new vfile to the list.
	inline void addToList(vfile *data){ vfs_filesP->insert(data->vfile_getName(),data); }
	/// Deletes a vfile from the list.
	inline void removeFromList(QString name){ vfs_filesP->remove(name); }

	/// Deletes a vfile from the list.
	void calculateURLSize(KUrl url,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop);
        
	VFS_TYPE      vfs_type;     //< the vfs type.
	KUrl          vfs_origin;   //< the path or file the VFS originates from.
	bool          vfs_busy;     //< true if vfs is busy with refreshing
	bool quietMode;             //< if true the vfs won't display error messages or emit signals
	bool disableRefresh;        //< true if refresh is disabled
	bool isWritable;            //< true if it's writable
	KUrl postponedRefreshURL;   //< true if vfs_refresh() was called when refresh is disabled.
	bool invalidated;           //< the content of the cache is invalidated
	bool panelConnected;        //< indicates that there's a panel connected. Important for disabling the dir watcher
	
protected slots:
	/// The slot for the KIO::DirectorySizeJob
	void slotKdsResult(KJob *job);
	void slotStatResultArrived(KJob *job);
        
private:
	vfileDict*  vfs_filesP;    //< Point to a lists of virtual files (vfile).
	vfileDict*  vfs_tempFilesP;//< Temporary files are stored here
	QHash<QString, vfile *>::iterator vfileIterator; //< Point to a dictionary of virtual files (vfile).
	
	// used in the calcSpace function
	bool* kds_busy;
	bool  stat_busy;
	bool  deletePossible;        
	bool  deleteRequested;
	KIO::UDSEntry entry;        
	KIO::filesize_t* kds_totalSize;
	unsigned long*   kds_totalFiles;
	unsigned long*   kds_totalDirs;
};

#endif
