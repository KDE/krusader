
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
#include "../krusader.h"
#include "../krservices.h"
#include "../kicons.h"
#include "../krslots.h"
#include "../MountMan/kmountman.h"

#include <qfile.h>
#include <qfontmetrics.h>
#include <qtextstream.h>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>

#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kfileitem.h>
#include <kprocess.h>
#include <kdiskfreespace.h>
#include <qcursor.h>

#ifdef Q_OS_LINUX
// For CD/DVD drive detection
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#define CDROM_GET_CAPABILITY    0x5331
#define CDSL_CURRENT            ((int) (~0U>>1))
#define CDC_DVD_R               0x10000 /* drive can write DVD-R */
#define CDC_DVD_RAM             0x20000 /* drive can write DVD-RAM */
#define CDC_CD_R                0x2000  /* drive is a CD-R */
#define CDC_CD_RW               0x4000  /* drive is a CD-RW */
#define CDC_DVD                 0x8000  /* drive is a DVD */
#include <qfile.h>
#endif



MediaButton::MediaButton( QWidget *parent ) : QToolButton( parent ),
		popupMenu( 0 ), rightMenu( 0 ), hasMedia( false ), waitingForMount( -1 ), mountCheckerTimer()
	 {
	KIconLoader * iconLoader = new KIconLoader();
	QPixmap icon = iconLoader->loadIcon( "blockdevice", KIconLoader::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setIcon( QIcon( icon ) );
	setText( i18n( "Open the available media list" ) );
	setToolTip( i18n( "Open the available media list" ) );
	setPopupMode( QToolButton::InstantPopup );
	setAcceptDrops( false );

	popupMenu = new QMenu( this );
	popupMenu->installEventFilter( this );
	Q_CHECK_PTR( popupMenu );

	setMenu( popupMenu );

	connect( popupMenu, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShow() ) );
	connect( popupMenu, SIGNAL( aboutToHide() ), this, SLOT( slotAboutToHide() ) );
	connect( popupMenu, SIGNAL( triggered( QAction * ) ), this, SLOT( slotPopupActivated( QAction * ) ) );

	connect( &mountCheckerTimer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
}

MediaButton::~MediaButton() {
	busy = false;
}

void MediaButton::slotAboutToShow() {
	hasMedia = KProtocolInfo::isKnownProtocol( QString( "media" ) );
	KConfigGroup group( krConfig, "Advanced" );
	if( group.readEntry( "DontUseMediaProt", !hasMedia ) )
		hasMedia = false;
	
	popupMenu->clear();
	urls.clear();
	mediaUrls.clear();
	mimes.clear();
	quasiMounted.clear();
	waitingForMount = -1;
	
	if( hasMedia )
		createListWithMedia();
	else
		createListWithoutMedia();
	
	mountCheckerTimer.start( 1000, true );
}

void MediaButton::slotAboutToHide() {
	if( rightMenu )
		rightMenu->close();
	
	if( waitingForMount < 0 )
		mountCheckerTimer.stop();
}

void MediaButton::createListWithMedia() {
	KIO::ListJob *job = KIO::listDir( KUrl( "media:/" ), KIO::HideProgressInfo );
	connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
		this, SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
	connect( job, SIGNAL( result( KJob* ) ),
		this, SLOT( slotListResult( KJob* ) ) );
	busy = true;
	
	if( !busy )
		qApp->processEvents();
}

void MediaButton::slotEntries( KIO::Job *, const KIO::UDSEntryList& entries ) 
{
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	KIO::UDSEntryList::const_iterator it = entries.begin();
	KIO::UDSEntryList::const_iterator end = entries.end();
	
	while( it != end )
	{
		KUrl url;
		QString text;
		QString mime;
		QString localPath;
		bool mounted = false;
		
		if( (*it).contains( KIO::UDSEntry::UDS_NAME ) )
			text = QUrl::fromPercentEncoding( (*it).stringValue( KIO::UDSEntry::UDS_NAME ).toLatin1() );
		if( (*it).contains( KIO::UDSEntry::UDS_URL ) )
			url = KUrl( (*it).stringValue( KIO::UDSEntry::UDS_URL ) );
		if( (*it).contains( KIO::UDSEntry::UDS_MIME_TYPE ) )
		{
			mime = (*it).stringValue( KIO::UDSEntry::UDS_MIME_TYPE );
			if( !mime.endsWith( "unmounted" ) )
				mounted = true;
		}
		if( (*it).contains( KIO::UDSEntry::UDS_LOCAL_PATH ) )
			localPath = (*it).stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );

		if( text != "." && text != ".." ) {
			int index = popupMenu->count();
			KMimeType::Ptr mt = KMimeType::mimeType( mime );
			QPixmap pixmap;
			if( mt )
				pixmap = FL_LOADICON( mt->iconName() );
			
			mediaUrls.append( url );
			
			if( mounted && !localPath.isEmpty() )
			{
				url = KUrl( localPath );
				if( !text.contains( url.path() ) )
					text += "  [" + url.path() + "]";
			}
			else if( mounted )
			{
				url = getLocalPath( url, &mountList );
				if( url.isLocalFile() && !text.contains( url.path() ) )
					text += "  [" + url.path() + "]";
			}
			
			QAction *act = popupMenu->addAction( pixmap, text );
			act->setData( QVariant( index ) );
			idActionMap[ index ] = act;
			
			urls.append( url );
			mimes.append( mime );
			quasiMounted.append( false );
		}
		++it;
	}
}

void MediaButton::slotListResult( KJob * ) {
	busy = false;
}

KUrl MediaButton::getLocalPath( const KUrl &url, KMountPoint::List *mountList ) {
	KMountPoint::List mountListRef;
	if( mountList == 0 ) {
		mountListRef = KMountPoint::currentMountPoints();
		mountList = &mountListRef;
	}
	
	for (KMountPoint::List::iterator it = mountList->begin(); it != mountList->end(); ++it) {
		QString name = (*it)->mountedFrom();
		name = name.mid( name.lastIndexOf( "/" ) + 1 );
		if( name == url.fileName() ) {
			QString point = (*it)->mountPoint();
			if( !point.isEmpty() )
				return KUrl( point );
		}
	}
	return url;
}


void MediaButton::createListWithoutMedia() {
	/* WORKAROUND CODE START */
	
	/* 1. the menu is drawn when we know all the mount points
	   2. the menu is corrected, when the HDD volume sizes arrive from df
	
	   when the volume sizes are added to the items, some cases widget resize
	   is necessary. If transparency is set for the widget, QT produces weird
	   looking widgets, and that's why this workaround is used.
	   Here we add additional spaces to the mounted HDD elements for avoiding
	   the buggy widget resize. These are extra spaces. */

	extraSpaces = "";
	QFontMetrics fm( popupMenu->font() );
	int requiredWidth = fm.width( "999.9 GB " );
	while( fm.width( extraSpaces ) < requiredWidth )
		extraSpaces+=" ";
	/* WORKAROUND CODE END */
	
	KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
	for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
		addMountPoint( &(*(*it)), false );
	}
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it) {
		addMountPoint( &(*(*it)), true );
	}
}

QString MediaButton::detectType( KMountPoint *mp )
{
	QString typeName = QString();
#ifdef Q_OS_LINUX
	// Guessing device types by mount point is not exactly accurate...
	// Do something accurate first, and fall back if necessary.
	
	bool isCd=false;
	QString devname=mp->mountedFrom().section('/', -1);
	if(devname.startsWith("scd") || devname.startsWith("sr"))
	{
		// SCSI CD/DVD drive
		isCd=true;
	}
	else if(devname.startsWith("hd"))
	{
		// IDE device -- we can't tell if this is a
		// CD/DVD drive or harddisk by just looking at the
		// filename
		QFile m(QString("/proc/ide/") + devname + "/media");
		if(m.open(QIODevice::ReadOnly))
		{
			QTextStream in(&m);
			QString buf=in.readLine();
			if(buf.contains("cdrom"))
				isCd=true;
			m.close();
		}
	}
	if(isCd)
	{
		int device=::open((const char *)QFile::encodeName(mp->mountedFrom()), O_RDONLY | O_NONBLOCK );
		if(device>=0)
		{
			int drv=::ioctl(device, CDROM_GET_CAPABILITY, CDSL_CURRENT);
			if(drv>=0)
			{
				if((drv & CDC_DVD_R) || (drv & CDC_DVD_RAM))
					typeName = "dvdwriter";
				else if((drv & CDC_CD_R) || (drv & CDC_CD_RW))
					typeName = "cdwriter";
				else if(drv & CDC_DVD)
					typeName = "dvd";
				else
					typeName = "cdrom";
			}
			
			::close(device);
		}
	}
	if( !typeName.isNull() )
		return typeName;

#elif defined(__FreeBSD__)
	if (-1!=mp->mountedFrom().find("/acd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/ad",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().find("/da",0,FALSE)) typeName="hdd";
	else if (-1!=mp->mountedFrom().find("/afd",0,FALSE)) typeName="zip";
	else
#endif

    /* Guessing of cdrom and cd recorder devices */
	if (-1!=mp->mountPoint().find("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("cdrom",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountPoint().find("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdwriter",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().find("cdrw",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountPoint().find("cdrecorder",0,FALSE)) typeName="cdwriter";
	else if (-1!=mp->mountedFrom().find("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvdrecorder",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountedFrom().find("dvdwriter",0,FALSE)) typeName="dvdwriter";
	else if (-1!=mp->mountPoint().find("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().find("dvd",0,FALSE)) typeName="dvd";   
	else if (-1!=mp->mountedFrom().find("/dev/scd",0,FALSE)) typeName="cdrom";
	else if (-1!=mp->mountedFrom().find("/dev/sr",0,FALSE)) typeName="cdrom";
	
	/* Guessing of floppy types */
	else if (-1!=mp->mountedFrom().find("fd",0,FALSE)) {
		if (-1!=mp->mountedFrom().find("360",0,FALSE)) typeName="floppy5";
			if (-1!=mp->mountedFrom().find("1200",0,FALSE)) typeName="floppy5";
			else typeName="floppy";
		}
	else if (-1!=mp->mountPoint().find("floppy",0,FALSE)) typeName="floppy";
	
	else if (-1!=mp->mountPoint().find("zip",0,FALSE)) typeName="zip";
	else if (-1!=mp->mountType().find("nfs",0,FALSE)) typeName="nfs";
	else if (-1!=mp->mountType().find("smb",0,FALSE)) typeName="smb";
	else if (-1!=mp->mountedFrom().find("//",0,FALSE)) typeName="smb";
	else typeName="hdd";
	
	return typeName;
}

void MediaButton::slotPopupActivated( QAction * action ) {
	if( action && action->data().canConvert<int>() )
	{
		int elem = action->data().toInt();
		if( !quasiMounted[ elem ] && mimes[ elem ].endsWith( "_unmounted" ) ) {
			mount( elem );
			waitingForMount = elem;
			maxMountWait = 20;
			newTabAfterMount = false;
			mountCheckerTimer.start( 1000, true );
			return;
		}
		emit openUrl( urls[ elem ] );
	}
}

void MediaButton::gettingSpaceData(const QString &mountPoint, quint64 kBSize, quint64, quint64 ) {
	KUrl mediaURL = KUrl( mountPoint );
	
	KIO::filesize_t size = kBSize;
	size *= 1024;
	QString sizeText = KIO::convertSize( size );
	
	for( int i=0; i != urls.size(); i++ ) {
		if( mediaURL.equals( urls[ i ], KUrl::CompareWithoutTrailingSlash ) ) {
			if( kBSize == 0 ) { // if df gives 0, it means the device is quasy umounted
				QString mime = mimes[ i ];
				if( mimes[ i ].endsWith( "_mounted" ) ) {
					quasiMounted[ i ] = true;
					mimes[ i ] = mimes[ i ].replace( "_mounted", "_unmounted" );
				}
				
				KMimeType::Ptr mt = KMimeType::mimeType( mimes[ i ] );
				QPixmap pixmap;
				if( mt )
					pixmap = FL_LOADICON( mt->iconName() );
				
				idActionMap[ i ]->setIcon( pixmap );
			}
			else if( mimes[ i ].contains( "hdd_" ) )
				idActionMap[ i ]->setText( sizeText + " " + idActionMap[ i ]->text().trimmed() );
			return;
		}
	}
}

void MediaButton::openPopup() {
	QMenu * pP = popup();
	if ( pP ) {
		popup() ->exec( mapToGlobal( QPoint( 0, height() ) ) );
	}
}

void MediaButton::addMountPoint( KMountPoint * mp, bool isMounted ) {
	QString mountString = isMounted ? "_mounted" : "_unmounted";
	if( mp->mountPoint() == "/dev/swap" || 
		mp->mountPoint() == "/dev/pts"  ||
		mp->mountPoint().startsWith( "/sys/kernel" ) ||
		mp->mountPoint().find( "/proc" ) == 0 )
		return;
	if( mp->mountType() == "swap" ||
		mp->mountType() == "sysfs" ||
		mp->mountType() == "tmpfs" ||
		mp->mountType() == "kernfs" ||
		mp->mountType() == "usbfs" ||
		mp->mountType() == "unknown" ||
		mp->mountType() == "none" ||
		mp->mountType() == "sunrpc" )
		return;
	if( mp->mountedFrom() == "none" ||
		mp->mountedFrom() == "tmpfs" ||
		mp->mountedFrom().find( "shm" )  != -1 )
		return;
	
	int overwrite = -1;
	KUrl mountURL = KUrl( mp->mountPoint() );
	
	for( int i=0; i != urls.size(); i++ ) 
		if( urls[ i ].equals( mountURL, KUrl::CompareWithoutTrailingSlash ) ) {
			overwrite = i;
			break;
		}
	
	QString name;
	QString type = detectType( mp );
	
	/* WORKAROUND CODE START */
	/* add spaces to avoid widget resize in gettingSpaceData,
	   which is buggy in QT when transparency is set */
	QString extSpc = ( isMounted && type == "hdd" ) ? extraSpaces : "";
	/* WORKAROUND CODE END */
		
	QString mimeBase = "media/";
	QString mime = mimeBase + type + mountString;
	
	if( type == "hdd" )
		name =  i18n( "Hard Disk" ) ;
	else if( type == "cdrom" )
		name = i18n( "CD-ROM" );
	else if( type == "cdwriter" )
		name = i18n( "CD Recorder" );
	else if( type == "dvdwriter" ) {
		mime = mimeBase + "cdwriter" + mountString;
		name = i18n( "DVD Recorder" );
	}
	else if( type == "dvd" )
		name = i18n( "DVD" );
	else if( type == "smb" )
		name = i18n( "Remote Share" );
	else if( type == "nfs" )
		name = i18n( "Remote Share" );
	else if( type == "floppy" )
		name = i18n( "Floppy" );
	else if( type == "floppy5" )
		name = i18n( "Floppy" );
	else if( type == "zip" )
		name = i18n( "Zip Disk" );
	else {
		mime = mimeBase + "hdd" + mountString;
		name = i18n( "Unknown" );
	}
	
	if( isMounted ) {
		KDiskFreeSpace *sp = KDiskFreeSpace::findUsageInfo( mp->mountPoint() );
		connect( sp, SIGNAL( foundMountPoint( const QString &, quint64, quint64, quint64 ) ),
		         this, SLOT( gettingSpaceData( const QString&, quint64, quint64, quint64 ) ) );
	}
	
	KMimeType::Ptr mt = KMimeType::mimeType( mime );
	QPixmap pixmap;
	if( mt )
		pixmap = FL_LOADICON( mt->iconName() );
	
	if( overwrite == -1 ) {
		int index = popupMenu->count();
		urls.append( KUrl( mp->mountPoint() ) );
		mimes.append( mime );
		mediaUrls.append( KUrl() );
		quasiMounted.append( false );
		QAction * act = popupMenu->addAction( pixmap, name + "  [" + mp->mountPoint() + "]" + extSpc );
		act->setData( QVariant( index ) );
		idActionMap[ index ] = act;
	}
	else {
		mimes[ overwrite ] = mime;
		idActionMap[ overwrite ]->setIcon( pixmap );
		idActionMap[ overwrite ]->setText( name + "  [" + mp->mountPoint() + "]" + extSpc );
	}
}

bool MediaButton::eventFilter( QObject *o, QEvent *e ) {
	if( o == popupMenu ) {
		if( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease ) {
			QMouseEvent *m = (QMouseEvent *)e;
			if( m->button() == Qt::RightButton ) {
				if( e->type() == QEvent::MouseButtonPress ) {
					QAction * act = popupMenu->actionAt( m->pos() );
					int id = -1;
					if( act && act->data().canConvert<int>() )
						id = act->data().toInt();
					if( id != -1 )
						rightClickMenu( id );
				}
 				m->accept();
				return true;
			}
		}
	}
	return false;
}

void MediaButton::rightClickMenu( int index ) {
	if( rightMenu )
		rightMenu->close();
	
	QString mime = mimes[ index ];
	bool ejectable = mime.contains( "dvd_" ) || mime.contains( "dvdwriter_" ) || mime.contains( "cdrom_" ) || mime.contains( "cdwriter_" );
	bool mounted = mime.contains( "_mounted" );
	
	QMenu * myMenu = rightMenu = new QMenu( popupMenu );
	QAction * actOpen = myMenu->addAction( i18n( "Open" ) );
	actOpen->setData( QVariant( 1 ) );
	QAction * actOpenNewTab = myMenu->addAction( i18n( "Open in a new tab" ) );
	actOpenNewTab->setData( QVariant( 2 ) );

	myMenu->addSeparator();
	if( !mounted ) {
		QAction * actMount = myMenu->addAction( i18n( "Mount" ) );
		actMount->setData( QVariant( 3 ) );
	} else {
		QAction * actUnmount = myMenu->addAction( i18n( "Unmount" ) );
		actUnmount->setData( QVariant( 4 ) );
	}
	if( ejectable ) {
		QAction * actEject = myMenu->addAction( i18n( "Eject" ) );
		actEject->setData( QVariant( 5 ) );
	}

	QAction *act = myMenu->exec( QCursor::pos() );

	int result = -1;
	if( act != 0 && act->data().canConvert<int>() )
		result = act->data().toInt();

	delete myMenu;
	if( rightMenu == myMenu )
		rightMenu = 0;
	else
		return;
	
	switch ( result ) {
	case 1:
	case 2:
		popupMenu->close();
		if( mounted || quasiMounted[ index ] ) {
			if( result == 1 )
				emit openUrl( urls[ index ] );
			else
				SLOTS->newTab( urls[ index ] );
		} else {
			mount( index ); // mount first, when mounted open the tab
			waitingForMount = index;
			maxMountWait = 20;
			newTabAfterMount = ( result == 2 );
			mountCheckerTimer.start( 1000, true );
		}
		break;
	case 3:
		mount( index );
		break;
	case 4:
		umount( index );
		break;
	case 5:
		eject( index );
		break;
	default:
		break;
	}
}

bool MediaButton::mount( int index ) {
	if ( index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-m" << mediaUrls[ index ].url();
			proc.startDetached();
		} else {
			krMtMan.mount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::umount( int index ) {
	if ( index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-u" << mediaUrls[ index ].url();
			proc.startDetached();
		} else {
			krMtMan.unmount( urls[ index ].path(), false );
		}
	}
	return false;
}

bool MediaButton::eject( int index ) {
	if ( index < mimes.count() ) {
		if( !mediaUrls[ index ].isEmpty() ) {
			KProcess proc;
			proc << KrServices::fullPathName( "kio_media_mounthelper" ) << "-e" << mediaUrls[ index ].url();
			proc.startDetached();
		} else {
			krMtMan.eject( urls[ index ].path() );
		}
	}
	return false;
}

void MediaButton::slotTimeout() {
	if( isHidden() && ( waitingForMount < 0 ) )
		return;
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	
	for( int index = 0; index < urls.count(); index++ ) {
		bool mounted = false;
		
		QString text = idActionMap[ index ]->text();
		
		if( mediaUrls[ index ].isEmpty() ) {
			for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it)
				if( (*it)->mountPoint() == urls[ index ].path() ) {
					mounted = true;;
					break;
				}
		} else {
			KUrl uri = getLocalPath( mediaUrls[ index ], &mountList );
			if(  uri.isLocalFile() ) {
				urls[ index ] = uri;
				mounted = true;
				
				if( !text.contains( uri.path() ) ) 
				{
					if( text.endsWith( "]" ) )
					{
						int ndx = text.lastIndexOf( "  [" );
						if( ndx >0 )
							text.truncate( ndx );
					}
					
					text += "  [" + uri.path() + "]";
				}
			}
			else
			{
				if( text.endsWith( "]" ) )
				{
					int ndx = text.lastIndexOf( "  [" );
					if( ndx >0 )
						text.truncate( ndx );
				}
			}
		}
		
		if( quasiMounted[ index ] ) // mounted but not listed with DF
			mounted = false;
		
		if( mimes[ index ].contains( "_mounted" ) && !mounted )
			mimes[ index ] = mimes[ index ].replace( "_mounted", "_unmounted" );
		if( mimes[ index ].contains( "_unmounted" ) && mounted )
			mimes[ index ] = mimes[ index ].replace( "_unmounted", "_mounted" );
		
		KMimeType::Ptr mt = KMimeType::mimeType( mimes[ index ] );
		QPixmap pixmap;
		if( mt )
			pixmap = FL_LOADICON( mt->iconName() );
		idActionMap[ index ]->setIcon( pixmap );
		idActionMap[ index ]->setText( text );
		
		if( ((int)index == waitingForMount) && mounted ) {
			waitingForMount = -1;
			if( newTabAfterMount )
				SLOTS->newTab( urls[ index ] );
			else
				emit openUrl( urls[ index ] );
		}
	}
	
	if( waitingForMount >= 0 ) {         // maximum wait for mounting expired ?
		if( --maxMountWait < 0 ) {
			waitingForMount = -1;
			return;
		}
	}
	
	mountCheckerTimer.start( 1000, true );
}


#include "mediabutton.moc"
