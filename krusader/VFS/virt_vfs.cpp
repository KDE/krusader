/***************************************************************************
                          virt_vfs.cpp  -  description
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

#include "virt_vfs.h"
#include <kfileitem.h>
#include <kurl.h>

virt_vfs::virt_vfs(QObject* panel, bool quiet): vfs(panel,quiet){
	// set the writable attribute
	isWritable = false;

	setVfsFilesP(&vfs_files);
	vfs_files.setAutoDelete(true);
	vfs_files_copy.setAutoDelete(true);

	vfs_type = VIRT;
}

virt_vfs::~virt_vfs(){
}

bool virt_vfs::populateVfsList(const KURL& origin, bool showHidden){
	vfs_origin = origin;
	
	QDictIterator<vfile> it(vfs_files_copy); // See QDictIterator
	for( ; it.current(); ++it )
		addToList( it.current() );
	
	return true; 
}

void virt_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir){

	for(KURL::List::Iterator url = fileUrls->begin(); url != fileUrls->end(); ++url){
		KFileItem kfi(KFileItem::Unknown,KFileItem::Unknown,*url);
		
		vfile* temp = new vfile((*url).prettyURL(),kfi.size(),kfi.permissionsString(),kfi.time(KIO::UDS_MODIFICATION_TIME),
          true,kfi.user(),kfi.group(),QString::null,kfi.mimetype(),(*url).prettyURL(),kfi.mode());
		vfs_files_copy.insert((*url).prettyURL(),temp);
	}
	vfs_refresh();
}

void virt_vfs::vfs_delFiles(QStringList *fileNames){

}

KURL::List* virt_vfs::vfs_getFiles(QStringList* names){
	KURL url;
	KURL::List* urls = new KURL::List();
	for(QStringList::Iterator name = names->begin(); name != names->end(); ++name){
		url = vfs_getFile(*name);
		urls->append(url);
	}
	return urls;
}

KURL virt_vfs::vfs_getFile(const QString& name){
	vfile* vf=vfs_search(name);
	if( !vf ) return KURL(); // empty 
 
	KURL url = vf->vfile_getUrl();
	if( vf->vfile_isDir() ) url.adjustPath(+1);
	return url;
}

void virt_vfs::vfs_mkdir(const QString& name){

}

void virt_vfs::vfs_rename(const QString& fileName,const QString& newName){

}

/// to be implemented
void virt_vfs::vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool * stop){

}