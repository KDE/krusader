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

MediaButton::MediaButton( QWidget *parent, const char *name ) : QToolButton( parent, name ),
		popupMenu( 0 )
	 {
	KIconLoader * iconLoader = new KIconLoader();
	QPixmap icon = iconLoader->loadIcon( "blockdevice", KIcon::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setPixmap( icon );
	setTextLabel( i18n( "Open the available media list" ), true );
	setPopupDelay( 1 ); // 0.01 seconds press
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
	
	int index = 0;
	
	KMountPoint::List mountList = KMountPoint::currentMountPoints();
	for (KMountPoint::List::iterator it = mountList.begin(); it != mountList.end(); ++it) {
		if( (*it)->mountPoint() == "/dev/swap" || 
		    (*it)->mountPoint() == "/dev/pts"  ||
		    (*it)->mountPoint().find( "/proc" ) == 0 )
			continue;
		if( (*it)->mountType() == "swap"       ||
		    (*it)->mountedFrom() == "sysfs" ||
		    (*it)->mountType() == "tmpfs" )
			continue;
		if( (*it)->mountedFrom() == "none" ||
		    (*it)->mountedFrom() == "tmpfs" ||
		    (*it)->mountedFrom().find( "shm" )  != -1 )
			continue;
		
		urls.append( KURL::fromPathOrURL( (*it)->mountPoint() ) );
		
		QString mime;
		QString name;
		QString type = detectType( (*it) );
		
#if KDE_IS_VERSION(3,4,0)
		QString mimeBase = "media/";
#else
		QString mimeBase = "kdedevice/";
#endif

		if( type == "hdd" ) {
			mime = "hdd_mounted";
			name = i18n( "Hard Disc" );
			
			KDiskFreeSp *sp = KDiskFreeSp::findUsageInfo( ( *it ) ->mountPoint() );
			connect( sp, SIGNAL( foundMountPoint( const QString &, unsigned long, unsigned long, unsigned long ) ),
			         this, SLOT( gettingSpaceData( const QString&, unsigned long, unsigned long, unsigned long ) ) );
		} else if( type == "cdrom" ) {
			mime = "cdrom_mounted";
			name = i18n( "CD-ROM" );
		} else if( type == "cdwriter" ) {
			mime = "cdwriter_mounted";
			name = i18n( "CD Recorder" );
		} else if( type == "dvd" ) {
			mime = "dvd_mounted";
			name = i18n( "DVD" );
		} else if( type == "smb" ) {
			mime = "smb_mounted";
			name = i18n( "Remote Share" );
		} else if( type == "nfs" ) {
			mime = "nfs_mounted";
			name = i18n( "Remote Share" );
		} else if( type == "floppy" ) {
			mime = "floppy_mounted";
			name = i18n( "Floppy" );
		} else if( type == "floppy5" ) {
			mime = "floppy5_mounted";
			name = i18n( "Floppy" );
		} else if( type == "zip" ) {
			mime = "zip_mounted";
			name = i18n( "Zip Disk" );
		} else {
			mime = "hdd_mounted";
			name = i18n( "Unknown" );
		}
		
		QPixmap pixmap = FL_LOADICON( KMimeType::mimeType( mimeBase+mime ) ->icon( QString::null, true ) );
		popupMenu->insertItem( pixmap, name + " [" + (*it)->mountPoint() + "]", index, index );
		index++;
	}
}

QString MediaButton::detectType( KMountPoint *mp )
{
	QString typeName;
#ifdef Q_OS_LINUX
	QString str;
	QString tmpInfo;
	if (mp->mountedFrom().startsWith("/dev/hd")) {
		QString tmp=mp->mountedFrom();
	tmp=tmp.right(tmp.length()-5);
	tmp=tmp.left(3);
	QString mediafilename="/proc/ide/"+tmp+"/media";
	QString modelfilename="/proc/ide/"+tmp+"/model";

	QFile infoFile(mediafilename);
	if (infoFile.open(IO_ReadOnly))
	{
		if (-1 == infoFile.readLine(tmpInfo,20)) 
			typeName="hdd";
		else
		{
			if (tmpInfo.contains("disk"))   // disks
				typeName="hdd";

			else if (tmpInfo.contains("cdrom")) {    // cdroms and cdwriters
					QFile modelFile(modelfilename);
					if(modelFile.open(IO_ReadOnly)) {
						if( -1 != modelFile.readLine(tmpInfo,80) ) {
							tmpInfo = tmpInfo.lower();
							if( tmpInfo.contains("-rw") ||
							    tmpInfo.contains("cdrw") ||
							    tmpInfo.contains("dvd-rw") ||
							    tmpInfo.contains("dvd+rw") )
								typeName="cdwriter";
							else 
								typeName="cdrom";
						}
						else
							typeName="cdrom";
						modelFile.close();
					}
					else 
						typeName="cdrom";
			}
			else if (tmpInfo.contains("floppy")) 
				typeName="zip"; // eg IDE zip drives
					
			else 
				typeName="hdd";
		}
		infoFile.close();
	} 
	else 
		typeName="hdd"; // this should never be reached
    }
    else
#elif defined(__FreeBSD__)
    if (-1!=mp->mountedFrom().find("/acd",0,FALSE)) typeName="cdrom";
    else if (-1!=mp->mountedFrom().find("/scd",0,FALSE)) typeName="cdrom";
    else if (-1!=mp->mountedFrom().find("/ad",0,FALSE)) typeName="hdd";
    else if (-1!=mp->mountedFrom().find("/da",0,FALSE)) typeName="hdd";
    else if (-1!=mp->mountedFrom().find("/afd",0,FALSE)) typeName="zip";
#if 0
    else if (-1!=mp->mountedFrom().find("/ast",0,FALSE)) typeName="tape";
#endif
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
    else if (-1!=mp->mountPoint().find("dvd",0,FALSE)) typeName="dvd";   
    else if (-1!=mp->mountedFrom().find("dvd",0,FALSE)) typeName="dvd";   
    else if (-1!=mp->mountedFrom().find("/dev/scd",0,FALSE)) typeName="cdrom";
    else if (-1!=mp->mountedFrom().find("/dev/sr",0,FALSE)) typeName="cdrom";


    /* Guessing of floppy types */
    else if (-1!=mp->mountedFrom().find("fd",0,FALSE)) {
            if (-1!=mp->mountedFrom().find("360",0,FALSE)) typeName="floppy5";
            if (-1!=mp->mountedFrom().find("1200",0,FALSE)) typeName="floppy5";
            else typeName+="floppy";
         }
    else if (-1!=mp->mountPoint().find("floppy",0,FALSE)) typeName="floppy";


    else if (-1!=mp->mountPoint().find("zip",0,FALSE)) typeName+="zip";
    else if (-1!=mp->mountType().find("nfs",0,FALSE)) typeName="nfs";
    else if (-1!=mp->mountType().find("smb",0,FALSE)) typeName="smb";
    else if (-1!=mp->mountedFrom().find("//",0,FALSE)) typeName="smb";
    else typeName="hdd";

  return typeName;
}

void MediaButton::slotPopupActivated( int elem ) {
	emit openUrl( urls[ elem ] );
}

void MediaButton::gettingSpaceData(const QString &mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail) {
	KURL mediaURL = KURL::fromPathOrURL( mountPoint );
	
	KIO::filesize_t size = kBSize;
	size *= 1024;
	QString sizeText = KIO::convertSize( size );
	
	for( unsigned i=0; i != urls.size(); i++ ) {
		if( mediaURL.equals( urls[ i ], true ) ) {
			popupMenu->changeItem( i, sizeText + " " + popupMenu->text( i ) );
			return;
		}
	}
}

void MediaButton::openPopup() {
	animateClick();
}

#include "mediabutton.moc"
