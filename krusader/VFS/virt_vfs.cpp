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

#include <unistd.h>
#include <time.h>
 
#include <kfileitem.h>
#include <kglobalsettings.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdirsize.h>
#include <kstandarddirs.h>

#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "virt_vfs.h"

#define VIRT_VFS_DB "virt_vfs.db"

QDict<KURL::List> virt_vfs::virtVfsDict;
KConfig* virt_vfs::virt_vfs_db=0;

virt_vfs::virt_vfs( QObject* panel, bool quiet ) : vfs( panel, quiet ) {
	// set the writable attribute
	isWritable = true;

	vfs_files.setAutoDelete( true );
	setVfsFilesP( &vfs_files );

	virtVfsDict.setAutoDelete( true );
	if ( virtVfsDict.isEmpty() ) {
		restore();
	}

	vfs_type = VIRT;
}

virt_vfs::~virt_vfs() {}

bool virt_vfs::populateVfsList( const KURL& origin, bool /*showHidden*/ ) {
	vfs_origin = origin;
	vfs_origin.adjustPath(-1);
	path = origin.path( -1 ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	KURL::List* urlList = virtVfsDict[ path ];
	if ( !urlList ) {
		urlList = new KURL::List();
		virtVfsDict.insert( path, urlList );
		virtVfsDict[ "/" ] ->append( KURL::fromPathOrURL( "virt:/" + path ) );
	}
	
	if ( urlList->isEmpty() ) return true;
	KURL::List::iterator it;
	for ( it = urlList->begin() ; it != urlList->end() ; /*++it*/ ) {
		KURL url = *it;
		// translate url->vfile and remove urls that no longer exist from the list
		vfile* vf = stat(url);
		if ( !vf ) {
			it = urlList->remove( it );
			// the iterator is advanced automaticly
			continue;
		}
		addToList( vf );
		++it;
	}
	save();
	return true;
}

void virt_vfs::vfs_addFiles( KURL::List *fileUrls, KIO::CopyJob::CopyMode /*mode*/, QObject* /*toNotify*/, QString /*dir*/ ) {
	if ( path == "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "You can't copy files directly to the 'virt:/' directory.\nYou can create a sub directory and copy your files into it." ), i18n( "Error" ) );
		return ;
	}
	
	KURL::List* urlList = virtVfsDict[ path ];
	(*urlList)+=(*fileUrls);

	vfs_refresh();
}

void virt_vfs::vfs_delFiles( QStringList *fileNames ) {
	if ( path == "/" ) {
		for ( uint i = 0 ; i < fileNames->count(); ++i ) {
			QString filename = ( *fileNames ) [ i ];
			virtVfsDict[ "/" ] ->remove( QString("virt:/")+filename );
			virtVfsDict.remove( filename );
		}
		vfs_refresh();
		return ;
	}

	KURL::List filesUrls;
	KURL url;

	// names -> urls
	for ( uint i = 0 ; i < fileNames->count(); ++i ) {
		QString filename = ( *fileNames ) [ i ];
		filesUrls.append( vfs_getFile( filename ) );
	}
	KIO::Job *job;

	// delete of move to trash ?
	krConfig->setGroup( "General" );
	if ( krConfig->readBoolEntry( "Move To Trash", _MoveToTrash ) ) {
#if KDE_IS_VERSION(3,4,0)
		job = KIO::trash( filesUrls, true );
#else
		job = new KIO::CopyJob( filesUrls, KGlobalSettings::trashPath(), KIO::CopyJob::Move, false, true );
#endif
		connect( job, SIGNAL( result( KIO::Job* ) ), krApp, SLOT( changeTrashIcon() ) );
	} else
		job = new KIO::DeleteJob( filesUrls, false, true );

	// refresh will remove the deleted files...
	connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( vfs_refresh( KIO::Job* ) ) );
}

KURL::List* virt_vfs::vfs_getFiles( QStringList* names ) {
	KURL url;
	KURL::List* urls = new KURL::List();
	for ( QStringList::Iterator name = names->begin(); name != names->end(); ++name ) {
		url = vfs_getFile( *name );
		urls->append( url );
	}
	return urls;
}

KURL virt_vfs::vfs_getFile( const QString& name ) {
	vfile * vf = vfs_search( name );
	if ( !vf ) return KURL(); // empty

	KURL url = vf->vfile_getUrl();
	if ( vf->vfile_isDir() ) url.adjustPath( + 1 );
	return url;
}

void virt_vfs::vfs_mkdir( const QString& name ) {
	if ( path != "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "Creating new directories is allowed only in the 'virt:/' directory." ), i18n( "Error" ) );
		return ;
	}
	KURL::List* temp = new KURL::List();
	virtVfsDict.insert( name, temp );
	virtVfsDict[ "/" ] ->append( QString( "virt:/" )+name );

	vfs_refresh();
}

void virt_vfs::vfs_rename( const QString& fileName, const QString& newName ) {
	KURL::List fileUrls;
	KURL url , dest;

	vfile* vf = vfs_search( fileName );
	if ( !vf ) return ; // not found

	if ( path == "/" ) {
		virtVfsDict[ "/" ] ->append( QString( "virt:/" ) + newName  );
		virtVfsDict[ "/" ] ->remove( QString( "virt:/" ) + fileName );
		virtVfsDict.insert( newName, virtVfsDict.take( fileName ) );
		vfs_refresh();
		return ;
	}

	url = vf->vfile_getUrl();
	fileUrls.append( url );

	dest = fromPathOrURL( newName );
	// add the new url to the list
	// the the list is refreshed only existing files remain -
	// so we don't have to worry if the job was successful
	virtVfsDict[ path ] ->append( dest );

	KIO::Job *job = new KIO::CopyJob( fileUrls, dest, KIO::CopyJob::Move, true, false );
	connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( vfs_refresh( KIO::Job* ) ) );
}

void virt_vfs::slotStatResult( KIO::Job* job ) {
	if( !job || job->error() ) entry = KIO::UDSEntry();
	else entry = static_cast<KIO::StatJob*>(job)->statResult();
	busy = false;
}

vfile* virt_vfs::stat( const KURL& url ) {
	if( url.protocol() == "virt" ){
		QString path = url.path().mid(1);
		if( path.isEmpty() ) path = "/";
		vfile * temp = new vfile( path, 0, "drwxr-xr-x", time( 0 ), false, getuid(), getgid(), "inode/directory", "", 0 );
		temp->vfile_setUrl( url );
		return temp;
	}
	KFileItem* kfi;
	if ( url.isLocalFile() ) {
		kfi = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url );
	}
	else {
		busy = true;
		KIO::StatJob* statJob = KIO::stat( url, false );
		connect( statJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
		while ( busy ) qApp->processEvents();
		if( entry.isEmpty()  ) return 0; // statJob failed
		
		kfi = new KFileItem(entry, url );
	}
	
	if ( !kfi->time( KIO::UDS_MODIFICATION_TIME ) ){
		 delete kfi;
		 return 0; // file not found		
	}
	
	vfile *temp;

	// get file statistics
	QString name;
	if( url.isLocalFile() )
		name = url.path();
	else
		name = url.prettyURL();

	KIO::filesize_t size = kfi->size();
	time_t mtime = kfi->time( KIO::UDS_MODIFICATION_TIME );
	bool symLink = kfi->isLink();
	mode_t mode = kfi->mode() | kfi->permissions();
	QString perm = KRpermHandler::mode2QString( mode );
// set the mimetype
	QString mime = kfi->mimetype();
	QString symDest = "";
	if ( symLink ) {
		symDest = kfi->linkDest();
		if ( kfi->isDir() || mime.contains( "directory" ) ) perm[ 0 ] = 'd';
	}

	// create a new virtual file object
	if ( kfi->user().isEmpty() )
		temp = new vfile( name, size, perm, mtime, symLink, getuid(), getgid(), mime, symDest, mode );
	else {
		QString currentUser = url.user();
		if ( currentUser.contains( "@" ) )  /* remove the FTP proxy tags from the username */
			currentUser.truncate( currentUser.find( '@' ) );
		if ( currentUser.isEmpty() )
			currentUser = KRpermHandler::uid2user( getuid() );
		temp = new vfile( name, size, perm, mtime, symLink, kfi->user(), kfi->group(), currentUser, mime, symDest, mode );
	}

	temp->vfile_setUrl( kfi->url() );
	delete kfi;
	return temp;
}

KConfig*  virt_vfs::getVirtDB(){
	if( !virt_vfs_db ){
		virt_vfs_db = new KConfig(VIRT_VFS_DB,false,"data");
	}
	return virt_vfs_db; 
}

bool virt_vfs::save(){
	KConfig* db = getVirtDB();
	
	db->setGroup("virt_db");
	QDictIterator<KURL::List> it( virtVfsDict ); // See QDictIterator
	for( ; it.current(); ++it ){
		KURL::List::iterator url;
		QStringList entry;
		for ( url = it.current()->begin() ; url != it.current()->end() ; ++url ) {
			entry.append( (*url).prettyURL() );
		}
		db->writeEntry(it.currentKey(),entry);
	}
	
	db->sync();
	
	return true;
}

bool virt_vfs::restore(){
	KConfig* db = getVirtDB();
	db->setGroup("virt_db");
	
	QMap<QString, QString> map = db->entryMap("virt_db");
	QMap<QString, QString>::Iterator it;
	KURL::List* urlList;
	for ( it = map.begin(); it != map.end(); ++it ) {
		urlList = new KURL::List( db->readListEntry(it.key()) );
		virtVfsDict.insert( it.key(),urlList );
	}

	if( !virtVfsDict["/" ]){
		urlList = new KURL::List();
		virtVfsDict.insert( "/", urlList );	
	}
		
	return true;
}


#include "virt_vfs.moc"
