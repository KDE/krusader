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
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qtimer.h>
#include <qregexp.h> 
// KDE includes
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
#include <kdeversion.h>
#include <qimage.h>
#include <qtabbar.h>
#include <kdebug.h>
#include <kurlrequester.h> 
// Krusader includes
#include "../krusader.h"
#include "../krslots.h"
#include "../kicons.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/krpermhandler.h"
#include "listpanel.h"
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

typedef QValueList<KServiceOffer> OfferList;

/////////////////////////////////////////////////////
// 					The list panel constructor             //
/////////////////////////////////////////////////////
ListPanel::ListPanel( QWidget *parent, bool left, const char *name ) :
QWidget( parent, name ), colorMask( 255 ), compareMode( false ), currDragItem( 0 ), statsAgent( 0 ), _left( left ) {

  func = new ListPanelFunc( this );
  setAcceptDrops( true );
  layout = new QGridLayout( this, 3, 2 );

  status = new KrSqueezedTextLabel( this );
  krConfig->setGroup( "Look&Feel" );
  status->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
  status->setBackgroundMode( PaletteBackground );
  status->setFrameStyle( QFrame::Box | QFrame::Raised );
  status->setLineWidth( 1 );		// a nice 3D touch :-)
  status->setText( "" );        // needed for initialization code!
  int sheight = QFontMetrics( status->font() ).height() + 4;
  status->setMaximumHeight( sheight );
  QWhatsThis::add( status, i18n( "The status bar displays information about the FILESYSTEM which hold your current directory: Total size, free space, type of filesystem etc." ) );
  connect( status, SIGNAL( clicked() ), this, SLOT( slotFocusOnMe() ) );

  // ... create the bookmark list
  bookmarksButton = new BookmarksButton( this );
  connect( bookmarksButton, SIGNAL( pressed() ), this, SLOT( slotFocusOnMe() ) );
  connect( bookmarksButton, SIGNAL( openUrl( const QString& ) ), func, SLOT( openUrl( const QString& ) ) );
  QWhatsThis::add( bookmarksButton, i18n( "Open menu with bookmarks. You can also add "
			  							  "current location to the list, edit bookmarks "
										  "or add subfolder to the list." ) );

  totals = new KrSqueezedTextLabel( this );
  krConfig->setGroup( "Look&Feel" );
  totals->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
  totals->setFrameStyle( QFrame::Box | QFrame::Raised );
  totals->setBackgroundMode( PaletteBackground );
  totals->setLineWidth( 1 );		// a nice 3D touch :-)
  totals->setMaximumHeight( sheight );
  QWhatsThis::add( totals, i18n( "The totals bar shows how much files exist, "
			  					 "how many did you select and the bytes math" ) );
  connect( totals, SIGNAL( clicked() ), this, SLOT( slotFocusOnMe() ) );

  quickSearch = new KrQuickSearch( this );
  krConfig->setGroup( "Look&Feel" );
  quickSearch->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
  quickSearch->setFrameStyle( QFrame::Box | QFrame::Raised );
  quickSearch->setLineWidth( 1 );		// a nice 3D touch :-)
  quickSearch->setMaximumHeight( sheight );

  origin = new KURLRequester( this );
  QWhatsThis::add( origin, i18n( "Use superb KDE file dialog to choose location. " ) );
  origin->setShowLocalProtocol( false );
  origin->lineEdit() ->setURLDropsEnabled( true );
  QWhatsThis::add( origin->lineEdit(), i18n( "Name of directory where you are. You can also "
			  					 "enter name of desired location to move there. "
								 "Use of Net protocols like ftp or fish is possible." ) );
  origin->setMode( KFile::Directory | KFile::ExistingOnly );
  connect( origin, SIGNAL( returnPressed( const QString& ) ), func, SLOT( openUrl( const QString& ) ) );
  connect( origin, SIGNAL( urlSelected( const QString& ) ), func, SLOT( openUrl( const QString& ) ) );

  view = new KrDetailedView( this, _left, krConfig );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( executed( QString& ) ), func, SLOT( execute( QString& ) ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( needFocus() ), this, SLOT( slotFocusOnMe() ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( selectionChanged() ), this, SLOT( slotUpdateTotals() ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( itemDescription( QString& ) ), krApp, SLOT( statusBarUpdate( QString& ) ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( contextMenu( const QPoint & ) ), this, SLOT( popRightClickMenu( const QPoint & ) ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( letsDrag( QStringList, QPixmap ) ), this, SLOT( startDragging( QStringList, QPixmap ) ) );
  connect( dynamic_cast<KrDetailedView*>( view ), SIGNAL( gotDrop( QDropEvent * ) ), this, SLOT( handleDropOnView( QDropEvent * ) ) );
  ////////////////////////////// to do connections ///////////////////////////////////////////////

  // make sure that a focus/path change reflects in the command line and activePanel
  connect( this, SIGNAL( cmdLineUpdate( QString ) ), SLOTS, SLOT( slotCurrentChanged( QString ) ) );
  connect( this, SIGNAL( activePanelChanged( ListPanel * ) ), SLOTS, SLOT( slotSetActivePanel( ListPanel * ) ) );

  // finish the layout
  layout->addMultiCellWidget( origin, 0, 0, 0, 1 );
  layout->addWidget( status, 1, 0 );
  layout->addWidget( bookmarksButton, 1, 1 );
  layout->addMultiCellWidget( dynamic_cast<KrDetailedView*>( view ) ->widget(), 2, 2, 0, 1 );
  layout->addMultiCellWidget( quickSearch, 3, 3, 0, 1 ); quickSearch->hide();
  layout->addMultiCellWidget( totals, 4, 4, 0, 1 );

  filter = ALL;
}

void ListPanel::slotUpdateTotals() {
  totals->setText( view->statistics() );
}

void ListPanel::select( bool select, bool all ) {
  if ( all )
    if ( select ) view->select( QString( "*" ) );
    else view->unselect( QString( "*" ) );
  else {
    QString answer = KRSpWidgets::getMask( ( select ? i18n( " Select Files " ) : i18n( " Unselect Files " ) ) );
    // if the user canceled - quit
    if ( answer == QString::null ) return ;
    if ( select ) view->select( answer );
    else view->unselect( answer );
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
}

// this is used to start the panel, AFTER setOther() has been used
//////////////////////////////////////////////////////////////////
void ListPanel::start(QString path) {
  bool left = _left;
  krConfig->setGroup( "Startup" );

  // set the startup path
  if (path != QString::null)  {
    virtualPath = path;
  } else
  if ( left ) {
    if ( krConfig->readEntry( "Left Panel Origin", _LeftPanelOrigin ) == i18n( "homepage" ) )
      virtualPath = krConfig->readEntry( "Left Panel Homepage", _LeftHomepage );
    else if ( krConfig->readEntry( "Left Panel Origin" ) == i18n( "the last place it was" ) )
      virtualPath = krConfig->readEntry( "lastHomeLeft", "/" );
    else virtualPath = getcwd( 0, 0 ); //get_current_dir_name();
  } else { // right
    if ( krConfig->readEntry( "Right Panel Origin", _RightPanelOrigin ) == i18n( "homepage" ) )
      virtualPath = krConfig->readEntry( "Right Panel Homepage", _RightHomepage );
    else if ( krConfig->readEntry( "Right Panel Origin" ) == i18n( "the last place it was" ) )
      virtualPath = krConfig->readEntry( "lastHomeRight", "/" );
    else virtualPath = getcwd( 0, 0 );
  }

  realPath = virtualPath;
  func->openUrl( virtualPath );
}

void ListPanel::slotStartUpdate() {
  // if the vfs couldn't make it  - go back
  if ( func->files() ->vfs_error() ) {
    func->inRefresh = false;
    func->dirUp();
    return ;
  }

  while ( func->inRefresh ) ; // wait until the last refresh finish
  func->inRefresh = true;  // make sure the next refresh wait for this one
  krApp->setCursor( KCursor::workingCursor() );
  view->clear();

  // set the virtual path
  virtualPath = func->files() ->vfs_getOrigin();
  if ( func->files() ->vfs_getType() == "normal" )
    realPath = virtualPath;
  this->origin->setURL( virtualPath );
  emit pathChanged(this);
  emit cmdLineUpdate( realPath );	// update the command line
}

void ListPanel::slotEndUpdate() {
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
  QString origin = func->files() ->vfs_getOrigin();
  if ( origin.right( 1 ) != "/" && !( ( func->files() ->vfs_getType() == "ftp" ) &&
                                      origin.find( '/', origin.find( ":/" ) + 3 ) == -1 ) ) {
    view->addItems( func->files() );
  } else view->addItems( func->files(), false );

  func->inRefresh = false;
}


void ListPanel::slotGetStats( QString path ) {
  QString templabel, label;

  if ( path.contains( "\\" ) ) {
    status->setText( i18n( "No space information inside archives" ) );
    return ;
  }

  if ( path.contains( ":/" ) ) {
    status->setText( i18n( "No space information on non-local filesystems" ) );
    return ;
  }

  // 1st, get information
  // mountMan tries to get information on the mountpoint regardless if
  // it is operational or not - rafi's fix
  status->setText( i18n( "Mt.Man: working ..." ) );
  statsAgent = new MountMan::statsCollector( path, this );
}

void ListPanel::gotStats( QString data ) {
  status->setText( data );
  if ( statsAgent ) delete statsAgent;
}

void ListPanel::handleDropOnView( QDropEvent *e ) {
  // if copyToPanel is true, then we call a simple vfs_addfiles
  bool copyToDirInPanel = false;
  bool dragFromOtherPanel = false;
  bool dragFromThisPanel = false;
  bool isWritable = func->files() ->vfs_isWritable();

  vfs* tempFiles = func->files();
  vfile *file;
  KrViewItem *i = view->getKrViewItemAt( e->pos() );

  if ( e->source() == otherPanel ) dragFromOtherPanel = true;
  if ( e->source() == this ) dragFromThisPanel = true;

  if ( i ) {
    file = func->files() ->vfs_search( i->name() );

    if ( !file ) { // trying to drop on the ".."
      if ( virtualPath.right( 1 ) == "\\" )    // root of archive..
        isWritable = false;
      else copyToDirInPanel = true;
    } else {
      if ( file->vfile_isDir() ) {
        copyToDirInPanel = true;
        isWritable = file->vfile_isWriteable();
        if ( isWritable ) {
          // keep the folder_open icon until we're finished, do it only
          // if the folder is writeable, to avoid flicker
        }
      } else
        if ( e->source() == this ) return ; // no dragging onto ourselves
    }
  } else    // if dragged from this panel onto an empty spot in the panel...
    if ( dragFromThisPanel ) {  // leave!
      e->ignore();
      return ;
    }

  if ( !isWritable ) {
    e->ignore();
    KMessageBox::sorry( 0, i18n( "Can't drop here, no write permissions." ) );
    return ;
  }
  //////////////////////////////////////////////////////////////////////////////
  // decode the data
  KURL::List URLs;
  QStrList list;
  if ( !QUriDrag::decode( e, list ) ) {
    e->ignore(); // not for us to handle!
    return ;
  } // now, the list of URLs is stored in 'list', we'll create a KURL::List
  QStrListIterator it( list );
  while ( it ) {
    URLs.append( *it );
    ++it;
  }

  KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy;

  // the KURL::List is finished, let's go
  // --> display the COPY/MOVE/LINK menu
  QPopupMenu popup( this );
  popup.insertItem( i18n( "Copy Here" ), 1 );
  if ( func->files() ->vfs_isWritable() ) popup.insertItem( i18n( "Move Here" ), 2 );
  if ( func->files() ->vfs_getType() == "normal" &&
       otherPanel->func->files() ->vfs_getType() == "normal" )
    popup.insertItem( i18n( "Link Here" ), 3 );
  popup.insertItem( i18n( "Cancel" ), 4 );
  QPoint tmp = mapToGlobal( e->pos() );
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
      case - 1 :     // user pressed outside the menu
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
    return ;
  }
  // KURL::List -> QStrList
  QStrList URIs;
  for ( KURL::List::Iterator url = urls->begin(); url != urls->end() ; ++url )
    URIs.append( ( *url ).url().local8Bit() );
  delete urls; // free memory

  QUriDrag *d = new QUriDrag( URIs, this );
  d->setPixmap( px, QPoint( -7, 0 ) );
  d->dragCopy();
}

// pops a right-click menu for items
void ListPanel::popRightClickMenu( const QPoint &loc ) {
  // these are the values that will always exist in the menu
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
  if ( items.empty() ) return ;
  if ( items.size() > 1 ) multipleSelections = true;
  KrViewItem *item = items.first();
  // create the menu
  QPopupMenu popup, openWith, linkPopup;
  // the OPEN option - open preferd service
  popup.insertItem( "Open/Run", OPEN_ID );      // create the open option
  if ( !multipleSelections ) { // meaningful only if one file is selected
    popup.changeItem( OPEN_ID, item->icon(),    // and add pixmap
                      i18n( ( item->isExecutable() ) && ( !item->isDir() ) ? "Run" : "Open" ) );
    popup.insertSeparator();
  }
  // Preview - normal vfs only ?
  KrPreviewPopup preview;
  if ( func->files() ->vfs_getType() == "normal" ) {
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
  for(unsigned int i=1; i<items.size(); ++i){
    if( (*items.at(i))->mime() != mime ){
      mime = QString::null;
      break;
    }
  }
  if ( !mime.isEmpty() ) {
    offers = KServiceTypeProfile::offers( mime );
    for ( unsigned int i = 0; i < offers.count(); ++i ) {
      KService::Ptr service = offers[ i ].service();
      if ( service->isValid() && service->type()=="Application" ) {
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
    if ( func->files() ->vfs_getType() == "normal" &&
         !item->isDir() && !multipleSelections )
      popup.insertItem( i18n( "Shred" ), SHRED_ID );
  }
  // create new shortcut or redirect links - not on ftp:
  if ( func->files() ->vfs_getType() != "ftp" ) {
    popup.insertSeparator();
    linkPopup.insertItem( i18n( "new symlink" ), NEW_SYMLINK );
    linkPopup.insertItem( i18n( "new hardlink" ), NEW_LINK );
    if ( item->isSymLink() )
      linkPopup.insertItem( i18n( "redirect link" ), REDIRECT_LINK );
    popup.insertItem( QPixmap(), &linkPopup, LINK_HANDLING );
    popup.changeItem( LINK_HANDLING, i18n( "Link handling" ) );
  }
  popup.insertSeparator();
  if ( func->files() ->vfs_getType() != "ftp" && ( item->isDir() || multipleSelections ) )
    krCalculate->plug( &popup );
  if ( func->files() ->vfs_getType() == "normal" && item->isDir() && !multipleSelections ) {
    if ( krMtMan.getStatus( func->files() ->vfs_getFile( item->name() ) ) == MountMan::KMountMan::MOUNTED )
      popup.insertItem( i18n( "Unmount" ), UNMOUNT_ID );
    else if ( krMtMan.getStatus( func->files() ->vfs_getFile( item->name() ) ) == MountMan::KMountMan::NOT_MOUNTED )
      popup.insertItem( i18n( "Mount" ), MOUNT_ID );
    if ( krMtMan.ejectable( func->files() ->vfs_getFile( item->name() ) ) )
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
      case - 1 : return ;     // the user clicked outside of the menu
      case OPEN_ID :           // Open/Run
      for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
        u.setPath( func->files() ->vfs_getFile( ( *it ) ->name() ) );
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
      MountMan::KMountMan::eject( func->files() ->vfs_getFile( item->name() ) );
      break;
      case SHRED_ID :
      if ( KMessageBox::warningContinueCancel( krApp,
                                               i18n( "Are you sure you want to shred " ) + "\"" + item->name() + "\"" +
                                               " ? Once shred, the file is gone forever !!!",
                                               QString::null, KStdGuiItem::cont(), "Shred" ) == KMessageBox::Continue )
        KShred::shred( func->files() ->vfs_getFile( item->name() ) );
      break;
      case OPEN_KONQ_ID :      // open in konqueror
      kapp->startServiceByDesktopName( "konqueror", func->files() ->vfs_getFile( item->name() ) );
      break;
      case CHOOSE_ID :         // Other...
      u.setPath( func->files() ->vfs_getFile( item->name() ) );
      lst.append( u );
      KRun::displayOpenWithDialog( lst );
      break;
      case MOUNT_ID :
      krMtMan.mount( func->files() ->vfs_getFile( item->name() ) );
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
      krMtMan.unmount( func->files() ->vfs_getFile( item->name() ) );
      break;
      case SEND_BY_EMAIL :
      SLOTS->sendFileByEmail( func->files() ->vfs_getFile( item->name() ) );
      break;
      case OPEN_TERM_ID :      // open in terminal
      QString save = getcwd( 0, 0 );
      chdir( func->files() ->vfs_getFile( item->name() ).local8Bit() );
      KProcess proc;
      krConfig->setGroup( "General" );
      QString term = krConfig->readEntry( "Terminal", _Terminal );
      proc << term;
      if ( !item->isDir() ) proc << "-e" << item->name();
      if ( !proc.start( KProcess::DontCare ) )
        KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + term + "\"" );
      chdir( save.local8Bit() );
      break;
  }
  if ( result >= SERVICE_LIST_ID ) {
    QStringList names;
    getSelectedNames( &names );
    KRun::run( *(offers[result-SERVICE_LIST_ID].service()),
                *(func->files()->vfs_getFiles(&names)) );
  }
}

void ListPanel::setFilter( FilterSpec f ) {
  switch ( f ) {
      case ALL : filter = ALL;
      break;
      case EXEC: filter = EXEC;
      break;
      case CUSTOM : filterMask = KRSpWidgets::getMask( i18n( " Select Files " ) );
      // if the user canceled - quit
      if ( filterMask == QString::null ) return ;
      filter = CUSTOM;
      break;
      default: return ;
  }
  func->refresh();
}

QString ListPanel::getCurrentName() {
  QString name = view->getCurrentItem();
  if ( name != ".." ) return name;
  else return QString::null;
}

void ListPanel::prepareToDelete() {
  view->setNameToMakeCurrent( view->firstUnmarkedAboveCurrent() );
}

void ListPanel::keyPressEvent( QKeyEvent *e ) {
  switch ( e->key() ) {
      case Key_Right :
      case Key_Left :
      if ( e->state() == ControlButton ) { // user pressed CTRL+Right/Left - refresh other panel to the same path as this one
        otherPanel->func->openUrl( realPath );
        slotFocusOnMe(); // return focus to us!
        return ;
      } else e->ignore();

      case Key_Down :
      if ( e->state() == ControlButton ) { // give the keyboard focus to the command line
        if ( krApp->mainView->cmdLine->isVisible() )
          krApp->mainView->cmdLineFocus();
        else if ( krApp->mainView->terminal_dock->isVisible() )
          krApp->mainView->terminal_dock->setFocus();
        return ;
      } else e->ignore();

      default:
      // if we got this, it means that the view is not doing
      // the quick search thing, so send the characters to the commandline, if normal key
      if (e->state() == NoButton)
         krApp->mainView->cmdLine->addText( e->text() );

      //e->ignore();
  }
}
