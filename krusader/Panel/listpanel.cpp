/***************************************************************************
                         listpanel.cpp
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

// QT includes
#include <qbitmap.h>
#include <qwhatsthis.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <kurldrag.h>
#include <qheader.h>
#include <qtimer.h>
#include <qregexp.h> 
#include <qsplitter.h>
// KDE includes
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include <ktrader.h>
#include <krun.h>
#include <kopenwith.h>
#include <kuserprofile.h>
#include <kiconloader.h>
#include <kshred.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <qtooltip.h>
#include <kdeversion.h>
#include <qimage.h>
#include <qtabbar.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <kurl.h> 
#include <kmountpoint.h>
// Krusader includes
#include "../krusader.h"
#include "../krslots.h"
#include "../kicons.h"
#include "../VFS/krpermhandler.h"
#include "listpanel.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../defaults.h"
#include "../resources.h"
#include "panelfunc.h"
#include "../MountMan/kmountman.h"
#include "../Dialogs/krdialogs.h"
#include "../BookMan/bookmarksbutton.h"
#include "../Dialogs/krspwidgets.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../GUI/kcmdline.h"
#include "krdetailedview.h"
#include "krpreviewpopup.h"
#include "../GUI/dirhistorybutton.h"
#include "../GUI/dirhistoryqueue.h"
#include "../GUI/syncbrowsebutton.h"
#include "../krservices.h"
#include "panelpopup.h" 

typedef QValueList<KServiceOffer> OfferList;

/////////////////////////////////////////////////////
// 					The list panel constructor             //
/////////////////////////////////////////////////////
ListPanel::ListPanel( QWidget *parent, bool &left, const char *name ) :
      QWidget( parent, name ), colorMask( 255 ), compareMode( false ), currDragItem( 0 ), statsAgent( 0 ), 
		quickSearch( 0 ), cdRootButton( 0 ), cdUpButton( 0 ), popupBtn(0), popup(0), _left( left ) {

   func = new ListPanelFunc( this );
   setAcceptDrops( true );
   layout = new QGridLayout( this, 3, 3 );

   status = new KrSqueezedTextLabel( this );
   krConfig->setGroup( "Look&Feel" );
   status->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   status->setBackgroundMode( PaletteBackground );
   status->setFrameStyle( QFrame::Box | QFrame::Raised );
   status->setLineWidth( 1 );		// a nice 3D touch :-)
   status->setText( "" );        // needed for initialization code!
   status->enableDrops( true );
   int sheight = QFontMetrics( status->font() ).height() + 4;
   status->setMaximumHeight( sheight );
   QWhatsThis::add
      ( status, i18n( "The status bar displays information about the FILESYSTEM "
                      "which hold your current directory: Total size, free space, "
                      "type of filesystem etc." ) );
   connect( status, SIGNAL( clicked() ), this, SLOT( slotFocusOnMe() ) );
   connect( status, SIGNAL( dropped( QDropEvent *) ), this, SLOT( handleDropOnStatus(QDropEvent *) ) );  

   // ... create the history button
   dirHistoryQueue = new DirHistoryQueue( this );
   historyButton = new DirHistoryButton( dirHistoryQueue, this, "historyButton" );
   connect( historyButton, SIGNAL( pressed() ), this, SLOT( slotFocusOnMe() ) );
   connect( historyButton, SIGNAL( openUrl( const KURL& ) ), func, SLOT( delayedOpenUrl( const KURL& ) ) );

   // ... create the bookmark list
   bookmarksButton = new BookmarksButton( this );
   connect( bookmarksButton, SIGNAL( pressed() ), this, SLOT( slotFocusOnMe() ) );
   connect( bookmarksButton, SIGNAL( openUrl( const KURL& ) ), func, SLOT( delayedOpenUrl( const KURL& ) ) );
   QWhatsThis::add
      ( bookmarksButton, i18n( "Open menu with bookmarks. You can also add "
                               "current location to the list, edit bookmarks "
                               "or add subfolder to the list." ) );

   QHBoxLayout *totalsLayout = new QHBoxLayout(this);
	totals = new KrSqueezedTextLabel( this );
   krConfig->setGroup( "Look&Feel" );
   totals->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   totals->setFrameStyle( QFrame::Box | QFrame::Raised );
   totals->setBackgroundMode( PaletteBackground );
   totals->setLineWidth( 1 );		// a nice 3D touch :-)
   totals->setMaximumHeight( sheight );
   totals->enableDrops( true );
   QWhatsThis::add
      ( totals, i18n( "The totals bar shows how much files exist, "
                      "how many did you select and the bytes math" ) );
   connect( totals, SIGNAL( clicked() ), this, SLOT( slotFocusOnMe() ) );
   connect( totals, SIGNAL( dropped( QDropEvent *) ), this, SLOT( handleDropOnTotals(QDropEvent *) ) );  
   
	// a quick button to open the popup panel
	popupBtn = new QToolButton( this, "popupbtn" );
   popupBtn->setFixedSize( 20, totals ->height() );
	popupBtn->setPixmap(krLoader->loadIcon("up", KIcon::Panel));
	connect(popupBtn, SIGNAL(clicked()), this, SLOT(togglePanelPopup()));
	QToolTip::add(  popupBtn, i18n( "Open the popup panel" ) );
	totalsLayout->addWidget(totals);
	totalsLayout->addWidget(popupBtn);
   
   quickSearch = new KrQuickSearch( this );
   krConfig->setGroup( "Look&Feel" );
   quickSearch->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   quickSearch->setFrameStyle( QFrame::Box | QFrame::Raised );
   quickSearch->setLineWidth( 1 );		// a nice 3D touch :-)
   quickSearch->setMaximumHeight( sheight );

   QHBox * hbox = new QHBox( this );
   origin = new KURLRequester( hbox );
   QPixmap pixMap = origin->button() ->iconSet() ->pixmap( QIconSet::Small, QIconSet::Normal );
   origin->button() ->setFixedSize( pixMap.width() + 4, pixMap.height() + 4 );
   QWhatsThis::add
      ( origin, i18n( "Use superb KDE file dialog to choose location. " ) );
   origin->setShowLocalProtocol( false );
   origin->lineEdit() ->setURLDropsEnabled( true );
   QWhatsThis::add
      ( origin->lineEdit(), i18n( "Name of directory where you are. You can also "
                                  "enter name of desired location to move there. "
                                  "Use of Net protocols like ftp or fish is possible." ) );
   origin->setMode( KFile::Directory | KFile::ExistingOnly );
   connect( origin, SIGNAL( returnPressed( const QString& ) ), func, SLOT( openUrl( const QString& ) ) );
   connect( origin, SIGNAL( urlSelected( const QString& ) ), func, SLOT( openUrl( const QString& ) ) );

   cdOtherButton = new QToolButton( hbox, "cdOtherButton" );
   cdOtherButton->setFixedSize( 20, origin->button() ->height() );
   cdOtherButton->setText( i18n( "=" ) );
	QToolTip::add(  cdOtherButton, i18n( "Equal" ) );
   connect( cdOtherButton, SIGNAL( clicked() ), this, SLOT( slotFocusAndCDOther() ) );

   cdUpButton = new QToolButton( hbox, "cdUpButton" );
   cdUpButton->setFixedSize( 20, origin->button() ->height() );
   cdUpButton->setText( i18n( ".." ) );
	QToolTip::add(  cdUpButton, i18n( "Up" ) );
   connect( cdUpButton, SIGNAL( clicked() ), this, SLOT( slotFocusAndCDup() ) );

   cdHomeButton = new QToolButton( hbox, "cdHomeButton" );
   cdHomeButton->setFixedSize( 20, origin->button() ->height() );
   cdHomeButton->setText( i18n( "~" ) );
	QToolTip::add(  cdHomeButton, i18n( "Home" ) );
   connect( cdHomeButton, SIGNAL( clicked() ), this, SLOT( slotFocusAndCDHome() ) );

   cdRootButton = new QToolButton( hbox, "cdRootButton" );
   cdRootButton->setFixedSize( 20, origin->button() ->height() );
   cdRootButton->setText( i18n( "/" ) );
	QToolTip::add(  cdRootButton, i18n( "Root" ) );
   connect( cdRootButton, SIGNAL( clicked() ), this, SLOT( slotFocusAndCDRoot() ) );

   // ... creates the button for sync-browsing
   syncBrowseButton = new SyncBrowseButton( hbox );

   setPanelToolbar();

	// create a splitter to hold the view and the popup
	QSplitter *splt = new QSplitter(this);
	splt->setOrientation(QObject::Vertical);
	
   view = new KrDetailedView( splt, this, _left, krConfig );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( executed( QString& ) ), func, SLOT( execute( QString& ) ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( needFocus() ), this, SLOT( slotFocusOnMe() ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( selectionChanged() ), this, SLOT( slotUpdateTotals() ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( itemDescription( QString& ) ), krApp, SLOT( statusBarUpdate( QString& ) ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( contextMenu( const QPoint & ) ), this, SLOT( popRightClickMenu( const QPoint & ) ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( letsDrag( QStringList, QPixmap ) ), this, SLOT( startDragging( QStringList, QPixmap ) ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( gotDrop( QDropEvent * ) ), this, SLOT( handleDropOnView( QDropEvent * ) ) );
   connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( middleButtonClicked( QListViewItem * ) ), SLOTS, SLOT( newTab( QListViewItem * ) ) );
	connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( currentChanged( QListViewItem* ) ), 
		SLOTS, SLOT( updatePopupPanel( QListViewItem* ) ) );
   // make sure that a focus/path change reflects in the command line and activePanel
   connect( this, SIGNAL( cmdLineUpdate( QString ) ), SLOTS, SLOT( slotCurrentChanged( QString ) ) );
   connect( this, SIGNAL( activePanelChanged( ListPanel * ) ), SLOTS, SLOT( slotSetActivePanel( ListPanel * ) ) );

	// add a popup
	popup = new PanelPopup(splt);
#if 0	
	// set initial size to be 1/3 of the panel's height
	QValueList<int> sizes;
	sizes.append(height()/2); sizes.append(height()/2);
	kdWarning()<<sizes[0]<<"--"<<sizes[1]<<"--"<<this->height()<<endl;
	splt->setSizes(sizes); 
#endif	
	connect(popup, SIGNAL(selection(const KURL&)), SLOTS, SLOT(refresh(const KURL&)));
	connect(popup, SIGNAL(hideMe()), this, SLOT(togglePanelPopup()));
	popup->hide();
	
   // finish the layout
   layout->addMultiCellWidget( hbox, 0, 0, 0, 2 );
   layout->addWidget( status, 1, 0 );
   layout->addWidget( historyButton, 1, 1 );
   layout->addWidget( bookmarksButton, 1, 2 );
	layout->addMultiCellWidget( splt, 2, 2, 0, 2 );
   layout->addMultiCellWidget( quickSearch, 3, 3, 0, 2 );
   quickSearch->hide();
   layout->addMultiCellLayout( totalsLayout, 4, 4, 0, 2 );
	
   //filter = ALL;
}

ListPanel::~ListPanel() {
   delete func;
   delete view;
   delete status;
   delete bookmarksButton;
   delete totals;
   delete quickSearch;
   delete origin;
   delete cdRootButton;
   delete cdHomeButton;
   delete cdUpButton;
   delete cdOtherButton;
   delete syncBrowseButton;
   delete layout;
}

void ListPanel::togglePanelPopup() {
	if (popup->isHidden()) {
		popup->show();
		popupBtn->setPixmap(krLoader->loadIcon("down", KIcon::Panel));
	} else {
		popup->hide();
		popupBtn->setPixmap(krLoader->loadIcon("up", KIcon::Panel));
	}
}

void ListPanel::setPanelToolbar() {
   krConfig->setGroup( "Look&Feel" );

   bool panelToolBarVisible = krConfig->readBoolEntry( "Panel Toolbar visible", _PanelToolBar );

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Root Button Visible", _cdRoot ) ) )
      cdRootButton->show();
   else
      cdRootButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Home Button Visible", _cdHome ) ) )
      cdHomeButton->show();
   else
      cdHomeButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Up Button Visible", _cdUp ) ) )
      cdUpButton->show();
   else
      cdUpButton->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "Equal Button Visible", _cdOther ) ) )
      cdOtherButton->show();
   else
      cdOtherButton->hide();

   if ( !panelToolBarVisible || ( krConfig->readBoolEntry( "Open Button Visible", _Open ) ) )
      origin->button() ->show();
   else
      origin->button() ->hide();

   if ( panelToolBarVisible && ( krConfig->readBoolEntry( "SyncBrowse Button Visible", _syncBrowseButton ) ) )
      syncBrowseButton->show();
   else
      syncBrowseButton->hide();
}

void ListPanel::slotUpdateTotals() {
   totals->setText( view->statistics() );
}

void ListPanel::slotFocusAndCDOther() {
   slotFocusOnMe();
   func->openUrl( otherPanel->func->files() ->vfs_getOrigin() );

}

void ListPanel::slotFocusAndCDHome() {
   slotFocusOnMe();
   func->openUrl( QString( "~" ), QString::null );
}

void ListPanel::slotFocusAndCDup() {
   slotFocusOnMe();
   func->dirUp();
}

void ListPanel::slotFocusAndCDRoot() {
   slotFocusOnMe();
   func->openUrl( QString( "/" ), QString::null );
}

void ListPanel::select( bool select, bool all ) {
   if ( all )
      if ( select )
         view->select( QString( "*" ) );
      else
         view->unselect( QString( "*" ) );
   else {
      QString answer = KRSpWidgets::getMask( ( select ? i18n( " Select Files " ) : i18n( " Unselect Files " ) ) );
      // if the user canceled - quit
      if ( answer == QString::null )
         return ;
      if ( select )
         view->select( answer );
      else
         view->unselect( answer );
   }
}

void ListPanel::invertSelection() {
   view->invertSelection();
}

void ListPanel::slotFocusOnMe() {
   // give this VFS the focus (the path bar)
   // we start by calling the KVFS function
   krConfig->setGroup( "Look&Feel" );

   // take care of the 'otherpanel'
   QPalette q( otherPanel->status->palette() );
   q.setColor( QColorGroup::Foreground, KGlobalSettings::textColor() );
   q.setColor( QColorGroup::Background, KGlobalSettings::baseColor() );

   otherPanel->status->setPalette( q );
   otherPanel->totals->setPalette( q );
   otherPanel->view->prepareForPassive();

   // now, take care of this panel
   QPalette p( status->palette() );
   p.setColor( QColorGroup::Foreground, KGlobalSettings::highlightedTextColor() );
   p.setColor( QColorGroup::Background, KGlobalSettings::highlightColor() );
   status->setPalette( p );
   totals->setPalette( p );

   view->prepareForActive();
   emit cmdLineUpdate( realPath );
   emit activePanelChanged( this );

   func->refreshActions();
   KrDetailedView * v = dynamic_cast<KrDetailedView *>( view );
   if ( v )
      v->refreshColors();
   v = dynamic_cast<KrDetailedView *>( otherPanel->view );
   if ( v )
      v->refreshColors();
}

// this is used to start the panel, AFTER setOther() has been used
//////////////////////////////////////////////////////////////////
void ListPanel::start( QString path ) {
   bool left = _left;
   krConfig->setGroup( "Startup" );

   // set the startup path
   if ( path != QString::null ) {
      virtualPath = path;
   } else
      if ( left ) {
         if ( krConfig->readEntry( "Left Panel Origin", _LeftPanelOrigin ) == i18n( "homepage" ) )
            virtualPath = krConfig->readEntry( "Left Panel Homepage", _LeftHomepage );
         else if ( krConfig->readEntry( "Left Panel Origin" ) == i18n( "the last place it was" ) )
				// read the first of the tabbar. lastHomeLeft is obsolete!
				virtualPath = (krConfig->readPathListEntry( "LeftTabBar" ))[0];
         else
            virtualPath = getcwd( 0, 0 ); //get_current_dir_name();
      } else { // right
         if ( krConfig->readEntry( "Right Panel Origin", _RightPanelOrigin ) == i18n( "homepage" ) )
            virtualPath = krConfig->readEntry( "Right Panel Homepage", _RightHomepage );
         else if ( krConfig->readEntry( "Right Panel Origin" ) == i18n( "the last place it was" ) )
				// read the first of the tabbar. lastHomeLeft is obsolete!
				virtualPath = (krConfig->readPathListEntry( "RightTabBar" ))[0];
         else
            virtualPath = getcwd( 0, 0 );
      }

   realPath = virtualPath;
   KURL url = vfs::fromPathOrURL( virtualPath );
   if ( !url.isValid() )
      url = "/";
   func->openUrl( url );
}

void ListPanel::slotStartUpdate() {
   while ( func->inRefresh )
      ; // wait until the last refresh finish
   func->inRefresh = true;  // make sure the next refresh wait for this one
   krApp->setCursor( KCursor::workingCursor() );
   view->clear();

   // set the virtual path
   virtualPath = func->files() ->vfs_getOrigin().prettyURL();
   if ( virtualPath.startsWith( "file:" ) )
      virtualPath = virtualPath.mid( 5 );

   if ( func->files() ->vfs_getType() == vfs::NORMAL )
      realPath = virtualPath;
   this->origin->setURL( virtualPath );
   emit pathChanged( this );
   emit cmdLineUpdate( realPath );	// update the command line

   slotGetStats( virtualPath );
   slotUpdate();
   if ( compareMode ) {
      otherPanel->view->clear();
      ( ( ListPanel* ) otherPanel ) ->slotUpdate();
   }
   // return cursor to normal arrow
   krApp->setCursor( KCursor::arrowCursor() );
   slotUpdateTotals();
}

void ListPanel::slotUpdate() {
   // if we are not at the root add the ".." entery
   QString protocol = func->files() ->vfs_getOrigin().protocol();
   bool isFtp = ( protocol == "ftp" || protocol == "smb" || protocol == "sftp" || protocol == "fish" );

   QString origin = func->files() ->vfs_getOrigin().prettyURL( -1 );
   if ( origin.right( 1 ) != "/" && !( ( func->files() ->vfs_getType() == vfs::FTP ) && isFtp &&
                                       origin.find( '/', origin.find( ":/" ) + 3 ) == -1 ) ) {
      view->addItems( func->files() );
   } else
      view->addItems( func->files(), false );

   func->inRefresh = false;
}


void ListPanel::slotGetStats( QString path ) {
   if ( path.contains( ":/" ) ) {
      status->setText( i18n( "No space information on non-local filesystems" ) );
      return ;
   }

	// check for special filesystems;
	if ( path.left(4) == "/dev") {
		status->setText(i18n( "No space information on [dev]" ));
		return;
	}
#if defined(BSD)
	if ( path.left( 5 ) == "/procfs" ) { // /procfs is a special case - no volume information
		status->setText(i18n( "No space information on [procfs]" ));
		return;
	}
#else
	if ( path.left( 5 ) == "/proc" ) { // /proc is a special case - no volume information
		status->setText(i18n( "No space information on [proc]" ));
		return;
	}
#endif

   status->setText( i18n( "Mt.Man: working ..." ) );
	statsAgent = KDiskFreeSp::findUsageInfo( path );
   connect( statsAgent, SIGNAL( foundMountPoint( const QString &, unsigned long, unsigned long, unsigned long ) ),
            this, SLOT( gotStats( const QString &, unsigned long, unsigned long, unsigned long ) ) );
}

void ListPanel::gotStats( const QString &mountPoint, unsigned long kBSize,
                          unsigned long,  unsigned long kBAvail ) {
	int perc = 0;
	if (kBSize != 0) { // make sure that if totalsize==0, then perc=0
		perc = (int)(((float)kBAvail / (float)kBSize)*100.0);
	}
	// mount point information - find it in the list first
	KMountPoint::List lst = KMountPoint::currentMountPoints();
	QString fstype = i18n("unknown");
   for (KMountPoint::List::iterator it = lst.begin(); it != lst.end(); ++it) {
		if ((*it)->mountPoint() == mountPoint) {
			fstype = (*it)->mountType();
			break;
		}
	}
	
	QString stats = i18n( "%1 free out of %2 (%3%) on %4 [ (%5) ]" )
        .arg( KIO::convertSizeFromKB( kBAvail ) )
        .arg( KIO::convertSizeFromKB( kBSize ) ).arg( perc )
        .arg( mountPoint ).arg( fstype );
	status->setText( stats );
}

void ListPanel::handleDropOnTotals( QDropEvent *e ) {
  handleDropOnView( e, totals );
}

void ListPanel::handleDropOnStatus( QDropEvent *e ) {
  handleDropOnView( e, status );
}

void ListPanel::handleDropOnView( QDropEvent *e, QWidget *widget ) {
   // if copyToPanel is true, then we call a simple vfs_addfiles
   bool copyToDirInPanel = false;
   bool dragFromOtherPanel = false;
   bool dragFromThisPanel = false;
   bool isWritable = func->files() ->vfs_isWritable();

   vfs* tempFiles = func->files();
   vfile *file;
   KrViewItem *i = 0;
   if( widget == 0 )
   {
      i = view->getKrViewItemAt( e->pos() );
      widget = this;
   }

   if ( e->source() == otherPanel )
      dragFromOtherPanel = true;
   if ( e->source() == this )
      dragFromThisPanel = true;

   if ( i ) {
      file = func->files() ->vfs_search( i->name() );

      if ( !file ) { // trying to drop on the ".."
         if ( virtualPath.right( 1 ) == "\\" )        // root of archive..
            isWritable = false;
         else
            copyToDirInPanel = true;
      } else {
         if ( file->vfile_isDir() ) {
            copyToDirInPanel = true;
            isWritable = file->vfile_isWriteable();
            if ( isWritable ) {
               // keep the folder_open icon until we're finished, do it only
               // if the folder is writeable, to avoid flicker
            }
         } else
            if ( e->source() == this )
               return ; // no dragging onto ourselves
      }
   } else    // if dragged from this panel onto an empty spot in the panel...
      if ( dragFromThisPanel ) {  // leave!
         e->ignore();
         return ;
      }

   if ( !isWritable && getuid() != 0 ) {
      e->ignore();
      KMessageBox::sorry( 0, i18n( "Can't drop here, no write permissions." ) );
      return ;
   }
   //////////////////////////////////////////////////////////////////////////////
   // decode the data
   KURL::List URLs;
   if ( !KURLDrag::decode( e, URLs ) ) {
      e->ignore(); // not for us to handle!
      return ;
   }

   KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy;

   // the KURL::List is finished, let's go
   // --> display the COPY/MOVE/LINK menu
   QPopupMenu popup( this );
   popup.insertItem( i18n( "Copy Here" ), 1 );
   if ( func->files() ->vfs_isWritable() )
      popup.insertItem( i18n( "Move Here" ), 2 );
   if ( func->files() ->vfs_getType() == vfs::NORMAL &&
         otherPanel->func->files() ->vfs_getType() == vfs::NORMAL )
      popup.insertItem( i18n( "Link Here" ), 3 );
   popup.insertItem( i18n( "Cancel" ), 4 );
   QPoint tmp = widget->mapToGlobal( e->pos() );
   int result = popup.exec( tmp );
   switch ( result ) {
         case 1 :
         mode = KIO::CopyJob::Copy;
         break;
         case 2 :
         mode = KIO::CopyJob::Move;
         break;
         case 3 :
         mode = KIO::CopyJob::Link;
         break;
         case - 1 :         // user pressed outside the menu
         case 4:
         return ; // cancel was pressed;
   }

   QString dir = "";
   if ( copyToDirInPanel ) {
      dir = i->name();
   }
   QWidget *notify = ( !e->source() ? 0 : e->source() );
   tempFiles->vfs_addFiles( &URLs, mode, notify, dir );
}

void ListPanel::startDragging( QStringList names, QPixmap px ) {
   KURL::List * urls = func->files() ->vfs_getFiles( &names );

   if ( urls->isEmpty() ) { // avoid draging empty urls
      delete urls;
      return ;
   }

	KURLDrag *d = new KURLDrag(*urls, this);
   d->setPixmap( px, QPoint( -7, 0 ) );
   d->dragCopy();

   delete urls; // free memory
}

// pops a right-click menu for items
void ListPanel::popRightClickMenu( const QPoint &loc ) {
   // these are the values that will always exist in the menu
#define OPEN_TAB_ID   89
#define OPEN_ID       90
#define OPEN_WITH_ID  91
#define OPEN_KONQ_ID  92
#define OPEN_TERM_ID  93
#define CHOOSE_ID     94
#define DELETE_ID     95
#define COPY_ID       96
#define MOVE_ID       97
#define RENAME_ID     98
#define PROPERTIES_ID 99
#define MOUNT_ID      100
#define UNMOUNT_ID    101
#define SHRED_ID      102
#define NEW_LINK      103
#define NEW_SYMLINK   104
#define REDIRECT_LINK 105
#define SEND_BY_EMAIL 106
#define LINK_HANDLING 107
#define EJECT_ID      108
#define PREVIEW_ID    109
   // those will sometimes appear
#define SERVICE_LIST_ID  200
   //////////////////////////////////////////////////////////
   bool multipleSelections = false;
   // a quick hack to check if we've got more that one file selected
   KrViewItemList items;
   view->getSelectedKrViewItems( &items );
   if ( items.empty() )
      return ;
   if ( items.size() > 1 )
      multipleSelections = true;
   KrViewItem *item = items.first();
   // create the menu
   KPopupMenu popup, openWith, linkPopup;
   // the OPEN option - open preferd service
   popup.insertItem( "Open/Run", OPEN_ID );      // create the open option
   if ( !multipleSelections ) { // meaningful only if one file is selected
      popup.changeItem( OPEN_ID, item->icon(),        // and add pixmap
                        i18n( ( item->isExecutable() ) && ( !item->isDir() ) ? "Run" : "Open" ) );
      // open in a new tab (if folder)
      if ( item->isDir() ) {
         popup.insertItem( i18n( "Open in a new tab" ), OPEN_TAB_ID );
         popup.changeItem( OPEN_TAB_ID, krLoader->loadIcon( "favorites", KIcon::Panel ), i18n( "Open in a new tab" ) );
      }
      popup.insertSeparator();
   }
   // Preview - normal vfs only ?
   KrPreviewPopup preview;
   if ( func->files() ->vfs_getType() == vfs::NORMAL ) {
      // create the preview popup
      QStringList names;
      getSelectedNames( &names );
      preview.setUrls( func->files() ->vfs_getFiles( &names ) );
      popup.insertItem( QPixmap(), &preview, PREVIEW_ID );
      popup.changeItem( PREVIEW_ID, i18n( "Preview" ) );
   }
   // Open with
   // try to find-out which apps can open the file
   OfferList offers;
   // this too, is meaningful only if one file is selected or if all the files
   // have the same mimetype !
   QString mime = item->mime();
   // check if all the list have the same mimetype
   for ( unsigned int i = 1; i < items.size(); ++i ) {
      if ( ( *items.at( i ) ) ->mime() != mime ) {
         mime = QString::null;
         break;
      }
   }
   if ( !mime.isEmpty() ) {
      offers = KServiceTypeProfile::offers( mime );
      for ( unsigned int i = 0; i < offers.count(); ++i ) {
         KService::Ptr service = offers[ i ].service();
         if ( service->isValid() && service->type() == "Application" ) {
            openWith.insertItem( service->name(), SERVICE_LIST_ID + i );
            openWith.changeItem( SERVICE_LIST_ID + i, service->pixmap( KIcon::Small ), service->name() );
         }
      }
      openWith.insertSeparator();
      if ( item->isDir() )
         openWith.insertItem( krLoader->loadIcon( "konsole", KIcon::Small ), i18n( "Terminal" ), OPEN_TERM_ID );
      openWith.insertItem( i18n( "Other..." ), CHOOSE_ID );
      popup.insertItem( QPixmap(), &openWith, OPEN_WITH_ID );
      popup.changeItem( OPEN_WITH_ID, i18n( "Open with" ) );
      popup.insertSeparator();
   }
   // COPY
   popup.insertItem( i18n( "Copy" ), COPY_ID );
   if ( func->files() ->vfs_isWritable() ) {
      // MOVE
      popup.insertItem( i18n( "Move" ), MOVE_ID );
      // RENAME - only one file
      if ( !multipleSelections )
         popup.insertItem( i18n( "Rename" ), RENAME_ID );
      // DELETE
      popup.insertItem( i18n( "Delete" ), DELETE_ID );
      // SHRED - only one file
      if ( func->files() ->vfs_getType() == vfs::NORMAL &&
            !item->isDir() && !multipleSelections )
         popup.insertItem( i18n( "Shred" ), SHRED_ID );
   }
   // create new shortcut or redirect links - only on local directories:
   if ( func->files() ->vfs_getType() == vfs::NORMAL ) {
      popup.insertSeparator();
      linkPopup.insertItem( i18n( "new symlink" ), NEW_SYMLINK );
      linkPopup.insertItem( i18n( "new hardlink" ), NEW_LINK );
      if ( item->isSymLink() )
         linkPopup.insertItem( i18n( "redirect link" ), REDIRECT_LINK );
      popup.insertItem( QPixmap(), &linkPopup, LINK_HANDLING );
      popup.changeItem( LINK_HANDLING, i18n( "Link handling" ) );
   }
   popup.insertSeparator();
   if ( func->files() ->vfs_getType() == vfs::NORMAL && ( item->isDir() || multipleSelections ) )
      krCalculate->plug( &popup );
   if ( func->files() ->vfs_getType() == vfs::NORMAL && item->isDir() && !multipleSelections ) {
      if ( krMtMan.getStatus( func->files() ->vfs_getFile( item->name() ).path( -1 ) ) == MountMan::KMountMan::MOUNTED )
         popup.insertItem( i18n( "Unmount" ), UNMOUNT_ID );
      else if ( krMtMan.getStatus( func->files() ->vfs_getFile( item->name() ).path( -1 ) ) == MountMan::KMountMan::NOT_MOUNTED )
         popup.insertItem( i18n( "Mount" ), MOUNT_ID );
      if ( krMtMan.ejectable( func->files() ->vfs_getFile( item->name() ).path( -1 ) ) )
         popup.insertItem( i18n( "Eject" ), EJECT_ID );
   }
   // send by mail (only for KDE version >= 2.2.0)
   if ( Krusader::supportedTools().contains( "MAIL" ) && !item->isDir() ) {
      popup.insertItem( i18n( "Send by email" ), SEND_BY_EMAIL );
   }
   // PROPERTIES
   popup.insertSeparator();
   krProperties->plug( &popup );
   // run it, on the mouse location
   int j = QFontMetrics( popup.font() ).height() * 2;
   int result = popup.exec( QPoint( loc.x() + 5, loc.y() + j ) );
   // check out the user's option
   KURL u;
   KURL::List lst;

   switch ( result ) {
         case - 1 :
         return ;     // the user clicked outside of the menu
         case OPEN_TAB_ID :               // Open/Run
         // assuming only 1 file is selected (otherwise we won't get here)
         ( krApp->mainView->activePanel == krApp->mainView->left ? krApp->mainView->leftMng :
           krApp->mainView->rightMng ) ->slotNewTab( func->files() ->vfs_getFile( item->name() ).url() );
         break;
         case OPEN_ID :               // Open in a new tab
         for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
            u = func->files() ->vfs_getFile( ( *it ) ->name() );
            KRun::runURL( u, item->mime() );
         }
         break;
         case COPY_ID :
         func->copyFiles();
         break;
         case MOVE_ID :
         func->moveFiles();
         break;
         case RENAME_ID :
         SLOTS->rename();
         break;
         case DELETE_ID :
         func->deleteFiles();
         break;
         case EJECT_ID :
         MountMan::KMountMan::eject( func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         break;
         case SHRED_ID :
         if ( KMessageBox::warningContinueCancel( krApp,
               i18n( "Are you sure you want to shred " ) + "\"" + item->name() + "\"" +
               i18n(" ? Once shred, the file is gone forever !!!"),
               QString::null, KStdGuiItem::cont(), "Shred" ) == KMessageBox::Continue )
            KShred::shred( func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         break;
         case OPEN_KONQ_ID :          // open in konqueror
         kapp->startServiceByDesktopName( "konqueror", func->files() ->vfs_getFile( item->name() ).url() );
         break;
         case CHOOSE_ID :             // Other...
         u = func->files() ->vfs_getFile( item->name() );
         lst.append( u );
         KRun::displayOpenWithDialog( lst );
         break;
         case MOUNT_ID :
         krMtMan.mount( func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         break;
         case NEW_LINK :
         func->krlink( false );
         break;
         case NEW_SYMLINK :
         func->krlink( true );
         break;
         case REDIRECT_LINK :
         func->redirectLink();
         break;
         case UNMOUNT_ID :
         krMtMan.unmount( func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         break;
         case SEND_BY_EMAIL :
         SLOTS->sendFileByEmail( func->files() ->vfs_getFile( item->name() ).url() );
         break;
         case OPEN_TERM_ID :          // open in terminal
         QString save = getcwd( 0, 0 );
         chdir( func->files() ->vfs_getFile( item->name() ).path( -1 ).local8Bit() );
         KProcess proc;
         krConfig->setGroup( "General" );
         QString term = krConfig->readEntry( "Terminal", _Terminal );
         proc << KrServices::separateArgs( term );
         if ( !item->isDir() )
            proc << "-e" << item->name();

         if ( term.contains( "konsole" ) )     /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
         {                                  /* Please remove the patch if the bug is corrected */
            proc << "&";
            proc.setUseShell( true );
         }

         if ( !proc.start( KProcess::DontCare ) )
            KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + term + "\"" );
         chdir( save.local8Bit() );
         break;
   }
   if ( result >= SERVICE_LIST_ID ) {
      QStringList names;
      getSelectedNames( &names );
      KRun::run( *( offers[ result - SERVICE_LIST_ID ].service() ),
                 *( func->files() ->vfs_getFiles( &names ) ) );
   }
}

void ListPanel::setFilter( KrView::FilterSpec f ) {
   switch ( f ) {
         case KrView::All :
         //case KrView::EXEC:
         break;
         case KrView::Custom :
         filterMask = KRSpWidgets::getMask( i18n( " Select Files " ) );
         // if the user canceled - quit
         if ( filterMask == QString::null )
            return ;
         view->setFilterMask( filterMask );
         break;
         default:
         return ;
   }
   view->setFilter( f ); // do that in any case
   func->refresh();
}

QString ListPanel::getCurrentName() {
   QString name = view->getCurrentItem();
   if ( name != ".." )
      return name;
   else
      return QString::null;
}

void ListPanel::prepareToDelete() {
   view->setNameToMakeCurrent( view->firstUnmarkedBelowCurrent() );
}

void ListPanel::keyPressEvent( QKeyEvent *e ) {
   switch ( e->key() ) {
         case Key_Enter :
         case Key_Return :
         if ( e->state() & ControlButton ) {
            SLOTS->insertFileName( ( e->state() & ShiftButton ) != 0 );
            krApp->mainView->cmdLine->setFocus();
         } else
            e->ignore();
         break;
         case Key_Right :
         case Key_Left :
         if ( e->state() == ControlButton ) {
            // user pressed CTRL+Right/Left - refresh other panel to the selected path if it's a
            // directory otherwise as this one
            if ( ( _left && e->key() == Key_Right ) || ( !_left && e->key() == Key_Left ) ) {
               KURL newPath;
               if ( view->getCurrentKrViewItem() ->isDir() ) {
                  newPath = func->files() ->vfs_getFile( view->getCurrentKrViewItem() ->name() );
               } else {
                  newPath = func->files() ->vfs_getOrigin();
               }
               otherPanel->func->openUrl( newPath );
            } else
               func->openUrl( otherPanel->func->files() ->vfs_getOrigin() );

            slotFocusOnMe(); // return focus to us!
            return ;
            //} else if (e->state() == ShiftButton) {
            //	krApp->mainView->leftMng->changePanel(e->key() == Key_Left);
         } else
            e->ignore();
         break;

         case Key_Down :
         if ( e->state() == ControlButton ) { // give the keyboard focus to the command line
            if ( krApp->mainView->cmdLine->isVisible() )
               krApp->mainView->cmdLineFocus();
            else if ( krApp->mainView->terminal_dock->isVisible() )
               krApp->mainView->terminal_dock->setFocus();
            return ;
         } else
            e->ignore();
         break;

         default:
         // if we got this, it means that the view is not doing
         // the quick search thing, so send the characters to the commandline, if normal key
         if ( e->state() == NoButton )
            krApp->mainView->cmdLine->addText( e->text() );

         //e->ignore();
   }
}

void ListPanel::slotItemAdded(vfile *vf) {
	view->addItem(vf);
}

void ListPanel::slotItemDeleted(const QString& name) {
	view->delItem(name);
}

void ListPanel::slotItemUpdated(vfile *vf) {
	view->updateItem(vf);
}
