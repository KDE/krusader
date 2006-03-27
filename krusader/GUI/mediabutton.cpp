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
#include "../MountMan/kdiskfreesp.h"

#include <qpopupmenu.h>
#include <qfile.h>

#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kmountpoint.h>
#include <kmimetype.h>

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



MediaButton::MediaButton( QWidget *parent, const char *name ) : QToolButton( parent, name ),
		popupMenu( 0 )
	 {
	KIconLoader * iconLoader = new KIconLoader();
	QPixmap icon = iconLoader->loadIcon( "blockdevice", KIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the available media list" ), true );
	setPopupDelay( 1 ); // immediate press
	setAcceptDrops( false );

	popupMenu = new QPopupMenu( this );
	Q_CHECK_PTR( popupMenu );

	setPopup( popupMenu );

	connect( popupMenu, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShow() ) );
	connect( popupMenu, SIGNAL( activated( int ) ), this, SLOT( slotPopupActivated( int ) ) );
}

MediaButton::~MediaButton() {}

void MediaButton::slotAboutToShow() {
	popupMenu->clear();
	urls.clear();
	
	KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
	for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
		addMountPoint( *it, false );
	}
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it) {
		addMountPoint( *it, true );
	}
}

QString MediaButton::detectType( KMountPoint *mp )
{
	QString typeName = QString::null;
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
		if(m.open(IO_ReadOnly))
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

void MediaButton::slotPopupActivated( int elem ) {
	emit openUrl( urls[ elem ] );
}

void MediaButton::gettingSpaceData(const QString &mountPoint, unsigned long kBSize, unsigned long, unsigned long ) {
	KURL mediaURL = KURL::fromPathOrURL( mountPoint );
	
	KIO::filesize_t size = kBSize;
	size *= 1024;
	QString sizeText = KIO::convertSize( size );
	
	for( unsigned i=0; i != urls.size(); i++ ) {
		if( mediaURL.equals( urls[ i ], true ) ) {
			if( kBSize == 0 ) { // if df gives 0, it means the device is quasy umounted
#if KDE_IS_VERSION(3,4,0)
				QString mimeBase = "media/";
#else
				QString mimeBase = "kdedevice/";
#endif
				QString realType = ( types[ i ] == "dvdwriter" ) ? "cdwriter" : types[ i ];
				QPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimeBase +  realType + "_unmounted" ) ->icon( QString::null, true ) );
				popupMenu->changeItem( i, pixmap, popupMenu->text( i ) );
			}
			else if( types[ i ] == "hdd" )
				popupMenu->changeItem( i, sizeText + " " + popupMenu->text( i ) );
			return;
		}
	}
}

void MediaButton::openPopup() {
	QPopupMenu * pP = popup();
	if ( pP ) {
		popup() ->exec( mapToGlobal( QPoint( 0, height() ) ) );
	}
}

void MediaButton::addMountPoint( KMountPoint * mp, bool isMounted ) {
	QString mountString = isMounted ? "_mounted" : "_unmounted";
	if( mp->mountPoint() == "/dev/swap" || 
		mp->mountPoint() == "/dev/pts"  ||
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
	KURL mountURL = KURL::fromPathOrURL( mp->mountPoint() );
	
	for( unsigned i=0; i != urls.size(); i++ ) 
		if( urls[ i ].equals( mountURL, true ) ) {
			overwrite = i;
			break;
		}
	
	QString name;
	QString type = detectType( mp );
		
#if KDE_IS_VERSION(3,4,0)
	QString mimeBase = "media/";
#else
	QString mimeBase = "kdedevice/";
#endif
	
	QString mime = mimeBase + type + mountString;
	
	if( type == "hdd" )
		name = i18n( "Hard Disk" );
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
		KDiskFreeSp *sp = KDiskFreeSp::findUsageInfo( mp->mountPoint() );
		connect( sp, SIGNAL( foundMountPoint( const QString &, unsigned long, unsigned long, unsigned long ) ),
		         this, SLOT( gettingSpaceData( const QString&, unsigned long, unsigned long, unsigned long ) ) );
	}
	
	QPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mime ) ->icon( QString::null, true ) );
	
	if( overwrite == -1 ) {
		int index = popupMenu->count();
		urls.append( KURL::fromPathOrURL( mp->mountPoint() ) );
		types.append( type );
		popupMenu->insertItem( pixmap, name + " [" + mp->mountPoint() + "]", index, index );
	}
	else {
		types[ overwrite ] = type;
		popupMenu->changeItem( overwrite, pixmap, name + " [" + mp->mountPoint() + "]" );
	}
}

#include "mediabutton.moc"
