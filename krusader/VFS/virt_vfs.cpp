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
#include <kio/directorysizejob.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kstandarddirs.h>

#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "virt_vfs.h"

#define VIRT_VFS_DB "virt_vfs.db"

Q3Dict<KUrl::List> virt_vfs::virtVfsDict;
KConfig* virt_vfs::virt_vfs_db=0;

virt_vfs::virt_vfs( QObject* panel, bool quiet ) : vfs( panel, quiet ) {
	// set the writable attribute
	isWritable = true;

	virtVfsDict.setAutoDelete( true );
	if ( virtVfsDict.isEmpty() ) {
		restore();
	}

	vfs_type = VIRT;
}

virt_vfs::~virt_vfs() {}

bool virt_vfs::populateVfsList( const KUrl& origin, bool /*showHidden*/ ) {
	vfs_origin = origin;
	vfs_origin.adjustPath(KUrl::RemoveTrailingSlash);
	path = origin.path( KUrl::RemoveTrailingSlash ).mid( 1 );
	if ( path.isEmpty() ) path = "/";

	KUrl::List* urlList = virtVfsDict[ path ];
	if ( !urlList ) {
		urlList = new KUrl::List();
		virtVfsDict.insert( path, urlList );
		virtVfsDict[ "/" ] ->append( KUrl( "virt:/" + path ) );
	}
	
	if ( urlList->isEmpty() ) return true;
	KUrl::List::iterator it;
	for ( it = urlList->begin() ; it != urlList->end() ; /*++it*/ ) {
		KUrl url = *it;
		// translate url->vfile and remove urls that no longer exist from the list
		vfile* vf = stat(url);
		if ( !vf ) {
			it = urlList->remove( it );
			// the iterator is advanced automaticly
			continue;
		}
		foundVfile( vf );
		++it;
	}
	save();
	return true;
}

void virt_vfs::vfs_addFiles( KUrl::List *fileUrls, KIO::CopyJob::CopyMode /*mode*/, QObject* /*toNotify*/, QString /*dir*/, PreserveMode /*pmode*/ ) {
	if ( path == "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "You can't copy files directly to the 'virt:/' directory.\nYou can create a sub directory and copy your files into it." ), i18n( "Error" ) );
		return ;
	}
	
	KUrl::List* urlList = virtVfsDict[ path ];
	for( int i=0; i != fileUrls->count(); i++ ) {
		if( !urlList->contains( (*fileUrls)[ i ] ) )
			urlList->push_back( (*fileUrls)[ i ] );
	}

	vfs_refresh();
}

void virt_vfs::vfs_delFiles( QStringList *fileNames ) {
	if ( path == "/" ) {
		for ( int i = 0 ; i < fileNames->count(); ++i ) {
			QString filename = ( *fileNames ) [ i ];
			virtVfsDict[ "/" ] ->remove( QString("virt:/")+filename );
			virtVfsDict.remove( filename );
		}
		vfs_refresh();
		return ;
	}

	KUrl::List filesUrls;
	KUrl url;

	// names -> urls
	for ( int i = 0 ; i < fileNames->count(); ++i ) {
		QString filename = ( *fileNames ) [ i ];
		filesUrls.append( vfs_getFile( filename ) );
	}
	KIO::Job *job;

	// delete of move to trash ?
	KConfigGroup group( krConfig, "General" );
	if ( group.readEntry( "Move To Trash", _MoveToTrash ) ) {
		job = KIO::trash( filesUrls, true );
		connect( job, SIGNAL( result( KJob* ) ), krApp, SLOT( changeTrashIcon() ) );
	} else
		job = KIO::del( filesUrls, false, true );

	// refresh will remove the deleted files...
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
}

void virt_vfs::vfs_removeFiles( QStringList *fileNames ) {
	if ( path == "/" )
		return; 
	
	// removing the URLs from the collection
	for ( int i = 0 ; i < fileNames->count(); ++i ) {
		KUrl::List* urlList = virtVfsDict[ path ];
		if( urlList )
			urlList->remove( vfs_getFile( ( *fileNames ) [ i ] ) );
	}
	
	vfs_refresh();
}

KUrl::List* virt_vfs::vfs_getFiles( QStringList* names ) {
	KUrl url;
	KUrl::List* urls = new KUrl::List();
	for ( QStringList::Iterator name = names->begin(); name != names->end(); ++name ) {
		url = vfs_getFile( *name );
		urls->append( url );
	}
	return urls;
}

KUrl virt_vfs::vfs_getFile( const QString& name ) {
	vfile * vf = vfs_search( name );
	if ( !vf ) return KUrl(); // empty

	KUrl url = vf->vfile_getUrl();
	if ( vf->vfile_isDir() ) url.adjustPath( KUrl::AddTrailingSlash );
	return url;
}

void virt_vfs::vfs_mkdir( const QString& name ) {
	if ( path != "/" ) {
		if ( !quietMode )
			KMessageBox::error( krApp, i18n( "Creating new directories is allowed only in the 'virt:/' directory." ), i18n( "Error" ) );
		return ;
	}
	KUrl::List* temp = new KUrl::List();
	virtVfsDict.insert( name, temp );
	virtVfsDict[ "/" ] ->append( QString( "virt:/" )+name );

	vfs_refresh();
}

void virt_vfs::vfs_rename( const QString& fileName, const QString& newName ) {
	KUrl::List fileUrls;
	KUrl url , dest;

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

	dest = KUrl( newName );
	// add the new url to the list
	// the the list is refreshed only existing files remain -
	// so we don't have to worry if the job was successful
	virtVfsDict[ path ] ->append( dest );

	KIO::Job *job = KIO::move( fileUrls, dest, true );
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
}

void virt_vfs::slotStatResult( KJob* job ) {
	if( !job || job->error() ) entry = KIO::UDSEntry();
	else entry = static_cast<KIO::StatJob*>(job)->statResult();
	busy = false;
}

vfile* virt_vfs::stat( const KUrl& url ) {
	if( url.protocol() == "virt" ){
		QString path = url.path().mid(1);
		if( path.isEmpty() ) path = "/";
		vfile * temp = new vfile( path, 0, "drwxr-xr-x", time( 0 ), false, getuid(), getgid(), "inode/directory", "", 0 );
		temp->vfile_setUrl( url );
		return temp;
	}
	KFileItem* kfi;
	if ( url.isLocalFile() ) {
		kfi = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url, true );
	}
	else {
		busy = true;
		KIO::StatJob* statJob = KIO::stat( url, false );
		connect( statJob, SIGNAL( result( KJob* ) ), this, SLOT( slotStatResult( KJob* ) ) );
		while ( busy && vfs_processEvents() );
		if( entry.count() == 0 ) return 0; // statJob failed
		
		kfi = new KFileItem(entry, url, true );
	}
	
	if ( kfi->time( KFileItem::ModificationTime ).isNull() ){
		 delete kfi;
		 return 0; // file not found		
	}
	
	vfile *temp;

	// get file statistics
	QString name;
	if( url.isLocalFile() )
		name = url.path();
	else
		name = url.prettyUrl();

	KIO::filesize_t size = kfi->size();
	time_t mtime = kfi->time( KFileItem::ModificationTime ).toTime_t();
	bool symLink = kfi->isLink();
	mode_t mode = kfi->mode() | kfi->permissions();
	QString perm = KRpermHandler::mode2QString( mode );
// set the mimetype
	QString mime = QString();
	QString symDest = "";
	if ( symLink ) {
		symDest = kfi->linkDest();
		if ( kfi->isDir() ) perm[ 0 ] = 'd';
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
		virt_vfs_db = new KConfig("data",VIRT_VFS_DB,KConfig::NoGlobals);
	}
	return virt_vfs_db; 
}

bool virt_vfs::save(){
	KConfig* db = getVirtDB();
	
	KConfigGroup group( krConfig, "virt_db");
	Q3DictIterator<KUrl::List> it( virtVfsDict ); // See QDictIterator
	for( ; it.current(); ++it ){
		KUrl::List::iterator url;
		QStringList entry;
		for ( url = it.current()->begin() ; url != it.current()->end() ; ++url ) {
			entry.append( (*url).prettyUrl() );
		}
		group.writeEntry(it.currentKey(),entry);
	}
	
	db->sync();
	
	return true;
}

bool virt_vfs::restore(){
	KConfig* db = getVirtDB();
	KConfigGroup dbGrp( db, "virt_db");
	
	QMap<QString, QString> map = db->entryMap("virt_db");
	QMap<QString, QString>::Iterator it;
	KUrl::List* urlList;
	for ( it = map.begin(); it != map.end(); ++it ) {
		urlList = new KUrl::List( dbGrp.readEntry(it.key(), QStringList() ) );
		virtVfsDict.insert( it.key(),urlList );
	}

	if( !virtVfsDict["/" ]){
		urlList = new KUrl::List();
		virtVfsDict.insert( "/", urlList );	
	}
		
	return true;
}

void virt_vfs::vfs_calcSpace( QString name , KIO::filesize_t* totalSize, unsigned long* totalFiles, unsigned long* totalDirs, bool* stop ) {
	if ( stop && *stop ) return ;
	if( path == "/" ) {
		KUrl::List* urlList = virtVfsDict[ name ];
		if ( urlList )
			for( int i=0; (i != urlList->size()) && !(*stop); i++ )
				calculateURLSize( (*urlList)[ i ], totalSize, totalFiles, totalDirs, stop );
		return;        
	}                
	return vfs::vfs_calcSpace( name, totalSize, totalFiles, totalDirs, stop );
}

#include "virt_vfs.moc"
