/***************************************************************************
                              kmountmangui.cpp
                           -------------------
  copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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



#include "kmountmangui.h"
#include "kmountman.h"
#include "../krusader.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../VFS/vfs.h"
#include <klocale.h>
#include <qpixmap.h>
#include <QGridLayout>
#include <QList>
#include <kmenu.h>
#include <qbitmap.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <kdiskfreespace.h>
#include <qcursor.h>
#include <kdebug.h>
#include <kguiitem.h>
#include <qfileinfo.h>
#include <sys/param.h>
#include <qheaderview.h>

#if defined(BSD)
#include <kmountpoint.h>
#include <kcodecs.h>
#else
#define MTAB	"/etc/mtab"
#endif

KMountManGUI::KMountManGUI() : KDialog( krApp ), info( 0 ), mountList( 0 ) {
   setWindowTitle( i18n("Mount.Man") );
   setWindowModality( Qt::WindowModal );

	watcher = new QTimer( this );
   connect( watcher, SIGNAL( timeout() ), this, SLOT( checkMountChange() ) );

   connect( this, SIGNAL( finishedGettingSpaceData() ), this, SLOT( updateList() ) );
   setButtonGuiItem(KDialog::Ok, KGuiItem( i18n( "&Close" ) ) );
   showButton( KDialog::Apply, false );
   showButton( KDialog::Cancel, false );
   setPlainCaption( i18n( "MountMan - Your Mount-Manager" ) );
   createLayout();
   setMainWidget( mainPage );

   // connections
   connect( mountList, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this,
            SLOT( doubleClicked( QTreeWidgetItem * ) ) );
   connect( mountList, SIGNAL( itemRightClicked( QTreeWidgetItem *, const QPoint &, int ) ),
            this, SLOT( clicked( QTreeWidgetItem*, const QPoint & ) ) );
   connect( mountList, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ), this,
            SLOT( changeActive( QTreeWidgetItem * ) ) );
   connect( mountList, SIGNAL( itemSelectionChanged() ), this,
            SLOT( changeActive() ) );

   getSpaceData();
   exec();
}

KMountManGUI::~KMountManGUI() {
   watcher->stop();
   delete watcher;

  KConfigGroup group( krConfig, "MountMan" );

  group.writeEntry( "Last State", mountList->header()->saveState() );
}

void KMountManGUI::createLayout() {
   mainPage = new QWidget( this );
   createMainPage();
}

void KMountManGUI::createMainPage() {
   // check if we need to clean up first!
   if ( mountList != 0 ) {
      mountList->hide();
      delete mountList;
      mountList = 0;
   }
   // clean up is finished...
   QGridLayout *layout = new QGridLayout( mainPage );
   layout->setSpacing( 10 );
   mountList = new KrTreeWidget( mainPage );  // create the main container
   KConfigGroup group( krConfig, "Look&Feel" );
   mountList->setFont( group.readEntry( "Filelist Font", *_FilelistFont ) );
   mountList->setSelectionMode( QAbstractItemView::SingleSelection );
   mountList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   mountList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

   QStringList labels;
   labels << i18n( "Name" );
   labels << i18n( "Type" );
   labels << i18n( "Mnt.Point" );
   labels << i18n( "Total Size" );
   labels << i18n( "Free Size" );
   labels << i18n( "Free %" );
   mountList->setHeaderLabels( labels );

   mountList->header()->setResizeMode( 0, QHeaderView::Interactive );
   mountList->header()->setResizeMode( 1, QHeaderView::Interactive );
   mountList->header()->setResizeMode( 2, QHeaderView::Interactive );
   mountList->header()->setResizeMode( 3, QHeaderView::Interactive );
   mountList->header()->setResizeMode( 4, QHeaderView::Interactive );
   mountList->header()->setResizeMode( 5, QHeaderView::Interactive );

   KConfigGroup grp( krConfig, "MountMan" );
   if( grp.hasKey( "Last State" ) )
      mountList->header()->restoreState( grp.readEntry( "Last State", QByteArray() ) );
   else
   {
     int i = QFontMetrics( mountList->font() ).width( "W" );
     int j = QFontMetrics( mountList->font() ).width( "0" );
     j = ( i > j ? i : j );

     mountList->setColumnWidth(0, j*8);
     mountList->setColumnWidth(1, j*4);
     mountList->setColumnWidth(2, j*8);
     mountList->setColumnWidth(3, j*6);
     mountList->setColumnWidth(4, j*6);
     mountList->setColumnWidth(5, j*5);
   }

   mountList->setAllColumnsShowFocus( true );
   mountList->header()->setSortIndicatorShown( true );

   mountList->sortItems( 0, Qt::AscendingOrder );

   // now the list is created, time to fill it with data.
   //=>krMtMan.forceUpdate();
   QGroupBox *box = new QGroupBox( i18n( "MountMan.Info" ), mainPage );
   box->setAlignment( Qt::AlignHCenter );
   QVBoxLayout *vboxl = new QVBoxLayout;
   vboxl->addWidget( info = new KRFSDisplay( box ) );
   info->resize( info->width(), height() );
   box->setLayout( vboxl );
   layout->addWidget( box, 0, 0 );
   layout->addWidget( mountList, 0, 1 );

   mainPage->setLayout( layout );
}

void KMountManGUI::getSpaceData() {
   fileSystemsTemp.clear();
	KrMountDetector::getInstance()->hasMountsChanged();
	
   mounted = KMountPoint::currentMountPoints();
	possible = KMountPoint::possibleMountPoints();
   if ( mounted.size() == 0 ) { // nothing is mounted
      updateList(); // let's continue
      return ;
   }

   numOfMountPoints = mounted.size();
   for ( KMountPoint::List::iterator it = mounted.begin(); it != mounted.end(); ++it ) {
   	  // don't bother with invalid file systems
      if (krMtMan.invalidFilesystem((*it)->mountType())) {
			--numOfMountPoints;
			continue;
		}
      KDiskFreeSpace *sp = KDiskFreeSpace::findUsageInfo( ( *it ) ->mountPoint() );
      connect( sp, SIGNAL( foundMountPoint( const QString &, quint64, quint64, quint64 ) ),
               this, SLOT( gettingSpaceData( const QString&, quint64, quint64, quint64 ) ) );
      connect( sp, SIGNAL( done() ), this, SLOT( gettingSpaceData() ) );
   }
}

// this decrements the counter, while the following uses the data
// used when certain filesystem (/dev, /sys) can't have the needed stats
void KMountManGUI::gettingSpaceData() {
   if ( --numOfMountPoints == 0 ) {
      fileSystems = fileSystemsTemp;
      emit finishedGettingSpaceData();
   }
}

void KMountManGUI::gettingSpaceData( const QString &mountPoint, quint64 kBSize,
                                     quint64 /*kBUsed*/, quint64 kBAvail ) {
   KSharedPtr<KMountPoint> m = KMountMan::findInListByMntPoint( mounted, mountPoint );
   if ( !( (bool)m ) ) { // this should never never never happen!
      KMessageBox::error( 0, i18n( "Critical Error" ),
                          i18n( "Internal error in MountMan\nPlease email the developers" ) );
      exit( 1 );
   }
   fsData data;
   data.setMntPoint( mountPoint );
   data.setMounted( true );
   data.setTotalBlks( kBSize );
   data.setFreeBlks( kBAvail );
   data.setName( m->mountedFrom() );
   data.setType( m->mountType() );
   fileSystemsTemp.append( data );
}

void KMountManGUI::addItemToMountList( KrTreeWidget *lst, fsData &fs ) {
   bool mtd = fs.mounted();

   QString tSize = QString( "%1" ).arg( KIO::convertSizeFromKiB( fs.totalBlks() ) );
   QString fSize = QString( "%1" ).arg( KIO::convertSizeFromKiB( fs.freeBlks() ) );
   QString sPrct = QString( "%1%" ).arg( 100 - ( fs.usedPerct() ) );
   QTreeWidgetItem *item = new QTreeWidgetItem( lst );
   item->setText( 0, fs.name() );
   item->setText( 1, fs.type() );
   item->setText( 2, fs.mntPoint() );
   item->setText( 3, ( mtd ? tSize : QString( "N/A" ) ) );
   item->setText( 4, ( mtd ? fSize : QString( "N/A" ) ) );
   item->setText( 5, ( mtd ? sPrct : QString( "N/A" ) ) );
   
	QString id = fs.name().left(7); // only works assuming devices start with  "/dev/XX"
   QPixmap *icon = 0;
   if ( id == "/dev/fd") {
      icon = new QPixmap( LOADICON( this, mtd ? "3floppy_mount" : "3floppy_unmount" ) );
	} else if ( id == "/dev/cd" || fs.type() == "iso9660" ) {
		icon = new QPixmap( LOADICON( this, mtd ? "cdrom_mount" : "cdrom_unmount" ) );
   } else if ( fs.type() == "nfs" || fs.type() == "smbfs" ) {
		icon = new QPixmap( LOADICON( this, mtd ? "nfs_mount" : "nfs_unmount" ) );
	} else icon = new QPixmap( LOADICON( this, mtd ? "hdd_mount" : "hdd_unmount" ) );

   item->setIcon( 0, *icon );
   delete icon;
}

void KMountManGUI::updateList() {
   mountList->clear();
   // this handles the mounted ones
	for ( QList<fsData>::iterator it = fileSystems.begin(); it != fileSystems.end() ; ++it ) {
		if (krMtMan.invalidFilesystem((*it).type())) {
			continue;
		}
      addItemToMountList( mountList, *it );
   }
	
	// now, handle the non-mounted ones
	for (KMountPoint::List::iterator it = possible.begin(); it != possible.end(); ++it) {
		// make sure we don't add things we've already added
		if (KMountMan::findInListByMntPoint(mounted, (*it)->mountPoint())) {
			continue;
		} else {
			fsData data;
			data.setMntPoint((*it)->mountPoint());
			data.setMounted(false);
			data.setType((*it)->mountType());
			data.setName((*it)->mountedFrom());
			fileSystems.append(data);
			
			if (krMtMan.invalidFilesystem(data.type())) continue;
			addItemToMountList(mountList, data);
		}
	}
	
   mountList->clearSelection();
   if ( info ) {
      info->setEmpty( true );
      info->repaint();
   }
   watcher->setSingleShot( true );
   watcher->start( WATCHER_DELAY );   // starting the watch timer ( single shot )
}

void KMountManGUI::checkMountChange() {
	if (KrMountDetector::getInstance()->hasMountsChanged())
		getSpaceData();
   watcher->setSingleShot( true );
   watcher->start( WATCHER_DELAY );   // starting the watch timer ( single shot )
}

void KMountManGUI::doubleClicked( QTreeWidgetItem *i ) {
   if ( !i )
		return; // we don't want to refresh to swap, do we ?
		 
   // change the active panel to this mountpoint
   connect( ( QObject* ) this, SIGNAL( refreshPanel( const KUrl & ) ), ( QObject* ) SLOTS,
            SLOT( refresh( const KUrl & ) ) );
   emit refreshPanel( KUrl( i->text(2) ) ); // text(2) ? so ugly ... 
   disconnect( this, SIGNAL( refreshPanel( const KUrl & ) ), 0, 0 );
   close();
}

void KMountManGUI::changeActive() {
   QList<QTreeWidgetItem *> seld = mountList->selectedItems();
   if( seld.count() > 0 )
     changeActive( seld[ 0 ] );
}

// when user clicks on a filesystem, change information
void KMountManGUI::changeActive( QTreeWidgetItem *i ) {
	if ( !i ) return ;
   fsData *system = 0;
	
	for (QList<fsData>::Iterator it = fileSystems.begin(); it != fileSystems.end(); ++it) {
		// the only thing which is unique is the mount point
		if ((*it).mntPoint() == i->text(2)) { // text(2) ? ugly ugly ugly
			system = &(*it);
			break;
		}
	}
	
	if (system == 0) {
		KMessageBox::error(0, i18n("Critical Error"), i18n("Internal error in MountMan\nCall the developers"));
		exit(1);
	}
   info->setAlias( system->mntPoint() );
   info->setRealName( system->name() );
   info->setMounted( system->mounted() );
   info->setEmpty( false );
	info->setTotalSpace( system->totalBlks() );
   info->setFreeSpace( system->freeBlks() );
   info->repaint();
}

// called when right-clicked on a filesystem
void KMountManGUI::clicked( QTreeWidgetItem *item, const QPoint & pos ) {
   // these are the values that will exist in the menu
#define MOUNT_ID       90
#define UNMOUNT_ID     91
#define FORMAT_ID      93
#define EJECT_ID       94
   //////////////////////////////////////////////////////////
	if ( !item ) return ;
	
	fsData *system = 0;
   for (QList<fsData>::Iterator it = fileSystems.begin(); it != fileSystems.end(); ++it) {
		// the only thing which is unique is the mount point
		if ((*it).mntPoint() == item->text(2)) { // text(2) ? ugly ugly ugly
			system = &(*it);
			break;
		}
	}
	
   if ( !system ) {
      KMessageBox::error( 0, i18n( "MountMan has an internal error. Please notify the developers. Thank you." ) );
      exit( 0 );
   }
   // create the menu
   KMenu popup;
   popup.setTitle( i18n( "MountMan" ) );
   if ( !system->mounted() ) {
      QAction *mountAct = popup.addAction( i18n( "Mount" ) );
      mountAct->setData( QVariant( MOUNT_ID ) );
		bool enable = !(krMtMan.nonmountFilesystem(system->type(), system->mntPoint()));
		mountAct->setEnabled( enable);
	} else {
		QAction * umountAct =  popup.addAction( i18n( "Unmount" ) );
		umountAct->setData( QVariant( UNMOUNT_ID ) );
		bool enable = !(krMtMan.nonmountFilesystem(system->type(), system->mntPoint()));
		umountAct->setEnabled( enable);
	}
   if ( krMtMan.ejectable( system->mntPoint() ) )
      //  if (system->type()=="iso9660" || krMtMan.followLink(system->name()).left(2)=="cd")
      popup.addAction( i18n( "Eject" ) )->setData( QVariant( EJECT_ID ) );
   else {
      QAction *formatAct = popup.addAction( i18n( "Format" ) );
      formatAct->setData( QVariant( FORMAT_ID ) );
      formatAct->setEnabled( false );
   }

   QString mountPoint = system->mntPoint();

   QAction * res = popup.exec( pos );
   int result = -1;

   if( res && res->data().canConvert<int>() )
     result = res->data().toInt();

   // check out the user's option
   switch ( result ) {
         case - 1 : return ;     // the user clicked outside of the menu
         case MOUNT_ID :
         case UNMOUNT_ID :
         krMtMan.toggleMount( mountPoint );
         break;
         case FORMAT_ID :
         break;
         case EJECT_ID :
         KMountMan::eject( mountPoint );
         break;
   }
}

KrMountDetector::KrMountDetector() {
   hasMountsChanged();
}

bool KrMountDetector::hasMountsChanged() {
#if defined(BSD)
   KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
   KMD5 md5;
   for(KMountPoint::List::iterator i = mountPoints.begin(); i != mountPoints.end(); ++i) {
      md5.update((*i)->mountedFrom().toUtf8());
      md5.update((*i)->realDeviceName().toUtf8());
      md5.update((*i)->mountPoint().toUtf8());
      md5.update((*i)->mountType().toUtf8());
   }
   QString s = md5.hexDigest();
   bool result = s != checksum;
   checksum = s;
#else
   bool result = QFileInfo(MTAB).lastModified() != lastMtab;
   lastMtab = QFileInfo(MTAB).lastModified();
#endif
   return result;
}

KrMountDetector krMountDetector;
KrMountDetector * KrMountDetector::getInstance() {
   return & krMountDetector;
}

#include "kmountmangui.moc"
