/***************************************************************************
                       ftp_vfs.cpp
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

// Sys includes
#include <time.h>
// QT includes
#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qeventloop.h>
// KDE includes
#include <kmimetype.h>
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>
#include <kdebug.h>
// Krusader includes
#include "ftp_vfs.h"
#include "krpermhandler.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krprogress.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"

ftp_vfs::ftp_vfs(QObject* panel):vfs(panel),busy(false){
  // set the writable attribute
  isWritable = true;

  vfs_filesP = &vfs_files;
  vfs_files.setAutoDelete(true);

  vfs_type = FTP;
}

void ftp_vfs::slotAddFiles(KIO::Job *, const KIO::UDSEntryList& entries){
  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  // as long as u can find files - add them to the vfs
  for ( ; it != end; ++it ){
    KFileItem kfi(*it,vfs_origin,false,true);
    vfile *temp;

    // get file statistics
    QString name=kfi.text();
    // ignore un-needed entries
    if (name.isEmpty() || name=="." || name == ".." ) continue;

    KIO::filesize_t size = kfi.size();
    time_t mtime = kfi.time(KIO::UDS_MODIFICATION_TIME);
    bool symLink = kfi.isLink();
    mode_t mode = kfi.mode() | kfi.permissions();
    QString perm = KRpermHandler::mode2QString(mode);
    // set the mimetype
    QString mime = kfi.mimetype(); 
    QString symDest = "";
		if(symLink)symDest = kfi.linkDest();
		// create a new virtual file object
    if( kfi.user().isEmpty() )
      temp=new vfile(name,size,perm,mtime,symLink,getuid(),getgid(),mime,symDest,mode);
    else
      temp=new vfile(name,size,perm,mtime,symLink,kfi.user(),kfi.group(),mime,symDest,mode);
    addToList(temp);
	}
}

void ftp_vfs::slotRedirection(KIO::Job *, const KURL &url){
  // update the origin
  vfs_origin    = url;
}

void ftp_vfs::slotListResult(KIO::Job *job){
 if( job && job->error()){
    // we failed to refresh
    QString msg= KIO::buildErrorString(job->error(),vfs_origin.prettyURL());
    // display error message
    if ( !msg.isEmpty() && !quietMode ) KMessageBox::sorry(krApp,msg);
    error = true;
  }
  else {
    // if we got so far - so good
    error = false;
    // tell the panel to refresh
    if (!quietMode) {
      emit startUpdate();
    }
  }
  busy = false;
}

void ftp_vfs::startLister() {
  // Open the directory	marked by origin
  krConfig->setGroup("Look&Feel");
  KIO::Job *job = new KIO::ListJob(vfs_origin,false,false,QString::null,
                      krConfig->readBoolEntry("Show Hidden",_ShowHidden));
  connect(job,SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
         this,SLOT(slotAddFiles(KIO::Job*,const KIO::UDSEntryList&)));
  connect(job,SIGNAL(redirection(KIO::Job*,const KURL&)),
         this,SLOT(slotRedirection(KIO::Job*,const KURL&)));
  connect(job,SIGNAL(result(KIO::Job*)),
         this,SLOT(slotListResult(KIO::Job*)));

  if( !quietMode ) new KrProgress(job);
}

bool ftp_vfs::vfs_refresh(const KURL& origin) {
	error = false;

	QString errorMsg = QString::null;	
	if ( origin.isMalformed() )
    errorMsg = i18n("Malformed URL:\n%1").arg(origin.url());
  if( !KProtocolInfo::supportsListing(origin) )
		errorMsg = i18n("Protocol not supported by Krusader:\n%1").arg(origin.url());

	if( !errorMsg.isEmpty() ){
    if (!quietMode) KMessageBox::sorry(krApp, errorMsg);
		error = true;
    return false;
	}

	busy = true;
  
  // clear the the list and back up out current situation
  vfs_files.clear();
  vfs_origin = origin;

  //QTimer::singleShot( 0,this,SLOT(startLister()) );
  startLister();

  while( 0 && busy ){
    qApp->processEvents();
    qApp->eventLoop()->processEvents( QEventLoop::AllEvents|QEventLoop::WaitForMore); 
  }

  if( error ) return false;

	return true;
}


// copy a file to the vfs (physical)	
void ftp_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir){
  KURL destUrl = vfs_origin;

  if(dir != "") destUrl.addPath(dir);

  KIO::Job* job = new KIO::CopyJob(*fileUrls,destUrl,mode,false,true );
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
    connect(job,SIGNAL(result(KIO::Job*)),toNotify,SLOT(vfs_refresh(KIO::Job*)) );
}

// remove a file from the vfs (physical)
void ftp_vfs::vfs_delFiles(QStringList *fileNames){
  KURL::List filesUrls;
  KURL url;

  // names -> urls
  for(uint i=0 ; i<fileNames->count(); ++i){
    QString filename = (*fileNames)[i];
    url = vfs_origin;
    url.addPath(filename);
    filesUrls.append( url );
  }
  KIO::Job *job = new KIO::DeleteJob(filesUrls, false, true);
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}


KURL::List* ftp_vfs::vfs_getFiles(QStringList* names){
  KURL url;
  KURL::List* urls = new KURL::List();
  for(QStringList::Iterator name = names->begin(); name != names->end(); ++name){
    url = vfs_getFile(*name);
    urls->append(url);
  }
  return urls;
}


// return a path to the file
KURL ftp_vfs::vfs_getFile(const QString& name){	
  KURL url = vfs_origin;

  url.addPath(name);

  return url;
}

void ftp_vfs::vfs_mkdir(const QString& name){
  KURL url =  vfs_origin;
  url.addPath(name);

  KIO::SimpleJob* job = KIO::mkdir(url);
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}

void ftp_vfs::vfs_rename(const QString& fileName,const QString& newName){
  KURL::List fileUrls;
  KURL oldUrl = vfs_origin;
  oldUrl.addPath(fileName) ;

  fileUrls.append(oldUrl);

  KURL newUrl = vfs_origin;
  newUrl.addPath(newName);

  KIO::Job *job = new KIO::CopyJob(fileUrls,newUrl,KIO::CopyJob::Move,false,true );
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh(KIO::Job*)));
}

QString ftp_vfs::vfs_workingDir(){
	return vfs_origin.url(-1);
}

// to be implemented
void ftp_vfs::vfs_calcSpace(QString ,KIO::filesize_t*,unsigned long*,unsigned long*, bool*){

}

#include "ftp_vfs.moc"
