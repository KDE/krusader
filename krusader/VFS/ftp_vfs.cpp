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


// QT includes
#include <qdir.h>
#include <qregexp.h>
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

template <class X> void kr_swap(X &a, X &b){
  X t = a;
  a = b;
  b = t;
}

ftp_vfs::ftp_vfs(QString origin,QWidget* panel):vfs(panel){
  supportCopyTo = true;
  supportMoveFrom = true;     // the big 4 is supported by ftp vfs...
	supportDelete = true;
	supportMoveTo = true;
	// set the writable attribute
	isWritable = true;
// 	  kdDebug() << origin << endl;
  vfs_filesP = &vfs_files;
  vfs_files.setAutoDelete(true);
  vfs_filesP2 = &vfs_files2;
  vfs_files2.setAutoDelete(true);
  notConnected = true;
		
	// breakdown the url;
	/* FIXME: untill KDE fixes the bug we have to check for
     passwords with @ in them... */
	bool bugfix = origin.find("@") != origin.findRev("@");
	if(bugfix) origin[origin.find("@")] = '^';
	KURL url = origin;
	port = url.port();
	loginName = url.user();
	password = url.pass();
	if(bugfix){
		password[password.find('^')] = '@';
		url.setPass(password);
	}
	

	vfs_type = "ftp";
  vfs_origin = url.prettyURL(-1);

  vfs_refresh();	
}

void ftp_vfs::slotAddFiles(KIO::Job * job, const KIO::UDSEntryList& entries){
	
	if(vfs_origin.right(1) == "/" ) vfs_origin = vfs_origin.left(vfs_origin.length()-1);

  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  // as long as u can find files - add them to the vfs
  for ( ; it != end; ++it ){
    KFileItem kfi(*it,vfs_url,true,true);
  	ftp_vfile *temp;
		
    // get file statistics
		QString name=kfi.text();
    // ignore un-needed entries
		if (name.isEmpty() || name=="." || name == ".." ) continue;
	
		unsigned long size = kfi.size();
  	QString perm = KRpermHandler::perm2QString(&kfi);
		QString	dateTime = KRpermHandler::time2QString(kfi.time(256 | 4 | 2));
		bool symLink = kfi.isLink();
		uid_t owner= 0;
		gid_t group= 0;
		mode_t mode = kfi.mode();
		QString mime; // set the mimetype
		if( kfi.isDir() ) mime = "inode/directory";
		else mime = KMimeType::findByURL( kfi.url(),mode,false,true)->name();
		QString symDest = "";
		if(symLink)symDest= kfi.linkDest();
		// create a new virtual file object
  	temp=new ftp_vfile(name,size,perm,dateTime,symLink,owner,group,mime,symDest,mode);
  	// set the ftp file permissions
  	temp->setWrite(KRpermHandler::ftpWriteable (kfi.user(),loginName,perm));
  	temp->setRead (KRpermHandler::ftpReadable  (kfi.user(),loginName,perm));
  	temp->setExec (KRpermHandler::ftpExecutable(kfi.user(),loginName,perm));
  	
  	vfs_addToList(temp);
	}
}

void ftp_vfs::slotRedirection(KIO::Job *, const KURL &url){
  // update the origin
  vfs_url    = url;
	vfs_origin = KURL::decode_string(url.prettyURL(-1));
  password   = url.pass();
  loginName  = url.user();
	port       = url.port();
}

void ftp_vfs::slotListResult(KIO::Job *job){
	if( job && job->error()){
    QString msg= KIO::buildErrorString(job->error(),vfs_origin);
    if ( !msg.isEmpty() && !quietMode )
      KMessageBox::sorry(krApp,msg);
    if( notConnected ){  // no option to revert
      error = true;
      if( !quietMode ) emit startUpdate();
    }
    else {
      kr_swap(vfs_filesP2,vfs_filesP);
      vfs_origin=origin_backup;
      vfs_url = vfs_url_backup;
    }
    return;
  }
  // if we got so far - so good
  notConnected = false;

  // tell the panel to get ready
  if (!quietMode) {
    emit startUpdate();
    endUpdate();
  }
}

bool ftp_vfs::vfs_refresh(QString origin) {
	error = false;
	KURL url = origin;

	kdDebug() << "Origin = " << origin << endl;

  if( !loginName.isEmpty()) url.setUser(loginName);
	if( !password.isEmpty() ) url.setPass(password);
	//if( port ) url.setPort(port);

	QString errorMsg = QString::null;	
	if ( url.isMalformed() )
    errorMsg = i18n("Malformed URL:\n%1").arg(url.url());
  if( !KProtocolInfo::supportsListing(url) )
		errorMsg = i18n("Protocol not supported by Krusader:\n").arg(url.url());

	if( !errorMsg.isEmpty() ){
    if (!quietMode) KMessageBox::sorry(krApp, errorMsg);
		error = true;
    return false;
	}

  // clear the the list and back up out current situation
	vfs_filesP2->clear();
  kr_swap(vfs_filesP2,vfs_filesP);
  origin_backup = vfs_origin;
  vfs_origin = origin;
  vfs_url_backup = vfs_url;
  vfs_url = url;
		
	// Open the directory	marked by origin
	krConfig->setGroup("Look&Feel");
  KIO::Job *job = new KIO::ListJob(url,false,false,QString::null,
                      krConfig->readBoolEntry("Show Hidden",_ShowHidden));
  connect(job,SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
      this,SLOT(slotAddFiles(KIO::Job*,const KIO::UDSEntryList&)));
  connect(job,SIGNAL(redirection(KIO::Job*,const KURL&)),
       this,SLOT(slotRedirection(KIO::Job*,const KURL&)));
  connect(job,SIGNAL(result(KIO::Job*)),
   this,SLOT(slotListResult(KIO::Job*)));

  if( !quietMode ) new KrProgress(job);

	return true;
}


// copy a file to the vfs (physical)	
void ftp_vfs::vfs_addFiles(KURL::List *fileUrls,KIO::CopyJob::CopyMode mode,QWidget* toNotify,QString dir){
  KURL destUrl = vfs_url;
  if(dir != "") destUrl.addPath(dir);


	destUrl.setUser(loginName);
  destUrl.setPass(password);
  if ( port ) destUrl.setPort(port);
	
  KIO::Job* job = new KIO::CopyJob(*fileUrls,destUrl,mode,false,true );
  connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
    connect(job,SIGNAL(result(KIO::Job*)),toNotify,SLOT(refresh()) );
}

// remove a file from the vfs (physical)
void ftp_vfs::vfs_delFiles(QStringList *fileNames){
	KURL::List filesUrls;
	KURL url;

 	// names -> urls
	for(uint i=0 ; i<fileNames->count(); ++i){
		QString filename = (*fileNames)[i];
		url = vfs_url;
    url.addPath(filename);
		url.setUser(loginName);
    url.setPass(password);
		if ( port ) url.setPort(port);
    filesUrls.append( url );
	}
	KIO::Job *job = new KIO::DeleteJob(filesUrls, false, true);
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()));
}


KURL::List* ftp_vfs::vfs_getFiles(QStringList* names){
  KURL url;
  KURL::List* urls = new KURL::List();
  for(QStringList::Iterator name = names->begin(); name != names->end(); ++name){
    url = vfs_getFile(*name);
    url.setUser(loginName);
    url.setPass(password);
		if ( port ) url.setPort(port);
    urls->append(url);
  }
  return urls;
}


// return a path to the file
QString ftp_vfs::vfs_getFile(QString name){	
	KURL url = vfs_url;
	
	if (vfs_search(name)->vfile_isDir())
		url.addPath(name+"/");
	else
	  url.addPath(name);

	url.setUser(loginName);
  url.setPass(password);
  if ( port ) url.setPort(port);

  return url.url();
}

void ftp_vfs::vfs_mkdir(QString name){
  KURL url =  vfs_url;
	url.addPath(name);
  url.setUser(loginName);
  url.setPass(password);
  if ( port ) url.setPort(port);

	KIO::SimpleJob* job = KIO::mkdir(url);
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()));
}


void ftp_vfs::vfs_rename(QString fileName,QString newName){
  KURL::List fileUrls;
  KURL url = vfs_url;
  url.addPath(fileName) ;
  url.setUser(loginName);
  url.setPass(password);

  fileUrls.append(url);

	KURL dest = vfs_url;
	dest.setPath(newName);
	dest.setUser(loginName);
  dest.setPass(password);
	if ( port ) dest.setPort(port);

  KIO::Job *job = new KIO::CopyJob(fileUrls,dest,KIO::CopyJob::Move,false,true );
	connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(vfs_refresh()));
}

QString ftp_vfs::vfs_workingDir(){
	QString ret = vfs_origin;
	if ( !password.isEmpty() ) ret.replace(QRegExp("@"),":"+password+"@");
  return ret;
}
	
#include "ftp_vfs.moc"
