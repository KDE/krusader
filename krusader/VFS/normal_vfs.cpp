/***************************************************************************
                       normal_vfs.cpp                             
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
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
// QT includes
#include <qdir.h>
#include <qtimer.h>
#include <QByteArray>
// KDE includes
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kio/jobclasses.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kde_file.h>
#include <kfileitem.h>
// Krusader includes
#include "normal_vfs.h"
#include "../Dialogs/krdialogs.h"
#include "../MountMan/kmountman.h"
#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"
#include "../krslots.h"

// header files for ACL
#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

normal_vfs::normal_vfs(QObject* panel):vfs(panel), watcher(0) {
  vfs_type=NORMAL;
}

bool normal_vfs::populateVfsList(const KUrl& origin, bool showHidden){
	QString path = origin.path(KUrl::RemoveTrailingSlash);
	
	// set the writable attribute to true, if that's not the case - the KIO job
	// will give the warnings and errors
	isWritable = true;
	
	if( watcher ) delete watcher; //stop watching the old dir
	watcher = 0;

	// set the origin...
	vfs_origin = origin;
	vfs_origin.adjustPath(KUrl::RemoveTrailingSlash);
	vfs_origin.setProtocol("file"); // do not remove !
	vfs_origin.cleanPath();
	
	// check that the new origin exists
	if ( !QDir(path).exists() )
	{
		if( !quietMode ) KMessageBox::error(krApp, i18n("Directory %1 does not exist!", path ), i18n("Error"));
		return false;
	}
    
	KConfigGroup group( krConfig, "Advanced");
	if (group.readEntry("AutoMount",_AutoMount)) krMtMan.autoMount(path);
	
	DIR* dir = opendir(path.toLocal8Bit());
	if(!dir) 
	{
		if( !quietMode ) KMessageBox::error(krApp, i18n("Can't open the %1 directory!", path ), i18n("Error"));
		return false;
	}

  // change directory to the new directory
	QString save = getcwd( 0, 0 );
	if (chdir(path.toLocal8Bit()) != 0) {
		if( !quietMode ) KMessageBox::error(krApp, i18n("Access denied to")+path, i18n("Error"));
		closedir(dir);
		return false;
	}

	struct dirent* dirEnt;
  QString name;

	while( (dirEnt=readdir(dir)) != NULL ){
    name = QString::fromLocal8Bit(dirEnt->d_name);

		// show hidden files ?
		if ( !showHidden && name.left(1) == "." ) continue ;
		// we dont need the ".",".." enteries
		if (name=="." || name == "..") continue;
	  
		vfile* temp = vfileFromName(name);
    foundVfile(temp);
  }
	// clean up
	closedir(dir);
	chdir( save.toLocal8Bit() );
	
	if( panelConnected )
	{
		watcher = new KDirWatch();
		// connect the watcher
		connect(watcher,SIGNAL(dirty(const QString&)),this,SLOT(vfs_slotDirty(const QString&)));
		connect(watcher,SIGNAL(created(const QString&)),this, SLOT(vfs_slotCreated(const QString&)));
		connect(watcher,SIGNAL(deleted(const QString&)),this, SLOT(vfs_slotDeleted(const QString&)));	
		watcher->addDir(vfs_getOrigin().path(KUrl::RemoveTrailingSlash), KDirWatch::WatchFiles); //start watching the new dir
		watcher->startScan(true);
	}

  return true;
}

// copy a file to the vfs (physical)	
void normal_vfs::vfs_addFiles(KUrl::List *fileUrls,KIO::CopyJob::CopyMode mode,QObject* toNotify,QString dir, PreserveMode pmode ){
  //if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
  if( watcher ) {
    delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
    watcher = 0;
  }

  KUrl dest;
  dest.setPath(vfs_workingDir()+"/"+dir);

  KIO::Job* job = PreservingCopyJob::createCopyJob( pmode, *fileUrls,dest,mode,false,true );
  connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob *)) );
  if(mode == KIO::CopyJob::Move) // notify the other panel
    connect(job,SIGNAL(result(KJob*)),toNotify,SLOT(vfs_refresh(KJob*)) );
  else
    job->ui()->setAutoErrorHandlingEnabled( true );
}

// remove a file from the vfs (physical)
void normal_vfs::vfs_delFiles(QStringList *fileNames){
	KUrl::List filesUrls;
	KUrl url;
	QDir local( vfs_workingDir() );
	vfile* vf;

//  if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
	if( watcher ) {
		delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
		watcher = 0;
	}

	// names -> urls
	for(int i=0 ; i<fileNames->count(); ++i){
		QString filename = (*fileNames)[i];
		vf = vfs_search(filename);
		url.setPath( vfs_workingDir()+"/"+filename);
		filesUrls.append(url);
	}
	KIO::Job *job;
	
	// delete of move to trash ?
	KConfigGroup group( krConfig, "General");
	if( group.readEntry("Move To Trash",_MoveToTrash) ){
	  job = KIO::trash(filesUrls);
	  connect(job,SIGNAL(result(KJob*)),SLOTS,SLOT(changeTrashIcon()));
	}
	else
	  job = KIO::del(filesUrls);
	
	connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob*)));
}

// return a path to the file
KUrl normal_vfs::vfs_getFile(const QString& name){	
  QString url;
	if ( vfs_workingDir() == "/" ) url = "/"+name;
	else url = vfs_workingDir()+"/"+name;

	return KUrl(url);
}

KUrl::List* normal_vfs::vfs_getFiles(QStringList* names){
  KUrl::List* urls = new KUrl::List();
  for(QStringList::Iterator name = names->begin(); name != names->end(); ++name){
    urls->append( vfs_getFile(*name) );
  }
  return urls;
}

void normal_vfs::vfs_mkdir(const QString& name){
	if (!QDir(vfs_workingDir()).mkdir(name))
	  if (!quietMode) KMessageBox::sorry(krApp,i18n("Can't create a directory. Check your permissions."));
  vfs::vfs_refresh();
}

void normal_vfs::vfs_rename(const QString& fileName,const QString& newName){
  KUrl::List fileUrls;
  KUrl url , dest;

  //if( watcher ) watcher->stopScan(); // we will refresh manually this time...	
  if( watcher ) {
    delete watcher;   // stopScan is buggy, leaves reference on the directory, that's why we delete the watcher
    watcher = 0;
  }

  url.setPath( vfs_workingDir()+"/"+fileName );
  fileUrls.append(url);
  dest.setPath(vfs_workingDir()+"/"+newName);

  KIO::Job *job = KIO::move(fileUrls,dest);
  connect(job,SIGNAL(result(KJob*)),this,SLOT(vfs_refresh(KJob*)));
}

vfile* normal_vfs::vfileFromName(const QString& name){
	QString path = vfs_workingDir()+"/"+name;
	QByteArray fileName = path.toLocal8Bit();
	
	KDE_struct_stat stat_p;
	KDE_lstat(fileName.data(),&stat_p);
	KIO::filesize_t size = stat_p.st_size;
	QString perm = KRpermHandler::mode2QString(stat_p.st_mode);
	bool symLink= S_ISLNK(stat_p.st_mode);
	if( S_ISDIR(stat_p.st_mode) ) perm[0] = 'd';
	
	KUrl mimeUrl = KUrl(path);
	QString mime=QString();

	char symDest[256];
	bzero(symDest,256); 
	if( S_ISLNK(stat_p.st_mode) ){  // who the link is pointing to ?
		int endOfName=0;
		endOfName=readlink(fileName.data(),symDest,256);
		if ( endOfName != -1 ){
			if ( QDir(QString::fromLocal8Bit( symDest ) ).exists() ) perm[0] = 'd';
			if ( !QDir(vfs_workingDir()).exists( QString::fromLocal8Bit ( symDest ) ) ) mime = "Broken Link !";
		}
		else krOut << "Failed to read link: "<< path<<endl;
	}
	
	int rwx = 0;
	if( ::access( fileName.data(), R_OK ) == 0 )
		rwx |= R_OK;
	if( ::access( fileName.data(), W_OK ) == 0 )
		rwx |= W_OK;
	if( ::access( fileName.data(), X_OK ) == 0 )
		rwx |= X_OK;
	
	// create a new virtual file object
	vfile* temp=new vfile(name,size,perm,stat_p.st_mtime,symLink,stat_p.st_uid,
                          stat_p.st_gid,mime,QString::fromLocal8Bit( symDest ),stat_p.st_mode, rwx);
	temp->vfile_setUrl( mimeUrl );
	return temp;
}

void normal_vfs::getACL( vfile *file, QString &acl, QString &defAcl )
{
	acl = defAcl = QString();
#if defined( HAVE_POSIX_ACL )
	QByteArray fileName = file->vfile_getUrl().path( KUrl::RemoveTrailingSlash ).toLocal8Bit();
#if HAVE_NON_POSIX_ACL_EXTENSIONS
	if ( acl_extended_file( fileName.data() ) )
	{
#endif
		acl = getACL( fileName.data(), ACL_TYPE_ACCESS );
		if( file->vfile_isDir() )
			defAcl = getACL( fileName.data(), ACL_TYPE_DEFAULT );
#if HAVE_NON_POSIX_ACL_EXTENSIONS
	}
#endif
#endif
}

QString normal_vfs::getACL( const QString & path, int type )
{
#if defined( HAVE_POSIX_ACL )
	acl_t acl = 0;
	// do we have an acl for the file, and/or a default acl for the dir, if it is one?
	if ( ( acl = acl_get_file( path.data(), type ) ) != 0 )
	{
		bool aclExtended = false;
		
#if HAVE_NON_POSIX_ACL_EXTENSIONS
		aclExtended = acl_equiv_mode( acl, 0 );
#else
		acl_entry_t entry;
		int ret = acl_get_entry( acl, ACL_FIRST_ENTRY, &entry );
		while ( ret == 1 ) {
			acl_tag_t currentTag;
			acl_get_tag_type( entry, &currentTag );
			if ( currentTag != ACL_USER_OBJ &&
				currentTag != ACL_GROUP_OBJ &&
				currentTag != ACL_OTHER )
			{
				aclExtended = true;
				break;
			}
			ret = acl_get_entry( acl, ACL_NEXT_ENTRY, &entry );
		}
#endif
		
		if ( !aclExtended )
		{
			acl_free( acl );
			acl = 0;
		}
	}
	
	if( acl == 0 )
		return QString();
	
	char *aclString = acl_to_text( acl, 0 );
	QString ret = QString::fromLatin1( aclString );
	acl_free( (void*)aclString );
	acl_free( acl );
	
	return ret;
#else
	return QString();
#endif
}

void normal_vfs::vfs_slotRefresh()
{
	KConfigGroup group( krConfig, "Advanced");
	int maxRefreshFrequency = group.readEntry("Max Refresh Frequency", 1000);
	vfs_refresh();
	disconnect( &refreshTimer, SIGNAL( timeout() ), this, SLOT( vfs_slotRefresh() ) );
	refreshTimer.start( maxRefreshFrequency, true );
}

bool normal_vfs::burstRefresh(const QString& path ){
	if( path == vfs_getOrigin().path(KUrl::RemoveTrailingSlash) ) {
		if( !refreshTimer.isActive() ) {
			// the directory itself is dirty - full refresh is needed
			QTimer::singleShot(0, this, SLOT( vfs_slotRefresh() ) ); // safety: dirty signal comes from KDirWatch!
			return true;
		}
		disconnect( &refreshTimer, SIGNAL( timeout() ), this, SLOT( vfs_slotRefresh() ) );
		connect( &refreshTimer, SIGNAL( timeout() ), this, SLOT( vfs_slotRefresh() ) );
		postponedRefreshURL = KUrl(path);
		return true;
	}
	return false;
}

void normal_vfs::vfs_slotDirty(const QString& path){ 
	if( disableRefresh ){
		postponedRefreshURL = KUrl(path);
		return;
	}
	
	if( burstRefresh( path ) )
		return;        
	
	KUrl url = KUrl(path);
	QString name = url.fileName();
	
	// do we have it already ?
	if( !vfs_search(name ) ) return vfs_slotCreated(path);
	
	// we have an updated file..
	removeFromList(name);
	vfile* vf = vfileFromName(name);
	addToList(vf);
	emit updatedVfile(vf);
}

void normal_vfs::vfs_slotCreated(const QString& path){  
	if( disableRefresh ){
		postponedRefreshURL = KUrl(path);
		return;
	}	
	
	if( burstRefresh( path ) )
		return;        
	
	
	KUrl url = KUrl(path);
	QString name = url.fileName();	
	// if it's in the CVS - it's an update not new file
	if( vfs_search(name) )
		return vfs_slotDirty(path);
	
	vfile* vf = vfileFromName(name);
	addToList(vf);
	emit addedVfile(vf);	
}

void normal_vfs::vfs_slotDeleted(const QString& path){ 
	if( disableRefresh ){
		postponedRefreshURL = KUrl(path);
		return;
	}
	
	if( burstRefresh( path ) )
		return;        
	
	
	KUrl url = KUrl(path);
	QString name = url.fileName();
	
	// if it's not in the CVS - do nothing
	if( vfs_search(name) ){
		emit deletedVfile(name);
		removeFromList(name);	
	}	
}

#include "normal_vfs.moc"
