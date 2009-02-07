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
#include <sys/param.h>
#include <unistd.h>
#ifdef BSD
#include <sys/types.h>
#endif 
// QT includes
#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qeventloop.h>
// KDE includes
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/deletejob.h>
#include <kuiserverjobtracker.h>
#include <kmessagebox.h>
#include <kprotocolmanager.h>
#include <kdebug.h> 
// Krusader includes
#include "ftp_vfs.h"
#include "krpermhandler.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"

ftp_vfs::ftp_vfs( QObject* panel ) : vfs( panel ), busy( false ) {
	// set the writable attribute
	isWritable = true;
	vfs_type = VFS_FTP;
}

ftp_vfs::~ftp_vfs() {
	busy = false;
}

void ftp_vfs::slotAddFiles( KIO::Job *, const KIO::UDSEntryList& entries ) {
	int rwx = -1;
	
	QString prot = vfs_origin.protocol();
	if( prot == "krarc" || prot == "tar" || prot == "zip" )
		rwx = PERM_ALL;
	
	KIO::UDSEntryList::const_iterator it = entries.begin();
	KIO::UDSEntryList::const_iterator end = entries.end();

	// as long as u can find files - add them to the vfs
	for ( ; it != end; ++it ) {
		KFileItem kfi( *it, vfs_origin, true, true );
		vfile *temp;

		// get file statistics
		QString name = kfi.text();
		// ignore un-needed entries
		if ( name.isEmpty() || name == "." || name == ".." ) continue;

		KIO::filesize_t size = kfi.size();
		time_t mtime = kfi.time( KFileItem::ModificationTime ).toTime_t();
		bool symLink = kfi.isLink();
		mode_t mode = kfi.mode() | kfi.permissions();
		QString perm = KRpermHandler::mode2QString( mode );
		// set the mimetype
		QString mime = kfi.mimetype();
		QString symDest = "";
		if ( symLink ) {
			symDest = kfi.linkDest();
			if ( kfi.isDir() ) perm[ 0 ] = 'd';
		}

		// create a new virtual file object
		if ( kfi.user().isEmpty() )
			temp = new vfile( name, size, perm, mtime, symLink, getuid(), getgid(), mime, symDest, mode, rwx );
		else {
			QString currentUser = vfs_origin.user();
			if ( currentUser.contains( "@" ) )  /* remove the FTP proxy tags from the username */
				currentUser.truncate( currentUser.indexOf( '@' ) );
			if ( currentUser.isEmpty() ) {
				if( vfs_origin.host().isEmpty() )
					currentUser = KRpermHandler::uid2user( getuid() );
				else {
					currentUser = ""; // empty, but not QString()
				}
			}
			temp = new vfile( name, size, perm, mtime, symLink,
			                  kfi.user(), kfi.group(), currentUser, 
			                  mime, symDest, mode, rwx, kfi.ACL().asString(),
			                  kfi.defaultACL().asString() );
		}

		if( !kfi.localPath().isEmpty() ){
			temp->vfile_setUrl( kfi.localPath() );
		} else {
			temp->vfile_setUrl( kfi.url() );
		}
		temp->vfile_setIcon( kfi.iconName() );
		foundVfile( temp );
	}
}

void ftp_vfs::slotPermanentRedirection( KIO::Job*, const KUrl&, const KUrl& newUrl ) {
	vfs_origin = newUrl;
	vfs_origin.adjustPath(KUrl::RemoveTrailingSlash);
}

void ftp_vfs::slotRedirection( KIO::Job *, const KUrl &url ) {
	// update the origin
	vfs_origin = url;
	vfs_origin.adjustPath(KUrl::RemoveTrailingSlash);
}

void ftp_vfs::slotListResult( KJob *job ) {
	if ( job && job->error() ) {
		// we failed to refresh
		listError = true;
		// display error message
		if ( !quietMode ) job->uiDelegate()->showErrorMessage();
	}
	busy = false;
}

bool ftp_vfs::populateVfsList( const KUrl& origin, bool showHidden ) {
	QString errorMsg = QString();
	if ( !origin.isValid() )
		errorMsg = i18n( "Malformed URL:\n%1", origin.url() );
	if ( !KProtocolManager::supportsListing( origin ) ) {
		if( origin.protocol() == "ftp" && KProtocolManager::supportsReading( origin ) ) 
			errorMsg = i18n( "Krusader doesn't support FTP access via HTTP.\nIf it is not the case, please check and change the Proxy settings in kcontrol." );
		else
			errorMsg = i18n( "Protocol not supported by Krusader:\n%1", origin.url() );
	}

	if ( !errorMsg.isEmpty() ) {
		if ( !quietMode ) KMessageBox::sorry( krApp, errorMsg );
		return false;
	}

	busy = true;

	vfs_origin = origin;
	vfs_origin.adjustPath(KUrl::RemoveTrailingSlash);

	//QTimer::singleShot( 0,this,SLOT(startLister()) );
	listError = false;
	// Open the directory	marked by origin
	//vfs_origin.adjustPath(KUrl::AddTrailingSlash);
	KIO::Job *job = KIO::listDir( vfs_origin, KIO::HideProgressInfo, showHidden );
	connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
	         this, SLOT( slotAddFiles( KIO::Job*, const KIO::UDSEntryList& ) ) );
	connect( job, SIGNAL( redirection( KIO::Job*, const KUrl& ) ),
	         this, SLOT( slotRedirection( KIO::Job*, const KUrl& ) ) );
	connect( job, SIGNAL( permanentRedirection( KIO::Job*, const KUrl&, const KUrl& ) ),
	         this, SLOT( slotPermanentRedirection( KIO::Job*, const KUrl&, const KUrl& ) ) );

	connect( job, SIGNAL( result( KJob* ) ),
	         this, SLOT( slotListResult( KJob* ) ) );

	job->ui()->setWindow( krApp );

	if ( !quietMode ) {
		emit startJob( job );
	}

	while ( busy && vfs_processEvents());

	if ( listError ) return false;

	return true;
}


// copy a file to the vfs (physical)
void ftp_vfs::vfs_addFiles( KUrl::List *fileUrls, KIO::CopyJob::CopyMode mode, QObject* toNotify, QString dir,  PreserveMode /*pmode*/ ) {
	KUrl destUrl = vfs_origin;

	if ( dir != "" ) {
		destUrl.addPath( dir );
		destUrl.cleanPath();  // removes the '..', '.' and extra slashes from the URL.

		if ( destUrl.protocol() == "tar" || destUrl.protocol() == "zip" || destUrl.protocol() == "krarc" ) {
			if ( QDir( destUrl.path( KUrl::RemoveTrailingSlash ) ).exists() )
				destUrl.setProtocol( "file" );  // if we get out from the archive change the protocol
		}
	}

	KIO::Job* job = 0;
	switch( mode ) {
	case KIO::CopyJob::Copy:
		job = KIO::copy( *fileUrls, destUrl );
		break;
	case KIO::CopyJob::Move:
		job = KIO::move( *fileUrls, destUrl );
		break;
	case KIO::CopyJob::Link:
		job = KIO::link( *fileUrls, destUrl );
		break;
	}

	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
	if ( mode == KIO::CopyJob::Move )  // notify the other panel
		connect( job, SIGNAL( result( KJob* ) ), toNotify, SLOT( vfs_refresh( KJob* ) ) );
}

// remove a file from the vfs (physical)
void ftp_vfs::vfs_delFiles( QStringList *fileNames, bool /* reallyDelete */ ) {
	KUrl::List filesUrls;
	KUrl url;

	// names -> urls
	for ( int i = 0 ; i < fileNames->count(); ++i ) {
		QString filename = ( *fileNames ) [ i ];
		url = vfs_origin;
		url.addPath( filename );
		filesUrls.append( url );
	}
	KIO::Job *job = KIO::del( filesUrls );
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
}


KUrl::List* ftp_vfs::vfs_getFiles( QStringList* names ) {
	KUrl url;
	KUrl::List* urls = new KUrl::List();
	for ( QStringList::Iterator name = names->begin(); name != names->end(); ++name ) {
		url = vfs_getFile( *name );
		urls->append( url );
	}
	return urls;
}


// return a path to the file
KUrl ftp_vfs::vfs_getFile( const QString& name ) {
	vfile * vf = vfs_search( name );
	if ( !vf ) return KUrl(); // empty

	KUrl url = vf->vfile_getUrl();
	if ( vf->vfile_isDir() ) url.adjustPath( KUrl::AddTrailingSlash );
	return url;
}

void ftp_vfs::vfs_mkdir( const QString& name ) {
	KUrl url = vfs_origin;
	url.addPath( name );

	KIO::SimpleJob* job = KIO::mkdir( url );
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
}

void ftp_vfs::vfs_rename( const QString& fileName, const QString& newName ) {
	KUrl oldUrl = vfs_origin;
	oldUrl.addPath( fileName ) ;

	KUrl newUrl = vfs_origin;
	newUrl.addPath( newName );

	KIO::Job *job = KIO::moveAs( oldUrl, newUrl, false );
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( vfs_refresh( KJob* ) ) );
}

QString ftp_vfs::vfs_workingDir() {
	return vfs_origin.url( KUrl::RemoveTrailingSlash );
}

#include "ftp_vfs.moc"
