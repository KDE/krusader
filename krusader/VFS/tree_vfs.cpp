/***************************************************************************
                       tree_vfs.cpp
                   -------------------
    copyright            : (C) 2000 by Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------

 ***************************************************************************

  A 

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// QT includes
#include <qdir.h>
// KDE includes
#include <kmessagebox.h>
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kglobalsettings.h>
// Krusader includes
#include "tree_vfs.h"
#include "../Dialogs/krdialogs.h"
#include "../krusader.h"
#include "../defaults.h"
#include "krpermhandler.h"

tree_vfs::tree_vfs(QString,QWidget* panel):vfs(panel){
	vfs_type="tree";
}

bool tree_vfs::vfs_isWritable(){
	return KRpermHandler::fileWriteable( vfs_getOrigin() );
}

bool tree_vfs::vfs_refresh(QString){
  return true;
}

// copy a file to the vfs (physical)	
void tree_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QWidget* toNotify,QString dir){
  KURL dest;
	dest.setPath(vfs_origin+"/"+dir);

	KIO::Job* job = new KIO::CopyJob(*fileUrls,dest,mode,false,true );
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
    connect(job,SIGNAL(result(KIO::Job*)),toNotify,SLOT(refresh()) );
}

// remove a file from the vfs (physical)
void tree_vfs::vfs_delFiles(QStringList *){
	KURL::List filesUrls;
	KURL url;
	url.setPath(vfs_origin);
	filesUrls.append(url);
	
	KIO::Job *job;
	// delete of move to trash ?
	krConfig->setGroup("General");
	if( krConfig->readBoolEntry("Move To Trash",_MoveToTrash) ){
	  job = new KIO::CopyJob(filesUrls,KGlobalSettings::trashPath(),KIO::CopyJob::Move,false,true );
		connect(job,SIGNAL(result(KIO::Job*)),krApp,SLOT(changeTrashIcon()));
	}
	else
	  job = new KIO::DeleteJob(filesUrls, false, true);
	
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()));
}

// return a path to the file
QString tree_vfs::vfs_getFile(QString){	
	return vfs_origin;
}

KURL::List* tree_vfs::vfs_getFiles(QStringList*){
  KURL url;
  KURL::List* urls = new KURL::List();
  url.setPath(vfs_origin);
  urls->append(url);

  return urls;
}

void tree_vfs::vfs_mkdir(QString name){
	if (!QDir(vfs_origin).mkdir(name)){
	  if (!quietMode) KMessageBox::sorry(krApp,i18n("Can't create a directory check your permissions."));
	}	
	else vfs_refresh();
}


void tree_vfs::vfs_rename(QString,QString newName){
  KURL::List fileUrls;
  KURL url , destUrl;

	url.setPath(vfs_origin);
  fileUrls.append(url);
	
  QString dest = vfs_origin.left(vfs_origin.findRev('/'));
	destUrl.setPath(dest+"/"+newName);

  KIO::Job *job = new KIO::CopyJob(fileUrls,dest,KIO::CopyJob::Move,false,false );
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()));
}

void tree_vfs::vfs_calcSpace(QString name ,long long *totalSize,long *totalFiles, long *totalDirs){
  if (name == "/proc") return;

  QFileInfo qfi(name);
  if(qfi.isSymLink() || !qfi.isDir()){ // single files are easy : )
    ++(*totalFiles);
    (*totalSize) += qfi.size();
  }
  else{  // handle directories
    ++(*totalDirs);
    QDir dir(name);
    dir.setFilter(QDir::All|QDir::Hidden);
	  dir.setSorting(QDir::Name|QDir::DirsFirst);
	
	  // recurse on all the files in the directory
	  QFileInfoList* fileList = const_cast<QFileInfoList*>(dir.entryInfoList());
	  for (QFileInfo* qfiP = fileList->first(); qfiP != 0; qfiP = fileList->next()){
	    if (qfiP->fileName() != "." && qfiP->fileName() != "..")
	      vfs_calcSpace(name+"/"+qfiP->fileName(),totalSize,totalFiles,totalDirs);
	  }
  }
}

#include "tree_vfs.moc"
