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
#include <kglobalsettings.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdirsize.h>

#include "../krusader.h"
#include "../defaults.h"

QDict<vfs::vfileDict> virt_vfs::virtVfsDict;

virt_vfs::virt_vfs(QObject* panel, bool quiet): vfs(panel,quiet){
	// set the writable attribute
	isWritable = true;

	setVfsFilesP(&vfs_files);

	virtVfsDict.setAutoDelete(true);
	if( virtVfsDict.isEmpty() ){
		vfs_files_copy = new vfileDict;
		vfs_files_copy->setAutoDelete(true);
		virtVfsDict.insert("/",vfs_files_copy);
	}
	
	vfs_type = VIRT;
}

virt_vfs::~virt_vfs(){
}

bool virt_vfs::populateVfsList(const KURL& origin, bool /*showHidden*/){
	vfs_origin = origin;
	path = origin.path(-1).mid(1);
	if( path.isEmpty() ) path = "/";
	
	vfs_files_copy = virtVfsDict[path];
	if( !vfs_files_copy ){
		vfs_files_copy = new vfileDict;
		vfs_files_copy->setAutoDelete(true);
		virtVfsDict.insert(path,vfs_files_copy);
		vfile* temp = new vfile(path,0,"drwxr-xr-x",time(0),false,getuid(),getgid(),"inode/directory","",0);
		temp->vfile_setUrl(QString("virt:/")+path);
		virtVfsDict["/"]->insert(path,temp);	
	} 

	if( vfs_files_copy->isEmpty() ) return true;
	QDictIterator<vfile> it(*vfs_files_copy); // See QDictIterator
	while( it.current() ){
		KURL url = it.current()->vfile_getUrl();
		// remove files that no longer exist from the list
		if( path != "/" ){
			KFileItem kfi (KFileItem::Unknown,KFileItem::Unknown,url);
			if( !kfi.time(KIO::UDS_MODIFICATION_TIME) ){
				vfs_files_copy->remove(url.prettyURL());
				// the iterator is advanced automaticly
				continue;
			}
		}
		addToList( it.current() );
		++it;
	}
	
	return true; 
}

void virt_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode /*mode*/,QObject* /*toNotify*/,QString /*dir*/){
	if( path == "/" ){
		if( !quietMode ) 
			KMessageBox::error(krApp, i18n("You can't copy files directly to the 'virt:/' diretory.\nYou can create a sub directory and copy your files into it"), i18n("Error"));	
		return;
	}
	
	for(KURL::List::Iterator url = fileUrls->begin(); url != fileUrls->end(); ++url){
		KFileItem kfi(KFileItem::Unknown,KFileItem::Unknown,*url);
		
		vfile* temp = new vfile((*url).prettyURL(),kfi.size(),kfi.permissionsString(),kfi.time(KIO::UDS_MODIFICATION_TIME),
          false,kfi.user(),kfi.group(),QString::null,kfi.mimetype(),QString::null/*(*url).prettyURL()*/,kfi.mode());
		temp->vfile_setUrl(*url);
		vfs_files_copy->insert((*url).prettyURL(),temp);
	}
	vfs_refresh();
}

void virt_vfs::vfs_delFiles(QStringList *fileNames){
	if( path == "/" ){
		for(uint i=0 ; i<fileNames->count(); ++i){
			QString filename = (*fileNames)[i];
			vfs_files_copy->remove(filename);
			virtVfsDict.remove(filename);
		}
		vfs_refresh();
		return;
	}

	KURL::List filesUrls;
	KURL url;
		
	// names -> urls
	for(uint i=0 ; i<fileNames->count(); ++i){
		QString filename = (*fileNames)[i];
		filesUrls.append(vfs_getFile(filename));
	}
	KIO::Job *job;
	
	// delete of move to trash ?
	krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ){
	  job = new KIO::CopyJob(filesUrls,KGlobalSettings::trashPath(),KIO::CopyJob::Move,false,true );
	  connect(job,SIGNAL(result(KIO::Job*)),krApp,SLOT(changeTrashIcon()));
	}
	else
	  job = new KIO::DeleteJob(filesUrls, false, true);
	
	// refresh will remove the deleted files...
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
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
	if( path !=	"/" ){
		if( !quietMode ) 
			KMessageBox::error(krApp, i18n("Creating new directories is allowed only in the 'virt:/' directory."), i18n("Error"));
		return;
	}
	vfileDict* temp = new vfileDict;
	temp->setAutoDelete(true);
	virtVfsDict.insert(name,temp);
	vfile* vf = new vfile(name,0,"drwxr-xr-x",time(0),false,getuid(),getgid(),"inode/directory","",0);
	vf->vfile_setUrl(QString("virt:/")+name);
	virtVfsDict["/"]->insert(name,vf);	
	vfs_refresh();
}

void virt_vfs::vfs_rename(const QString& fileName,const QString& newName){
	KURL::List fileUrls;
	KURL url , dest;
	
	vfile* vf=vfs_search(fileName);
	if( !vf ) return; // not found
	
	if( path == "/" ){
		vfile* temp = new vfile(newName,0,"drwxr-xr-x",time(0),false,getuid(),getgid(),"inode/directory","",0);
		temp->vfile_setUrl(QString("virt:/")+newName);
		vfs_files_copy->insert(newName,temp);
		vfs_files_copy->remove(fileName);
		virtVfsDict.insert(newName,virtVfsDict.take(fileName));
		vfs_refresh();
		return;
	}

	url = vf->vfile_getUrl();
	fileUrls.append(url);
	
	dest = fromPathOrURL(newName);
	// create an (almost) identical vfile and add it to the list.
	// the the list is refreshed only existing files remain -
	// so we don't have to worry if the job was successful
	KFileItem kfi(KFileItem::Unknown,KFileItem::Unknown,url);
	vfile* temp = new vfile((dest).prettyURL(),kfi.size(),kfi.permissionsString(),kfi.time(KIO::UDS_MODIFICATION_TIME),
          false,kfi.user(),kfi.group(),QString::null,kfi.mimetype(),QString::null/*dest.prettyURL()*/,kfi.mode());
	temp->vfile_setUrl(dest);
	vfs_files_copy->insert(dest.prettyURL(),temp);
	
	KIO::Job *job = new KIO::CopyJob(fileUrls,dest,KIO::CopyJob::Move,true,false );
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}

void virt_vfs::slotListResult(KIO::Job* /*job*/){
	busy = false;
}

/// to be implemented
void virt_vfs::vfs_calcSpace(QString name ,KIO::filesize_t *totalSize,unsigned long *totalFiles,unsigned long *totalDirs, bool *stop){
	if (stop && *stop) return;
	busy = true;
	KDirSize* kds = KDirSize::dirSizeJob(vfs_getFile(name));
	krOut << "vfs_calcSpace: " << vfs_getFile(name) << endl;
  connect(kds,SIGNAL(result(KIO::Job*)),this,SLOT(slotListResult(KIO::Job*)));

	//while (busy && (!stop || !(*stop))) qApp->processEvents();
	
	*totalSize+=kds->totalSize();
	*totalFiles+=kds->totalFiles();
	*totalDirs+=kds->totalSubdirs();
	
	kds->kill(true);
}

#include "virt_vfs.moc.cpp"
