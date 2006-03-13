/***************************************************************************
                         mediabutton.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
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

#include "mediabutton.h"
#include "../kicons.h"

#include <qpopupmenu.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kmountpoint.h>
#include <kmimetype.h>

MediaButton::MediaButton( QWidget *parent, const char *name ) : QToolButton( parent, name ),
		loadInProgress( false ), popupMenu( 0 )
	 {
	KIconLoader * iconLoader = new KIconLoader();
	QPixmap icon = iconLoader->loadIcon( "blockdevice", KIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the available media list" ), true );
	setPopupDelay( 10 ); // 0.01 seconds press
	setAcceptDrops( false );

	popupMenu = new QPopupMenu( this );
	Q_CHECK_PTR( popupMenu );

	setPopup( popupMenu );

	connect( popupMenu, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShow() ) );
	connect( popupMenu, SIGNAL( activated( int ) ), this, SLOT( slotPopupActivated( int ) ) );
}

MediaButton::~MediaButton() {}

void MediaButton::slotAboutToShow() {
	if( !loadInProgress ) {
		loadInProgress = true;
		popupMenu->setEnabled( false );
		
		mimes.clear();
		urls.clear();
		names.clear();
		
#if KDE_IS_VERSION(3,4,0)
		KURL mediaURL =  KURL::fromPathOrURL( "media:/" );
#else
		KURL mediaURL =  KURL::fromPathOrURL( "devices:/" );
#endif
		KIO::Job *listJob = KIO::listDir( mediaURL, false, false );
		
		connect( listJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
			this, SLOT( slotAddFiles( KIO::Job*, const KIO::UDSEntryList& ) ) );
		connect( listJob, SIGNAL( result( KIO::Job* ) ),
			this, SLOT( slotListResult( KIO::Job* ) ) );
	}
}

void MediaButton::slotAddFiles( KIO::Job * job, const KIO::UDSEntryList &list ) {
	KIO::UDSEntryListConstIterator it = list.begin();
	KIO::UDSEntryListConstIterator end = list.end();
	
	for (; it != end; ++it) {
		KURL url;
		QString name, mime;
		time_t mtime = (time_t)-1;
		KIO::UDSEntry::ConstIterator it2 = (*it).begin();
		for( ; it2 != (*it).end(); it2++ ) {
			switch ((*it2).m_uds) {
			case KIO::UDS_NAME:
				name = (*it2).m_str;
				break;
			case KIO::UDS_URL:
				url = KURL::fromPathOrURL((*it2).m_str);
				break;
			case KIO::UDS_MIME_TYPE:
				mime = (*it2).m_str;
				break;
			}
		}
		
		if( url.isEmpty() ) {
			url = ((KIO::SimpleJob *)job)->url();
			url.addPath( name );
		}
		
		names.append( name );
		urls.append( url );
		mimes.append( mime );
	}
}

void MediaButton::slotListResult( KIO::Job *job ) {
	loadInProgress = false;
	
	KURL url;
	if( job )
		url = ((KIO::SimpleJob *)job)->url();
	
	if( !job || job->error() ) {
		KMessageBox::error(this, i18n("Error at listing URL %1!").arg( url.prettyURL(0, KURL::StripFileProtocol ) ) );
		return;
	}

	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	for( unsigned i=0; i != urls.count(); i++ ) {
		if( urls[ i ].protocol() == "media" ) {
			QString path = urls[ i ].path(-1);
			while( path.startsWith("/" ) )
				path = path.mid( 1 );
			
			for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it) {
				if( KURL( (*it)->mountedFrom() ).fileName() == path )
					urls[ i ] = KURL::fromPathOrURL( (*it)->mountPoint() );
			}
		}
	}
	
	popupMenu->clear();
	for( int i=0; i != urls.count(); i++ ) {
		QPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimes[ i ] ) ->icon( QString::null, true ) );
		popupMenu->insertItem( pixmap, names[ i ], i, i );
	}
	
	popupMenu->setEnabled( true );
	popupMenu->show();
}

void MediaButton::slotPopupActivated( int elem ) {
	emit openUrl( urls[ elem ] );
}

#include "mediabutton.moc"
