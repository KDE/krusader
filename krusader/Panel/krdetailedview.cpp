/***************************************************************************
                         krdetailedview.cpp
                        -------------------
copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#include "krdetailedview.h"
#include "krdetailedviewitem.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../krslots.h"
#include "../VFS/krpermhandler.h"
#include "../GUI/kcmdline.h"
#include "../Dialogs/krspecialwidgets.h"
#include "listpanel.h"
#include "panelfunc.h"
#include <kconfigbase.h>
#include <qlayout.h>
#include <qdir.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <kdebug.h>
#include <kprogress.h>
#include <kstatusbar.h>
#include <klineeditdlg.h>

//////////////////////////////////////////////////////////////////////////
//  The following is KrDetailedView's settings in KConfig:
// Group name: KrDetailedView
//
// Ext Column
#define _ExtColumn          true 
// Mime Column
#define _MimeColumn         false 
// Size Column
#define _SizeColumn         true 
// DateTime Column
#define _DateTimeColumn     true 
// Perm Column
#define _PermColumn         false 
// KrPerm Column
#define _KrPermColumn       true 
// Owner Column
#define _OwnerColumn        false 
// Group Column
#define _GroupColumn        false 
// Do Quicksearch
#define _DoQuicksearch      true 
// Classic Quicksearch
#define _ClassicQuicksearch false
//////////////////////////////////////////////////////////////////////////

QString KrDetailedView::ColumnName[ MAX_COLUMNS ];

KrDetailedView::KrDetailedView( QWidget *parent, bool left, KConfig *cfg, const char *name ) :
      KListView( parent, name ), KrView( cfg ), _focused( false ), _currDragItem( 0L ),
_nameInKConfig( QString( "KrDetailedView" ) + QString( ( left ? "Left" : "Right" ) ) ), _left( left ) {

   if ( ColumnName[ 0 ].isEmpty() ) {
      ColumnName[ 0 ] = i18n( "Name" );
      ColumnName[ 1 ] = i18n( "Ext" );
      ColumnName[ 2 ] = i18n( "Type" );
      ColumnName[ 3 ] = i18n( "Size" );
      ColumnName[ 4 ] = i18n( "Modified" );
      ColumnName[ 5 ] = i18n( "Perms" );
      ColumnName[ 6 ] = i18n( "rwx" );
      ColumnName[ 7 ] = i18n( "Owner" );
      ColumnName[ 8 ] = i18n( "Group" );
   }

   KConfigGroupSaver grpSvr( _config, nameInKConfig() );
   // setup the default sort and filter
   _filter = KrView::All;
   _filterMask = "*";
   _sortMode = static_cast<SortSpec>( KrView::Name | KrView::Descending | KrView::DirsFirst );
   if ( !_config->readBoolEntry( "Case Sensative Sort", _CaseSensativeSort ) )
      _sortMode = static_cast<SortSpec>( _sortMode | KrView::IgnoreCase );

   // first, clear the columns list. it will be updated by newColumn()
   for ( int i = 0; i < MAX_COLUMNS; i++ )
      _columns[ i ] = Unused;

   /////////////////////////////// listview ////////////////////////////////////
   { // use the {} so that KConfigGroupSaver will work correctly!
      KConfigGroupSaver grpSvr( _config, "Look&Feel" );
      krConfig->setGroup( "Look&Feel" );
      setFont( _config->readFontEntry( "Filelist Font", _FilelistFont ) );
      // a change in the selection needs to update totals
      connect( this, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( slotClicked( QListViewItem* ) ) );
      connect( this, SIGNAL( doubleClicked( QListViewItem* ) ), this, SLOT( slotDoubleClicked( QListViewItem* ) ) );
      connect( this, SIGNAL( returnPressed( QListViewItem* ) ), this, SIGNAL( executed( QListViewItem* ) ) );
      connect( this, SIGNAL( onItem( QListViewItem* ) ), this, SLOT( slotItemDescription( QListViewItem* ) ) );
      connect( this, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
               this, SLOT( handleContextMenu( QListViewItem*, const QPoint&, int ) ) );
      connect( this, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( setNameToMakeCurrent( QListViewItem* ) ) );
      connect( this, SIGNAL( mouseButtonClicked ( int, QListViewItem *, const QPoint &, int ) ),
               this, SLOT( slotMouseClicked ( int, QListViewItem *, const QPoint &, int ) ) );
   }

   setWidget( this );

   // add whatever columns are needed to the listview
   _withIcons = _config->readBoolEntry( "With Icons", _WithIcons ); // we we display icons ?
   newColumn( Name );  // we always have a name
   setColumnWidthMode( column( Name ), QListView::Manual );
   if ( _config->readBoolEntry( "Ext Column", _ExtColumn ) ) {
      newColumn( Extention );
      setColumnWidthMode( column( Extention ), QListView::Manual );
      setColumnWidth( column( Extention ), QFontMetrics( font() ).width( "tar.bz2" ) );
   }
   if ( _config->readBoolEntry( "Mime Column", _MimeColumn ) ) {
      newColumn( Mime );
      setColumnWidthMode( column( Mime ), QListView::Manual );
      setColumnWidth( column( Mime ), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( _config->readBoolEntry( "Size Column", _SizeColumn ) ) {
      newColumn( Size );
      setColumnWidthMode( column( Size ), QListView::Manual );
      setColumnWidth( column( Size ), QFontMetrics( font() ).width( "9" ) * 10 );
      setColumnAlignment( column( Size ), Qt::AlignRight ); // right-align numbers
   }
   if ( _config->readBoolEntry( "DateTime Column", _DateTimeColumn ) ) {
      newColumn( DateTime );
      setColumnWidthMode( column( DateTime ), QListView::Manual );
      //setColumnWidth( column( DateTime ), QFontMetrics( font() ).width( "99/99/99  99:99" ) );
      setColumnWidth( column( DateTime ), QFontMetrics( font() ).width( KGlobal::locale() ->formatDateTime(
                         QDateTime ( QDate( 2099, 12, 29 ), QTime( 23, 59 ) ) ) ) + 3 );
   }
   if ( _config->readBoolEntry( "Perm Column", _PermColumn ) ) {
      newColumn( Permissions );
      setColumnWidthMode( column( Permissions ), QListView::Manual );
      setColumnWidth( column( Permissions ), QFontMetrics( font() ).width( "drwxrwxrwx" ) );
   }
   if ( _config->readBoolEntry( "KrPerm Column", _KrPermColumn ) ) {
      newColumn( KrPermissions );
      setColumnWidthMode( column( KrPermissions ), QListView::Manual );
      setColumnWidth( column( KrPermissions ), QFontMetrics( font() ).width( "RWX" ) );
   }
   if ( _config->readBoolEntry( "Owner Column", _OwnerColumn ) ) {
      newColumn( Owner );
      setColumnWidthMode( column( Owner ), QListView::Manual );
      setColumnWidth( column( Owner ), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( _config->readBoolEntry( "Group Column", _GroupColumn ) ) {
      newColumn( Group );
      setColumnWidthMode( column( Group ), QListView::Manual );
      setColumnWidth( column( Group ), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   // determine basic settings for the listview
   setAcceptDrops( true );
   setDragEnabled( true );
   setTooltipColumn( column( Name ) );
   //setDropVisualizer(false);
   //setDropHighlighter(false);
   setSelectionModeExt( KListView::FileManager );
   setAllColumnsShowFocus( true );
   setShowSortIndicator( true );
   header() ->setStretchEnabled( true, column( Name ) );

   //---- don't enable these lines, as it causes an ugly bug with inplace renaming
   //-->  setItemsRenameable( true );
   //-->  setRenameable( column( Name ), true );
   //-------------------------------------------------------------------------------

   // allow in-place renaming
   connect( renameLineEdit(), SIGNAL( done( QListViewItem *, int ) ),
            this, SLOT( inplaceRenameFinished( QListViewItem*, int ) ) );
   connect( this, SIGNAL( renameItem( const QString &, const QString & ) ),
            dynamic_cast<ListPanel*>( parent ) ->func, SLOT( rename( const QString &, const QString & ) ) );
   // connect quicksearch
   connect( dynamic_cast<ListPanel*>( parent ) ->quickSearch, SIGNAL( textChanged( const QString& ) ),
            this, SLOT( quickSearch( const QString& ) ) );
   connect( dynamic_cast<ListPanel*>( parent ) ->quickSearch, SIGNAL( otherMatching( const QString&, int ) ),
            this, SLOT( quickSearch( const QString& , int ) ) );
   connect( dynamic_cast<ListPanel*>( parent ) ->quickSearch, SIGNAL( stop( QKeyEvent* ) ),
            this, SLOT( stopQuickSearch( QKeyEvent* ) ) );


   setFocusPolicy( StrongFocus );
   restoreSettings();
}

KrDetailedView::~KrDetailedView() {
   //saveSettings();
}

void KrDetailedView::newColumn( ColumnType type ) {
   int i;
   for ( i = 0; i < MAX_COLUMNS; i++ )
      if ( _columns[ i ] == Unused )
         break;
   if ( i >= MAX_COLUMNS )
      perror( "KrDetailedView::newColumn() - too many columns" );
   // add the new type to the column handler
   _columns[ i ] = type;
   addColumn( ColumnName[ type ], -1 );
}

/**
 * returns the number of column which holds values of type 'type'.
 * if such values are not presented in the view, -1 is returned.
 */
int KrDetailedView::column( ColumnType type ) {
   for ( int i = 0; i < MAX_COLUMNS; i++ )
      if ( _columns[ i ] == type )
         return i;
   return -1;
}

void KrDetailedView::addItems( vfs *v, bool addUpDir ) {
   QListViewItem * item = firstChild();
   QListViewItem *currentItem = item;
   QString size, name;

   // add the up-dir arrow if needed
   if ( addUpDir ) {
      KListViewItem * item = new KrDetailedViewItem( this, ( QListViewItem* ) 0L, ( vfile* ) 0L );
      item->setText( column( Name ), ".." );
      item->setText( column( Size ), "<DIR>" );
      item->setPixmap( column( Name ), FL_LOADICON( "up" ) );
      item->setSelectable( false );
   }

   // add a progress bar to the totals statusbar
   //	KProgress *pr = new KProgress(krApp->mainView->activePanel->totals);
   //  krApp->mainView->activePanel->totals->addWidget(pr,true);
   //  pr->setTotalSteps(v->vfs_noOfFiles());
   // make sure the listview stops sorting itself - it makes us slower!
   int cnt = 0;
   int cl = columnSorted();
   bool as = ascendingSort();
   setSorting( -1 ); // disable sorting

   for ( vfile * vf = v->vfs_getFirstFile(); vf != 0 ; vf = v->vfs_getNextFile() ) {
      size = KRpermHandler::parseSize( vf->vfile_getSize() );
      name = vf->vfile_getName();
      bool isDir = vf->vfile_isDir();
      /*KRListItem::cmpColor color = KRListItem::none;
      if( otherPanel->type == "list" && compareMode ){
        vfile* ovf = otherPanel->files->vfs_search(vf->vfile_getName());
        if (ovf == 0 ) color = KRListItem::exclusive;  // this file doesn't exist on the other panel
        else{ // if we found such a file
            QString date1 = KRpermHandler::date2qstring(vf->vfile_getDateTime());
            QString date2 = KRpermHandler::date2qstring(ovf->vfile_getDateTime());
            if (date1 > date2) color = KRListItem::newer; // this file is newer than the other
            else
            if (date1 < date2) color = KRListItem::older; // this file is older than the other
            else
            if (date1 == date2) color = KRListItem::identical; // the files are the same
        }
      }*/

      if ( !isDir || ( isDir && ( _filter & ApplyToDirs ) ) ) {
         switch ( _filter ) {
               case KrView::All :
                  break;
               case KrView::Custom :
               if ( !QDir::match( _filterMask, name ) )
                  continue;
               break;
               case KrView::Dirs:
               if ( !vf->vfile_isDir() )
                  continue;
               break;
               case KrView::Files:
               if ( vf->vfile_isDir() )
                  continue;
               break;

               case KrView::ApplyToDirs :
               break; // no-op, stop compiler complaints
         }
         /*if ( compareMode && !(color & colorMask) ) continue;*/
      }

      item = new KrDetailedViewItem( this, item, vf );
      if ( isDir )
         ++_numDirs;
      else
         _countSize += dynamic_cast<KrViewItem*>( item ) ->size();
      ++_count;
      // if the item should be current - make it so
      if ( ( dynamic_cast<KrViewItem*>( item ) ) ->
            name() == nameToMakeCurrent() )
         currentItem = item;

      cnt++;
      //    if (cnt % 300 == 0) {
      //      pr->show();
      //      pr->advance(300);
      //      kapp->processOneEvent();
      //    }
   }

   // kill progressbar
   //  krApp->mainView->activePanel->totals->removeWidget(pr);
   //  delete(pr);

   // re-enable sorting
   setSorting( cl, as );
   sort();

   if ( !currentItem )
      currentItem = firstChild();
   KListView::setCurrentItem( currentItem );
   ensureItemVisible( currentItem );
}

QString KrDetailedView::getCurrentItem() const {
   QListViewItem * it = currentItem();
   if ( !it )
      return QString::null;
   else
      return dynamic_cast<KrViewItem*>( it ) ->name();
}

void KrDetailedView::setCurrentItem( const QString& name ) {
   for ( QListViewItem * it = firstChild(); it != 0; it = it->itemBelow() )
      if ( dynamic_cast<KrViewItem*>( it ) ->
            name() == name ) {
         KListView::setCurrentItem( it );
         break;
      }
}

void KrDetailedView::clear() {
   emit KListView::selectionChanged(); /* to avoid rename crash at refresh */
   KListView::clear();
   _count = _numSelected = _numDirs = _selectedSize = _countSize = 0;
}

void KrDetailedView::setSortMode( SortSpec mode ) {
   _sortMode = mode; // the KrViewItems will check it by themselves
   bool ascending = !( mode & KrView::Descending );
   int cl = -1;
   if ( mode & KrView::Name )
      cl = column( Name );
   else
      if ( mode & KrView::Size )
         cl = column( Extention );
      else
         if ( mode & KrView::Type )
            cl = column( Mime );
         else
            if ( mode & KrView::Modified )
               cl = column( DateTime );
            else
               if ( mode & KrView::Permissions )
                  cl = column( Permissions );
               else
                  if ( mode & KrView::KrPermissions )
                     cl = column( KrPermissions );
                  else
                     if ( mode & KrView::Owner )
                        cl = column( Owner );
                     else
                        if ( mode & KrView::Group )
                           cl = column( Group );
   setSorting( cl, ascending );
   KListView::sort();
}

void KrDetailedView::slotClicked( QListViewItem *item ) {
   if ( !item )
      return ;

   KConfigGroupSaver grpSvr( _config, nameInKConfig() );
   QString tmp = dynamic_cast<KrViewItem*>( item ) ->name();

   if ( _config->readBoolEntry( "Single Click Selects", _SingleClickSelects ) ) {
      emit executed( tmp );
   }
}

void KrDetailedView::slotDoubleClicked( QListViewItem *item ) {
   KConfigGroupSaver grpSvr( _config, nameInKConfig() );
   if ( !_config->readBoolEntry( "Single Click Selects", _SingleClickSelects ) ) {
      if ( !item )
         return ;
      QString tmp = dynamic_cast<KrViewItem*>( item ) ->name();
      emit executed( tmp );
   }
}

void KrDetailedView::prepareForActive() {
   setFocus();
   _focused = true;
}

void KrDetailedView::prepareForPassive() {
   if ( krApp && krApp->mainView && krApp->mainView->activePanel &&
       krApp->mainView->activePanel->quickSearch && krApp->mainView->activePanel->quickSearch->isShown() )
     stopQuickSearch(0);
   _focused = false;
}

void KrDetailedView::slotItemDescription( QListViewItem *item ) {
   KrViewItem * it = dynamic_cast<KrViewItem*>( item );
   if ( !it )
      return ;
   QString desc = it->description();
   emit itemDescription( desc );
}

void KrDetailedView::slotCurrentChanged( QListViewItem *item ) {
   if ( !item )
      return ;
   _nameToMakeCurrent = dynamic_cast<KrViewItem*>( item ) ->name();
}

void KrDetailedView::contentsMousePressEvent( QMouseEvent *e ) {
   if ( !_focused )
      emit needFocus();
   KListView::contentsMousePressEvent( e );
}

void KrDetailedView::contentsWheelEvent( QWheelEvent *e ) {
   if ( !_focused )
      emit needFocus();
   KListView::contentsWheelEvent( e );
}

void KrDetailedView::handleContextMenu( QListViewItem* it, const QPoint& pos, int ) {
   if ( !_focused )
      emit needFocus();
   if ( !it )
      return ;
   if ( dynamic_cast<KrViewItem*>( it ) ->
         name() == ".." )
      return ;
   emit contextMenu( QPoint( pos.x(), pos.y() - header() ->height() ) );
}

void KrDetailedView::startDrag() {
   QStringList items;
   getSelectedItems( &items );
   if ( items.empty() )
      return ; // don't drag an empty thing
   QPixmap px;
   if ( items.count() > 1 )
      px = FL_LOADICON( "queue" ); // how much are we dragging
   else
      px = getCurrentKrViewItem() ->icon();
   emit letsDrag( items, px );
}

KrViewItem *KrDetailedView::getKrViewItemAt( const QPoint &vp ) {
   return dynamic_cast<KrViewItem*>( KListView::itemAt( vp ) );
}

bool KrDetailedView::acceptDrag( QDropEvent* ) const {
   return true;
}

void KrDetailedView::contentsDropEvent( QDropEvent *e ) {
   /*  if (_currDragItem)
       dynamic_cast<KListViewItem*>(_currDragItem)->setPixmap(column(Name), FL_LOADICON("folder"));*/
   e->setPoint( contentsToViewport( e->pos() ) );
   emit gotDrop( e );
   e->ignore();
}

void KrDetailedView::contentsDragMoveEvent( QDragMoveEvent *e ) {
   /*  KrViewItem *i=getKrViewItemAt(contentsToViewport(e->pos()));
     // reset the last used icon
     if (_currDragItem != i && _currDragItem)
       dynamic_cast<KListViewItem*>(_currDragItem)->setPixmap(column(Name), FL_LOADICON("folder"));
     if (!i) {
       e->acceptAction();
       _currDragItem = 0L;
       KListView::contentsDragMoveEvent(e);
       return;
     }
     // otherwise, check if we're dragging on a directory
     if (i->name()=="..") {
       _currDragItem = 0L;
       e->acceptAction();
       KListView::contentsDragMoveEvent(e);
       return;
     }
     if (i->isDir()) {
       dynamic_cast<KListViewItem*>(i)->setPixmap(column(Name),FL_LOADICON("folder_open"));
       _currDragItem = i;
       KListView::contentsDragMoveEvent(e);
     }*/
   KListView::contentsDragMoveEvent( e );
}

void KrDetailedView::keyPressEvent( QKeyEvent *e ) {
   KConfigGroupSaver grpSvr( _config, nameInKConfig() );

   if ( !e || !firstChild() )
      return ; // subclass bug
   if ( krApp->mainView->activePanel->quickSearch->isShown() ) {
      krApp->mainView->activePanel->quickSearch->myKeyPressEvent( e );
      return ;
   }
   switch ( e->key() ) {
         case Key_Enter :
         case Key_Return : {
            if ( e->state() & ControlButton )  // let the panel handle it
               e->ignore();
            else {
               KrViewItem * i = getCurrentKrViewItem();
               QString tmp = i->name();
               emit executed( tmp );
            }
            break;
         }
         case Key_QuoteLeft :   // Terminal Emulator bugfix
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->home(); // ask krusader to move up a directory
            return ;         // safety
         }
         break;
         case Key_Right :
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else { // just a normal click - do a lynx-like moving thing
            KrViewItem *i = getCurrentKrViewItem();
            if ( i->name() == ".." ) { // if clicking on the ".." entry
               SLOTS->dirUp(); // ask krusader to move up a directory
               return ;
            }
            if ( i->isDir() ) {             // we create a return-pressed event,
               QString tmp = i->name();
               emit executed( tmp );  // thereby emulating a chdir
            }
            return ; // safety
         }
         case Key_Backspace :                  // Terminal Emulator bugfix
         case Key_Left :
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->dirUp(); // ask krusader to move up a directory
            return ;         // safety
         }
         //case Key_Up :
         //KListView::keyPressEvent( e );
         //break;
         case Key_Down :
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else
            KListView::keyPressEvent( e );
         break;
         case Key_Delete :              // kill file
         SLOTS->deleteFiles();
         return ;
         case Key_Space : {
            KrDetailedViewItem * viewItem = dynamic_cast<KrDetailedViewItem *> ( getCurrentKrViewItem() );
            if ( !viewItem || !( viewItem->isDir() && viewItem->size() <= 0 ) ) {
               KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Space, 0, 0 ) );
               return ; // wrong type, just mark(unmark it)
            }
            //
            // NOTE: this is buggy incase somewhere down in the folder we're calculating,
            // there's a folder we can't enter (permissions). in that case, the returned
            // size will not be correct.
            //
            KIO::filesize_t totalSize = 0;
            unsigned long totalFiles = 0, totalDirs = 0;
            QStringList names;
            names.push_back( viewItem->name() );
            if ( krApp->mainView->activePanel->func->calcSpace( names, totalSize, totalFiles, totalDirs ) ) {
               // did we succeed to calcSpace? we'll fail if we don't have permissions
               if ( totalSize == 0 ) { // just mark it, and bail out
                  KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Space, 0, 0 ) );
                  return ;
               }
               viewItem->setSize( totalSize );
               _countSize += totalSize;
               viewItem->repaintItem();
            }
            //KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Space, 0, 0 ) );
         }
         break;
         case Key_A :          // mark all
         if ( e->state() == ControlButton ) {
            KListView::keyPressEvent( e );
            updateView();
            break;
         }
         default:
         // if the key is A..Z or 1..0 do quick search otherwise...
         if ( ( e->key() >= Key_A && e->key() <= Key_Z ) ||
               ( e->key() >= Key_0 && e->key() <= Key_9 ) ||
               ( e->key() == Key_Backspace ) ||
               ( e->key() == Key_Down ) ||
               ( e->key() == Key_Period ) ) {
            // are we doing quicksearch? if not, send keys to panel
            if ( _config->readBoolEntry( "Do Quicksearch", _DoQuicksearch ) ) {
               // are we using krusader's classic quicksearch, or wincmd style?
               if ( _config->readBoolEntry( "Classic Quicksearch", _ClassicQuicksearch ) )
                  KListView::keyPressEvent( e );
               else {
                  // first, show the quicksearch if its hidden
                  if ( krApp->mainView->activePanel->quickSearch->isHidden() ) {
                     krApp->mainView->activePanel->quickSearch->show();
                     // second, we need to disable the dirup action - hack!
                     krDirUp->setEnabled( false );
                  }
                  // now, send the key to the quicksearch
                  krApp->mainView->activePanel->quickSearch->myKeyPressEvent( e );
               }
            } else
               e->ignore(); // send to panel
         } else {
            if ( krApp->mainView->activePanel->quickSearch->isShown() ) {
               krApp->mainView->activePanel->quickSearch->hide();
               krApp->mainView->activePanel->quickSearch->clear();
               krDirUp->setEnabled( true );
            }
            KListView::keyPressEvent( e );
         }
   }
   // emit the new item description
   slotItemDescription( currentItem() ); // actually send the QListViewItem
}

// overridden to make sure EXTENTION won't be lost during rename
void KrDetailedView::rename( QListViewItem *item, int c ) {
   // do we have an EXT column? if so, handle differently:
   // copy the contents of the EXT column over to the name
   if ( column( Extention ) != -1 ) {
      item->setText( column( Name ), dynamic_cast<KrViewItem*>( item ) ->name() );
      item->setText( column( Extention ), QString::null );
      repaintItem( item );
   }

   KListView::rename( item, c );
   renameLineEdit() ->selectAll();
}

void KrDetailedView::renameCurrentItem() {
   int c;
   QString newName, fileName;

   KrViewItem *it = getCurrentKrViewItem();
   if ( it )
      fileName = it->name();
   else
      return ; // quit if no current item available
   // don't allow anyone to rename ..
   if ( fileName == ".." )
      return ;

   // determine which column is inplace renameable
   for ( c = 0; c < columns(); c++ )
      if ( isRenameable( c ) )
         break; // one MUST be renamable
   if ( !isRenameable( c ) )
      c = -1; // failsafe

   if ( c >= 0 ) {
      rename( dynamic_cast<QListViewItem*>( it ), c );
      // signal will be emited when renaming is done, and finalization
      // will occur in inplaceRenameFinished()
   } else { // do this in case inplace renaming is disabled
      // good old dialog box
      bool ok = false;
      newName = KLineEditDlg::getText( i18n( "Rename " ) + fileName + i18n( " to:" ), fileName, &ok, krApp );
      // if the user canceled - quit
      if ( !ok || newName == fileName )
         return ;
      emit renameItem( it->name(), newName );
   }
}

void KrDetailedView::inplaceRenameFinished( QListViewItem * it, int ) {
   if ( !it ) { // major failure - call developers
      kdWarning() << "Major failure at inplaceRenameFinished(): item is null" << endl;
      exit( 0 );
   }
   // check if the item was indeed renamed
   if ( it->text( column( Name ) ) != dynamic_cast<KrDetailedViewItem*>( it ) ->name() )     // was renamed
      emit renameItem( dynamic_cast<KrDetailedViewItem*>( it ) ->name(), it->text( column( Name ) ) );
   else if ( column( Extention ) != -1 ) { // nothing happened, restore the view (if needed)
      int i;
      QString ext, name = dynamic_cast<KrDetailedViewItem*>( it ) ->name();
      if ( ( i = name.findRev( '.' ) ) > 0 ) {
         ext = name.mid( i + 1 );
         name = name.mid( 0, i );
      }
      it->setText( column( Name ), name );
      it->setText( column( Extention ), ext );
      repaintItem( it );
   }
   setFocus();
}

void KrDetailedView::quickSearch( const QString & str, int direction ) {
   KrViewItem * item = getCurrentKrViewItem();
   if ( !direction ) {
      if ( item->name().startsWith( str ) )
         return ;
      direction = 1;
   }
   KrViewItem * startItem = item;
   while ( true ) {
      item = ( direction > 0 ) ? getNext( item ) : getPrev( item );
      if ( !item )
         item = ( direction > 0 ) ? getFirst() : getLast();
      if ( item == startItem )
         return ;
      if ( item->name().startsWith( str ) ) {
         makeItemVisible( item );
         setCurrentItem( item->name() );
         return ;
      }
   }
}

void KrDetailedView::stopQuickSearch( QKeyEvent * e ) {
   krApp->mainView->activePanel->quickSearch->hide();
   krApp->mainView->activePanel->quickSearch->clear();
   krDirUp->setEnabled( true );
   if ( e )
      keyPressEvent( e );
}

//void KrDetailedView::focusOutEvent( QFocusEvent * e )
//{
//  if ( krApp->mainView->activePanel->quickSearch->isShown() )
//    stopQuickSearch(0);
//  KListView::focusOutEvent( e );
//}

void KrDetailedView::setNameToMakeCurrent( QListViewItem * it ) {
   KrView::setNameToMakeCurrent( dynamic_cast<KrViewItem*>( it ) ->name() );
}

void KrDetailedView::slotMouseClicked( int button, QListViewItem * item, const QPoint&, int ) {
   if ( button == Qt::MidButton )
      emit middleButtonClicked( item );
}
