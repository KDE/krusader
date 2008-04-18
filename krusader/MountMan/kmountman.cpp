/***************************************************************************
                        kmountman.cpp
                     -------------------
copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------

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


#include <sys/param.h>
#include <time.h>
#include "kmountman.h" 
// KDE includes
#include <kmessagebox.h>
#include <kprocess.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kio/jobuidelegate.h>
#include <kuiserverjobtracker.h>
#include <ktoolbarpopupaction.h>

// Krusader includes
#include "../krusader.h"
#include "../defaults.h"
#include "../Dialogs/krdialogs.h"
#include "../krservices.h"
#include "kmountmangui.h"
#include <unistd.h>
#include "../VFS/krpermhandler.h"

#ifdef _OS_SOLARIS_
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

static int __delayedIdx; // ugly: pass the processEvents deadlock

KMountMan::KMountMan() : QObject(), Operational( false ), waiting(false), mountManGui( 0 ) {
   _actions = 0L;

	// added as a precaution, although we use kde services now
	if( !KrServices::cmdExist( "df" ) || !KrServices::cmdExist( "mount" ) ) {
    	Operational = false;
	} else { 
		Operational = true;
	}

	// list of FS that we don't manage at all
	invalid_fs << "swap" << "/dev/pts" << "tmpfs" << "devpts" << "sysfs" << "rpc_pipefs" << "usbfs" << "binfmt_misc";
#if defined(BSD)
	invalid_fs << "procfs";
#else
	invalid_fs << "proc";
#endif

	// list of FS that we don't allow to mount/unmount
	nonmount_fs << "supermount";
	{
		KConfigGroup group( krConfig, "Advanced");
		QStringList nonmount = QStringList::split(",", group.readEntry("Nonmount Points", _NonMountPoints));
		nonmount_fs_mntpoint += nonmount;
		// simplify the white space
		for ( QStringList::Iterator it = nonmount_fs_mntpoint.begin(); it != nonmount_fs_mntpoint.end(); ++it ) {
			*it = (*it).simplified();
		}
	}
	
}

KMountMan::~KMountMan() {}

bool KMountMan::invalidFilesystem(QString type) {
	return (invalid_fs.contains(type) > 0);
}

// this is an ugly hack, but type can actually be a mountpoint. oh well...
bool KMountMan::nonmountFilesystem(QString type, QString mntPoint) {
	return((nonmount_fs.contains(type) > 0) || (nonmount_fs_mntpoint.contains(mntPoint) > 0));
}

void KMountMan::mainWindow() {
   mountManGui = new KMountManGUI();
   delete mountManGui;   /* as KMountManGUI is modal, we can now delete it */
   mountManGui = 0; /* for sanity */
}

KSharedPtr<KMountPoint> KMountMan::findInListByMntPoint(KMountPoint::List &lst, QString value) {
	KSharedPtr<KMountPoint> m;
	for (KMountPoint::List::iterator it = lst.begin(); it != lst.end(); ++it) {
		m = *it;
		if (m->mountPoint() == value)
			return m;
	}
	
	return KSharedPtr<KMountPoint>();
}

void KMountMan::jobResult(KJob *job) {
	waiting = false;
	if ( job->error() )
		job->uiDelegate()->showErrorMessage();
}

void KMountMan::mount( QString mntPoint, bool blocking ) {
	KMountPoint::List possible = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions);
	KSharedPtr<KMountPoint> m = findInListByMntPoint(possible, mntPoint);
	if (!((bool)m)) return;
	if (blocking)
	   waiting = true; // prepare to block
	KIO::SimpleJob *job = KIO::mount(false, m->mountType().local8Bit(), m->mountedFrom(), m->mountPoint(), false);
	job->setUiDelegate(new KIO::JobUiDelegate() );
	KIO::getJobTracker()->registerJob(job);
	connect(job, SIGNAL(result(KJob* )), this, SLOT(jobResult(KJob* )));
	while (blocking && waiting) {
		qApp->processEvents();
		usleep( 1000 );
	}
}

void KMountMan::unmount( QString mntPoint, bool blocking ) {
	if (blocking)
	   waiting = true; // prepare to block
	KIO::SimpleJob *job = KIO::unmount(mntPoint, false);
	job->setUiDelegate(new KIO::JobUiDelegate() );
	KIO::getJobTracker()->registerJob(job);
	connect(job, SIGNAL(result(KJob* )), this, SLOT(jobResult(KJob* )));
	while (blocking && waiting) {
		qApp->processEvents();
		usleep( 1000 );
	}
}

KMountMan::mntStatus KMountMan::getStatus( QString mntPoint ) {	
	KMountPoint::List::iterator it;
   KSharedPtr<KMountPoint> m;
	
	// 1: is it already mounted
	KMountPoint::List current = KMountPoint::currentMountPoints();
	m = findInListByMntPoint(current, mntPoint);
	if ((bool)m) 
		return MOUNTED;
	
	// 2: is it a mount point but not mounted?
	KMountPoint::List possible = KMountPoint::possibleMountPoints();
	m = findInListByMntPoint(possible, mntPoint);
	if ((bool)m) 
		return NOT_MOUNTED;
	
	// 3: unknown
	return DOESNT_EXIST;
}


void KMountMan::toggleMount( QString mntPoint ) {
	mntStatus status = getStatus(mntPoint);
	switch (status) {
		case MOUNTED:
			unmount(mntPoint);
			break;
		case NOT_MOUNTED:
			mount(mntPoint);
			break;
		case DOESNT_EXIST:
			// do nothing: no-op to make the compiler quiet ;-)
			break;
	}
}

void KMountMan::autoMount( QString path ) {
   if ( getStatus( path ) == NOT_MOUNTED )
      mount( path );
}

void KMountMan::eject( QString mntPoint ) {
   KProcess proc;
   proc << KrServices::fullPathName( "eject" ) << mntPoint;
   proc.start();
   proc.waitForFinished(-1); // -1 msec blocks without timeout
   if ( proc.exitStatus() != QProcess::NormalExit || proc.exitStatus() != 0 ) // if we failed with eject
      KMessageBox::information( 0, //parent
          i18n( "<qt>Error ejecting device!\n You have to configure the path to the 'eject' tool."
                "Please check the <b>Dependencies</b> page in Krusader's settings.</qt>"),
          i18n( "Error" ), // caption
          "CantExecuteEjectWarning" ); // don't-show-again config-key
}

// returns true if the path is an ejectable mount point (at the moment CDROM and DVD)
bool KMountMan::ejectable( QString path ) {
#if !defined(BSD) && !defined(_OS_SOLARIS_)
	KMountPoint::List possible = KMountPoint::possibleMountPoints();
	KSharedPtr<KMountPoint> m = findInListByMntPoint(possible, path);
	if (m && (m->mountType()=="iso9660" || m->mountedFrom().left(7)=="/dev/cd" || m->mountedFrom().left(8)=="/dev/dvd"))
			return KrServices::cmdExist( "eject" );
#endif

   return false;
}


// a mountMan special version of KIO::convertSize, which deals
// with large filesystems ==> >4GB, it actually recieve size in
// a minimum block of 1024 ==> data is KB not bytes
QString KMountMan::convertSize( KIO::filesize_t size ) {
   float fsize;
   QString s;
   // Tera-byte
   if ( size >= 1073741824 ) {
      fsize = ( float ) size / ( float ) 1073741824;
      if ( fsize > 1024 )         // no name for something bigger than tera byte
         // let's call it Zega-Byte, who'll ever find out? :-)
         s = i18n( "%1 ZB", KGlobal::locale() ->formatNumber( fsize / ( float ) 1024, 1 ) );
      else
         s = i18n( "%1 TB", KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Giga-byte
   else if ( size >= 1048576 ) {
      fsize = ( float ) size / ( float ) 1048576;
      s = i18n( "%1 GB", KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Mega-byte
   else if ( size > 1024 ) {
      fsize = ( float ) size / ( float ) 1024;
      s = i18n( "%1 MB", KGlobal::locale() ->formatNumber( fsize, 1 ) );
   }
   // Kilo-byte
   else {
      fsize = ( float ) size;
      s = i18n( "%1 KB", KGlobal::locale() ->formatNumber( fsize, 0 ) );
   }
   return s;
}


// populate the pop-up menu of the mountman tool-button with actions
void KMountMan::quickList() {
   if ( !Operational ) {
      KMessageBox::error( 0, i18n( "MountMan is not operational. Sorry" ) );
      return ;
   }

   // clear the popup menu
   ( ( KToolBarPopupAction* ) krMountMan ) ->menu() ->clear();

   // create lists of current and possible mount points
   KMountPoint::List current = KMountPoint::currentMountPoints();
   KMountPoint::List possible = KMountPoint::possibleMountPoints();

   // create a menu, displaying mountpoints with possible actions
   // also, populate a small array with the actions
   if ( _actions )
      delete[] _actions;
   _actions = new QString[ possible.size() ];

   KMountPoint::List::iterator it;
   KSharedPtr<KMountPoint> m;
   int idx;
   for ( it = possible.begin(), idx = 0; it != possible.end(); ++it, ++idx ) {
      m = *it;
		// skip nonmountable file systems
		if (nonmountFilesystem(m->mountType(), m->mountPoint()) || invalidFilesystem(m->mountType()))
			continue;
      // does the mountpoint exist in current list? if so, it can only
      // be umounted, otherwise, it can be mounted
      bool needUmount = false;
      KMountPoint::List::iterator otherIt;
      for ( otherIt = current.begin(); otherIt != current.end(); ++otherIt ) {
         if ( ( *otherIt ) ->mountPoint() == m->mountPoint() ) { // found it, in needs umount
            needUmount = true;
            break;
         }
      }
      // add the item to the menu
      _actions[ idx ] = QString( needUmount ? "_U_" : "_M_" ) + m->mountPoint();
      QString text = QString( ( needUmount ? i18n( "Unmount" ) : i18n( "Mount" ) ) ) + " " + m->mountPoint() +
                     " (" + m->mountedFrom() + ")";


      ( ( KToolBarPopupAction* ) krMountMan ) ->menu() ->insertItem( text, idx );
   }
   connect( ( ( KToolBarPopupAction* ) krMountMan ) ->menu(), SIGNAL( activated( int ) ),
            this, SLOT( delayedPerformAction( int ) ) );

}

void KMountMan::delayedPerformAction( int idx ) {
   __delayedIdx = idx;
   QTimer::singleShot(0, this, SLOT(performAction(int)));   
}

void KMountMan::performAction( int idx ) {
   while ( qApp->hasPendingEvents() )
      qApp->processEvents();

   // ugly !!! take idx from the value put there by delayedPerformAction so 
   // as to NOT DIE because of a processEvents deadlock!!! @#$@!@
   idx = __delayedIdx;
   
   if ( idx < 0 )
      return ;
   bool domount = _actions[ idx ].left( 3 ) == "_M_";
   QString mountPoint = _actions[ idx ].mid( 3 );
   if ( !domount ) { // umount
      unmount( mountPoint);
   } else { // mount
      mount( mountPoint);
   }

   // free memory
   delete[] _actions;
   _actions = 0L;
   disconnect( ( ( KToolBarPopupAction* ) krMountMan ) ->menu(), SIGNAL( activated( int ) ), 0, 0 );
}

#include "kmountman.moc"
