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
#include "krcolorcache.h"
#include "krselectionmode.h"
#include "../krusader.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../krslots.h"
#include "../VFS/krpermhandler.h"
#include "../GUI/kcmdline.h"
#include "../Dialogs/krspecialwidgets.h"
#include "listpanel.h"
#include "panelfunc.h"
#include <qlayout.h>
#include <qdir.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <kprogress.h>
#include <kstatusbar.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>

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
//////////////////////////////////////////////////////////////////////////

#define CANCEL_TWO_CLICK_RENAME {singleClicked = false;renameTimer.stop();}
#define COLUMN(X)	static_cast<KrDetailedViewProperties*>(_properties)->column[ KrDetailedViewProperties::X ]
#define PROPS	static_cast<KrDetailedViewProperties*>(_properties)	

QString KrDetailedView::ColumnName[ KrDetailedViewProperties::MAX_COLUMNS ];

KrDetailedView::KrDetailedView( QWidget *parent, ListPanel *panel, bool &left, KConfig *cfg, const char *name ) :
      KListView( parent, name ), KrView( cfg ), _focused( false ), _currDragItem( 0L ),
_nameInKConfig( QString( "KrDetailedView" ) + QString( ( left ? "Left" : "Right" ) ) ), _left( left ) {
	initProperties(); // don't forget this!

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

   /////////////////////////////// listview ////////////////////////////////////
   { // use the {} so that KConfigGroupSaver will work correctly!
      KConfigGroupSaver grpSvr( _config, "Look&Feel" );
      setFont( _config->readFontEntry( "Filelist Font", _FilelistFont ) );
      // decide on single click/double click selection
      if ( _config->readBoolEntry( "Single Click Selects", _SingleClickSelects ) ) {
         connect( this, SIGNAL( executed( QListViewItem* ) ), this, SLOT( slotExecuted( QListViewItem* ) ) );
      } else {
         connect( this, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( slotClicked( QListViewItem* ) ) );
         connect( this, SIGNAL( doubleClicked( QListViewItem* ) ), this, SLOT( slotDoubleClicked( QListViewItem* ) ) );
      }

      // a change in the selection needs to update totals
      connect( this, SIGNAL( onItem( QListViewItem* ) ), this, SLOT( slotItemDescription( QListViewItem* ) ) );
      connect( this, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
               this, SLOT( handleContextMenu( QListViewItem*, const QPoint&, int ) ) );
      connect( this, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( setNameToMakeCurrent( QListViewItem* ) ) );
      connect( this, SIGNAL( mouseButtonClicked ( int, QListViewItem *, const QPoint &, int ) ),
               this, SLOT( slotMouseClicked ( int, QListViewItem *, const QPoint &, int ) ) );
      connect( &KrColorCache::getColorCache(), SIGNAL( colorsRefreshed() ), this, SLOT( refreshColors() ) );
   }

   setWidget( this );

   // add whatever columns are needed to the listview
   krConfig->setGroup( "Look&Feel" );
   
	newColumn( KrDetailedViewProperties::Name );  // we always have a name
   setColumnWidthMode( COLUMN(Name), QListView::Manual );
   if ( _config->readBoolEntry( "Ext Column", _ExtColumn ) ) {
      newColumn( KrDetailedViewProperties::Extention );
      setColumnWidthMode( COLUMN(Extention), QListView::Manual );
      setColumnWidth( COLUMN(Extention), QFontMetrics( font() ).width( "tar.bz2" ) );
   }
   if ( _config->readBoolEntry( "Mime Column", _MimeColumn ) ) {
      newColumn( KrDetailedViewProperties::Mime );
      setColumnWidthMode( COLUMN(Mime), QListView::Manual );
      setColumnWidth( COLUMN(Mime), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( _config->readBoolEntry( "Size Column", _SizeColumn ) ) {
      newColumn( KrDetailedViewProperties::Size );
      setColumnWidthMode( COLUMN(Size), QListView::Manual );
      setColumnWidth( COLUMN(Size), QFontMetrics( font() ).width( "9" ) * 10 );
      setColumnAlignment( COLUMN(Size), Qt::AlignRight ); // right-align numbers
   }
   if ( _config->readBoolEntry( "DateTime Column", _DateTimeColumn ) ) {
      newColumn( KrDetailedViewProperties::DateTime );
      setColumnWidthMode( COLUMN(DateTime), QListView::Manual );
      //setColumnWidth( column( DateTime ), QFontMetrics( font() ).width( "99/99/99  99:99" ) );
      setColumnWidth( COLUMN(DateTime), QFontMetrics( font() ).width( KGlobal::locale() ->formatDateTime(
                         QDateTime ( QDate( 2099, 12, 29 ), QTime( 23, 59 ) ) ) ) + 3 );
   }
   if ( _config->readBoolEntry( "Perm Column", _PermColumn ) ) {
      newColumn( KrDetailedViewProperties::Permissions );
      setColumnWidthMode( COLUMN(Permissions), QListView::Manual );
      setColumnWidth( COLUMN(Permissions), QFontMetrics( font() ).width( "drwxrwxrwx" ) );
   }
   if ( _config->readBoolEntry( "KrPerm Column", _KrPermColumn ) ) {
      newColumn( KrDetailedViewProperties::KrPermissions );
      setColumnWidthMode( COLUMN(KrPermissions), QListView::Manual );
      setColumnWidth( COLUMN(KrPermissions), QFontMetrics( font() ).width( "RWX" ) );
   }
   if ( _config->readBoolEntry( "Owner Column", _OwnerColumn ) ) {
      newColumn( KrDetailedViewProperties::Owner );
      setColumnWidthMode( COLUMN(Owner), QListView::Manual );
      setColumnWidth( COLUMN(Owner), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( _config->readBoolEntry( "Group Column", _GroupColumn ) ) {
      newColumn( KrDetailedViewProperties::Group );
      setColumnWidthMode( COLUMN(Group), QListView::Manual );
      setColumnWidth( COLUMN(Group), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   
   // determine basic settings for the listview
   setAcceptDrops( true );
   setDragEnabled( true );
   setTooltipColumn( COLUMN(Name) );
   //setDropVisualizer(false);
   //setDropHighlighter(false);
   setSelectionModeExt( KListView::FileManager );
   setAllColumnsShowFocus( true );
   setShowSortIndicator( true );
   header() ->setStretchEnabled( true, COLUMN(Name) );

   //---- don't enable these lines, as it causes an ugly bug with inplace renaming
   //-->  setItemsRenameable( true );
   //-->  setRenameable( column( Name ), true );
   //-------------------------------------------------------------------------------

   // allow in-place renaming
   connect( renameLineEdit(), SIGNAL( done( QListViewItem *, int ) ),
            this, SLOT( inplaceRenameFinished( QListViewItem*, int ) ) );
   connect( this, SIGNAL( renameItem( const QString &, const QString & ) ),
            panel->func, SLOT( rename( const QString &, const QString & ) ) );
   // connect quicksearch
   connect( panel->quickSearch, SIGNAL( textChanged( const QString& ) ),
            this, SLOT( quickSearch( const QString& ) ) );
   connect( panel->quickSearch, SIGNAL( otherMatching( const QString&, int ) ),
            this, SLOT( quickSearch( const QString& , int ) ) );
   connect( panel->quickSearch, SIGNAL( stop( QKeyEvent* ) ),
            this, SLOT( stopQuickSearch( QKeyEvent* ) ) );
   connect( panel->quickSearch, SIGNAL( process( QKeyEvent* ) ),
            this, SLOT( handleQuickSearchEvent( QKeyEvent* ) ) );
   connect( &renameTimer, SIGNAL( timeout() ), this, SLOT( renameCurrentItem() ) );
   connect( &contextMenuTimer, SIGNAL (timeout()), this, SLOT (showContextMenu()));


   setFocusPolicy( StrongFocus );
   restoreSettings();
   refreshColors();

   CANCEL_TWO_CLICK_RENAME;
}

KrDetailedView::~KrDetailedView() {
	delete _properties;
	_properties = 0;
}

void KrDetailedView::newColumn( KrDetailedViewProperties::ColumnType type ) {
	// get the next available column
	int max = KrDetailedViewProperties::Unused;
	for (int i=0; i<KrDetailedViewProperties::MAX_COLUMNS; ++i) {
		if (PROPS->column[i]>=max)
			max = PROPS->column[i]+1;
	}
	if ( max >= KrDetailedViewProperties::MAX_COLUMNS )
      perror( "KrDetailedView::newColumn() - too many columns" );
	
	PROPS->column[type] = max;
	addColumn( ColumnName[type], -1 );
}

/**
 * returns the number of column which holds values of type 'type'.
 * if such values are not presented in the view, -1 is returned.
 */
int KrDetailedView::column( KrDetailedViewProperties::ColumnType type ) {
	return PROPS->column[type];
}

void KrDetailedView::addItem( vfile *vf ) {
   QString size = KRpermHandler::parseSize( vf->vfile_getSize() );
   QString name = vf->vfile_getName();
   bool isDir = vf->vfile_isDir();
   if ( !isDir || ( isDir && ( _properties->filter & KrViewProperties::ApplyToDirs ) ) ) {
      switch ( _properties->filter ) {
            case KrViewProperties::All :
               break;
            case KrViewProperties::Custom :
            if ( !QDir::match( _properties->filterMask, name ) ) return ;
            break;
            case KrViewProperties::Dirs:
            if ( !vf->vfile_isDir() ) return ;
            break;
            case KrViewProperties::Files:
            if ( vf->vfile_isDir() ) return ;
            break;
            case KrViewProperties::ApplyToDirs :
            break; // no-op, stop compiler complaints
      }
   }
   // passed the filter ...
   KrDetailedViewItem *item = new KrDetailedViewItem( this, lastItem(), vf );
   // add to dictionary
   dict.insert( vf->vfile_getName(), item );
   if ( isDir )
      ++_numDirs;
   else _countSize += dynamic_cast<KrViewItem*>( item ) ->size();
   ++_count;
   
   if (item->name() == nameToMakeCurrent() )
      setCurrentItem(item->name()); // dictionary based - quick

   ensureItemVisible( currentItem() );
   emit selectionChanged();
}

void KrDetailedView::delItem( const QString &name ) {
	 KrDetailedViewItem * it = dict[ name ];
   if ( !it ) {
      krOut << "got signal deletedVfile(" << name << ") but can't find KrViewItem" << endl;
		return;
	} 
	if (it->isDir()) {
		--_numDirs;
	} else {
		_countSize -= it->size();
	}
	--_count;
	
	delete it;
	emit selectionChanged();
}

void KrDetailedView::updateItem( vfile *vf ) {
   // since we're deleting the item, make sure we keep
   // it's properties first and repair it later
   KrDetailedViewItem * it = dict[ vf->vfile_getName() ];
   if ( !it ) {
      krOut << "got signal updatedVfile(" << vf->vfile_getName() << ") but can't find KrViewItem" << endl;
   } else {
		bool selected = it->isSelected();
      bool current = ( getCurrentKrViewItem() == it );
      delItem( vf->vfile_getName() );
      addItem( vf );
      // restore settings
      ( dict[ vf->vfile_getName() ] ) ->setSelected( selected );
      if ( current )
         setCurrentItem( vf->vfile_getName() );
   }
	emit selectionChanged();
}


void KrDetailedView::addItems( vfs *v, bool addUpDir ) {
   QListViewItem * item = firstChild();
   QListViewItem *currentItem = item;
   QString size, name;

   // add the up-dir arrow if needed
   if ( addUpDir ) {
      KListViewItem * item = new KrDetailedViewItem( this, ( QListViewItem* ) 0L, ( vfile* ) 0L );
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
      if ( !isDir || ( isDir && ( _properties->filter & KrViewProperties::ApplyToDirs ) ) ) {
         switch ( _properties->filter ) {
               case KrViewProperties::All :
               break;
               case KrViewProperties::Custom :
               if ( !QDir::match( _properties->filterMask, name ) )
                  continue;
               break;
               case KrViewProperties::Dirs:
               if ( !vf->vfile_isDir() )
                  continue;
               break;
               case KrViewProperties::Files:
               if ( vf->vfile_isDir() )
                  continue;
               break;

               case KrViewProperties::ApplyToDirs :
               break; // no-op, stop compiler complaints
         }
      }

      item = new KrDetailedViewItem( this, item, vf );
      dict.insert( vf->vfile_getName(), dynamic_cast<KrDetailedViewItem*>(item) );
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
   KrDetailedViewItem * it = dict[ name ];
   if ( it )
      KListView::setCurrentItem( it );
#if 0
   for ( QListViewItem * it = firstChild(); it != 0; it = it->itemBelow() )
      if ( dynamic_cast<KrViewItem*>( it ) ->
            name() == name ) {
         KListView::setCurrentItem( it );
         break;
      }
#endif

}

void KrDetailedView::clear() {
   emit KListView::selectionChanged(); /* to avoid rename crash at refresh */
   KListView::clear();
   _count = _numSelected = _numDirs = _selectedSize = _countSize = 0;
   dict.clear();
}

void KrDetailedView::setSortMode( KrViewProperties::SortSpec mode ) {
   KrView::setSortMode(mode); // the KrViewItems will check it by themselves
   bool ascending = !( mode & KrViewProperties::Descending );
   int cl = -1;
   if ( mode & KrViewProperties::Name )
      cl = COLUMN( Name );
   else
      if ( mode & KrViewProperties::Size )
         cl = COLUMN( Extention );
      else
         if ( mode & KrViewProperties::Type )
            cl = COLUMN( Mime );
         else
            if ( mode & KrViewProperties::Modified )
               cl = COLUMN( DateTime );
            else
               if ( mode & KrViewProperties::Permissions )
                  cl = COLUMN( Permissions );
               else
                  if ( mode & KrViewProperties::KrPermissions )
                     cl = COLUMN( KrPermissions );
                  else
                     if ( mode & KrViewProperties::Owner )
                        cl = COLUMN( Owner );
                     else
                        if ( mode & KrViewProperties::Group )
                           cl = COLUMN( Group );
   setSorting( cl, ascending );
   KListView::sort();
}

void KrDetailedView::slotClicked( QListViewItem *item ) {
   if ( !item ) return ;

   if ( !modifierPressed ) {
      if ( singleClicked && !renameTimer.isActive() ) {
         KConfig * config = KGlobal::config();
         config->setGroup( "KDE" );
         int doubleClickInterval = config->readNumEntry( "DoubleClickInterval", 400 );

         int msecsFromLastClick = clickTime.msecsTo( QTime::currentTime() );

         if ( msecsFromLastClick > doubleClickInterval && msecsFromLastClick < 5 * doubleClickInterval ) {
            singleClicked = false;
            renameTimer.start( doubleClickInterval, true );
            return ;
         }
      }

      CANCEL_TWO_CLICK_RENAME;
      singleClicked = true;
      clickTime = QTime::currentTime();
      clickedItem = item;
   }
}

void KrDetailedView::slotDoubleClicked( QListViewItem *item ) {
   CANCEL_TWO_CLICK_RENAME;
   if ( !item )
      return ;
   QString tmp = dynamic_cast<KrViewItem*>( item ) ->name();
   emit executed( tmp );
}

void KrDetailedView::prepareForActive() {
   setFocus();
   _focused = true;
}

void KrDetailedView::prepareForPassive() {
   CANCEL_TWO_CLICK_RENAME;
   if ( renameLineEdit() ->isVisible() )
      renameLineEdit() ->clearFocus();
   KConfigGroupSaver grpSvr( _config, "Look&Feel" );
   if ( _config->readBoolEntry( "New Style Quicksearch", _NewStyleQuicksearch ) ) {
      if ( MAIN_VIEW ) {
         if ( ACTIVE_PANEL ) {
            if ( ACTIVE_PANEL->quickSearch ) {
               if ( ACTIVE_PANEL->quickSearch->isShown() ) {
                  stopQuickSearch( 0 );
               }
            }
         }
      }
   }
   _focused = false;
}

void KrDetailedView::slotItemDescription( QListViewItem * item ) {
   KrViewItem * it = dynamic_cast<KrViewItem*>( item );
   if ( !it )
      return ;
   QString desc = it->description();
   emit itemDescription( desc );
}

void KrDetailedView::handleQuickSearchEvent( QKeyEvent * e ) {
   switch ( e->key() ) {
         case Key_Insert:
         KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Space, 0, 0 ) );
         keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Down, 0, 0 ) );
         break;
         case Key_Home:
         QListView::setCurrentItem( firstChild() );
         keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Down, 0, 0 ) );
         break;
         case Key_End:
         QListView::setCurrentItem( firstChild() );
         keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Up, 0, 0 ) );
         break;
   }
}


void KrDetailedView::slotCurrentChanged( QListViewItem * item ) {
   CANCEL_TWO_CLICK_RENAME;
   if ( !item )
      return ;
   _nameToMakeCurrent = dynamic_cast<KrViewItem*>( item ) ->name();
}

void KrDetailedView::contentsMousePressEvent( QMouseEvent * e ) {
   bool callDefaultHandler = true, processEvent = true, selectionChanged = false;
   QListViewItem * oldCurrent = currentItem();
   QListViewItem *newCurrent = itemAt( contentsToViewport( e->pos() ) );
   if (e->button() == RightButton)
   {
     if (KrSelectionMode::getSelectionHandler()->rightButtonSelects())
     {
       if (KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() && !(e->state() & ShiftButton)
          && !(e->state() & ControlButton) && !(e->state() & AltButton))
       {
         if (newCurrent)
         {
           newCurrent->setSelected(!newCurrent->isSelected());
           newCurrent->repaint();
			  selectionChanged = true;
         }
         callDefaultHandler = false;
         processEvent = false;
         e->accept();
       }
     }
     else
     {
       callDefaultHandler = false;
       processEvent = false;
       e->accept();
     }
   }
   if (e->button() == LeftButton)
   {
     if (KrSelectionMode::getSelectionHandler()->leftButtonSelects())
     {
       if (KrSelectionMode::getSelectionHandler()->leftButtonPreservesSelection() && !(e->state() & ShiftButton)
          && !(e->state() & ControlButton) && !(e->state() & AltButton))
       {
         if (newCurrent)
         {
           newCurrent->setSelected(!newCurrent->isSelected());
           newCurrent->repaint();
			  selectionChanged = true;
         }
         callDefaultHandler = false;
         processEvent = false;
         e->accept();
       }
     }
     else
     {
       callDefaultHandler = false;
       processEvent = false;
       e->accept();
     }
   }

   modifierPressed = false;
   if ( (e->state() & ShiftButton) || (e->state() & ControlButton) || (e->state() & AltButton) ) {
      CANCEL_TWO_CLICK_RENAME;
      modifierPressed = true;
   }

   // stop quick search in case a mouse click occured
   KConfigGroupSaver grpSvr( _config, "Look&Feel" );
   if ( _config->readBoolEntry( "New Style Quicksearch", _NewStyleQuicksearch ) ) {
      if ( MAIN_VIEW ) {
         if ( ACTIVE_PANEL ) {
            if ( ACTIVE_PANEL->quickSearch ) {
               if ( ACTIVE_PANEL->quickSearch->isShown() ) {
                  stopQuickSearch( 0 );
               }
            }
         }
      }
   }

   if ( !_focused )
      emit needFocus();
   if (processEvent && ( (e->state() & ShiftButton) || (e->state() & ControlButton) || (e->state() & AltButton) ) && !KrSelectionMode::getSelectionHandler()->useQTSelection()){
      if ( oldCurrent && newCurrent && oldCurrent != newCurrent && e->state() & ShiftButton ) {
         int oldPos = oldCurrent->itemPos();
         int newPos = newCurrent->itemPos();
         QListViewItem *top = 0, *bottom = 0;
         if ( oldPos > newPos ) {
            top = newCurrent;
            bottom = oldCurrent;
         } else {
            top = oldCurrent;
            bottom = newCurrent;
         }
         QListViewItemIterator it( top );
         for ( ; it.current(); ++it ) {
            if ( !it.current() ->isSelected() ) {
               it.current() ->setSelected( true );
               selectionChanged = true;
            }
            if ( it.current() == bottom )
               break;
         }
         QListView::setCurrentItem( newCurrent );
         callDefaultHandler = false;
      }
   }
	
	if (selectionChanged)
		updateView(); // don't call triggerUpdate directly!
	
   //   QListViewItem * i = itemAt( contentsToViewport( e->pos() ) );
   if (callDefaultHandler)
     KListView::contentsMousePressEvent( e );
   //   if (i != 0) // comment in, if click sould NOT select
   //     setSelected(i, FALSE);
   if (newCurrent) QListView::setCurrentItem(newCurrent);

   if ( ACTIVE_PANEL->quickSearch->isShown() ) {
      ACTIVE_PANEL->quickSearch->hide();
      ACTIVE_PANEL->quickSearch->clear();
      krDirUp->setEnabled( true );
   }
   if ( OTHER_PANEL->quickSearch->isShown() ) {
      OTHER_PANEL->quickSearch->hide();
      OTHER_PANEL->quickSearch->clear();
      krDirUp->setEnabled( true );
   }
}

void KrDetailedView::contentsMouseReleaseEvent( QMouseEvent * e ) {
  if (e->button() == RightButton)
    contextMenuTimer.stop();
  KListView::contentsMouseReleaseEvent( e );
}

void KrDetailedView::contentsMouseMoveEvent ( QMouseEvent * e ) {
   if ( ( singleClicked || renameTimer.isActive() ) && itemAt( contentsToViewport( e->pos() ) ) != clickedItem )
      CANCEL_TWO_CLICK_RENAME;
   KListView::contentsMouseMoveEvent( e );
}

void KrDetailedView::contentsWheelEvent( QWheelEvent * e ) {
   if ( !_focused )
      emit needFocus();
   KListView::contentsWheelEvent( e );
}

void KrDetailedView::handleContextMenu( QListViewItem * it, const QPoint & pos, int ) {
   if ( !_focused )
      emit needFocus();
   if ( !it )
      return ;
   if ( dynamic_cast<KrViewItem*>( it ) ->
         name() == ".." )
      return ;
   int i = KrSelectionMode::getSelectionHandler()->showContextMenu();
   contextMenuPoint = QPoint( pos.x(), pos.y() - header() ->height() );
   if (i < 0)
     showContextMenu();
   else if (i > 0)
     contextMenuTimer.start(i, true);
}

void KrDetailedView::showContextMenu()
{
   emit contextMenu( contextMenuPoint );
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

KrViewItem *KrDetailedView::getKrViewItemAt( const QPoint & vp ) {
   return dynamic_cast<KrViewItem*>( KListView::itemAt( vp ) );
}

bool KrDetailedView::acceptDrag( QDropEvent* ) const {
   return true;
}

void KrDetailedView::contentsDropEvent( QDropEvent * e ) {
   /*  if (_currDragItem)
       dynamic_cast<KListViewItem*>(_currDragItem)->setPixmap(column(Name), FL_LOADICON("folder"));*/
   e->setPoint( contentsToViewport( e->pos() ) );
   emit gotDrop( e );
   e->ignore();
   KListView::contentsDropEvent( e );
}

void KrDetailedView::contentsDragMoveEvent( QDragMoveEvent * e ) {
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

void KrDetailedView::keyPressEvent( QKeyEvent * e ) {
   if ( !e || !firstChild() )
      return ; // subclass bug
   if ( ACTIVE_PANEL->quickSearch->isShown() ) {
      ACTIVE_PANEL->quickSearch->myKeyPressEvent( e );
      return ;
   }
   switch ( e->key() ) {
         case Key_Up :
			if (e->state() == ControlButton) { // let the panel handle it - jump to origin bar
				e->ignore();
				break;
			} else if (!KrSelectionMode::getSelectionHandler()->useQTSelection()) {
            QListViewItem * i = currentItem();
            if ( !i ) break;
            if ( e->state() == ShiftButton ) setSelected( i, !i->isSelected() );
            i = i->itemAbove();
         	if ( i ) {
					QListView::setCurrentItem( i );
					QListView::ensureItemVisible( i );
				}
         } else KListView::keyPressEvent(e);
         return;
         case Key_Down :
         if ( e->state() == ControlButton ) { // let the panel handle it - jump to command line
            e->ignore();
            break;
         } else if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            QListViewItem * i = currentItem();
            if ( !i ) break;
            if ( e->state() == ShiftButton ) setSelected( i, !i->isSelected() );
            i = i->itemBelow();
         if ( i ) {QListView::setCurrentItem( i ); QListView::ensureItemVisible( i ); }
         } else KListView::keyPressEvent(e);
         return;
         case Key_Next:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            QListViewItem * i = currentItem(), *j;
            if ( !i ) break;
            QRect r( itemRect( i ) );
            if ( !r.height() ) break;
            for ( int page = visibleHeight() / r.height() - 1; page > 0 && ( j = i->itemBelow() ); --page )
               i = j;
            if ( i ) {QListView::setCurrentItem( i ); QListView::ensureItemVisible( i ); }
         } else KListView::keyPressEvent(e);
         return;
         case Key_Prior:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            QListViewItem * i = currentItem(), *j;
            if ( !i ) break;
            QRect r( itemRect( i ) );
            if ( !r.height() ) break;
            for ( int page = visibleHeight() / r.height() - 1; page > 0 && ( j = i->itemAbove() ); --page )
               i = j;
            if ( i ) {QListView::setCurrentItem( i ); QListView::ensureItemVisible( i ); }
         } else KListView::keyPressEvent(e);
         return;
         case Key_Home:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            if ( e->state() & ShiftButton )  /* Shift+Home */
            {
               clearSelection();
               KListView::keyPressEvent( e );
               emit selectionChanged();
               triggerUpdate();
               break;
            } else {
               QListViewItem * i = firstChild();
               if ( i ) {QListView::setCurrentItem( i ); QListView::ensureItemVisible( i ); }
            }
         } else KListView::keyPressEvent(e);
         return;
         case Key_End:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            if ( e->state() & ShiftButton )  /* Shift+End */
            {
               clearSelection();
               KListView::keyPressEvent( e );
               emit selectionChanged();
               triggerUpdate();
               break;
            } else {
               QListViewItem *i = firstChild(), *j;
               while ( ( j = i->nextSibling() ) )
                  i = j;
               while ( ( j = i->itemBelow() ) )
                  i = j;
            if ( i ) {QListView::setCurrentItem( i ); QListView::ensureItemVisible( i ); }
               break;
            }
         } else KListView::keyPressEvent(e);
         return;
         case Key_Enter :
         case Key_Return : {
            if ( e->state() & ControlButton )         // let the panel handle it
               e->ignore();
            else {
               KrViewItem * i = getCurrentKrViewItem();
               QString tmp = i->name();
               emit executed( tmp );
            }
            break;
         }
         case Key_QuoteLeft :          // Terminal Emulator bugfix
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->home(); // ask krusader to move up a directory
            return ;         // safety
         }
         break;
         case Key_Right :
         if ( e->state() == ControlButton || e->state() == ShiftButton ) { // let the panel handle it
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
         case Key_Backspace :                         // Terminal Emulator bugfix
         case Key_Left :
         if ( e->state() == ControlButton || e->state() == ShiftButton ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->dirUp(); // ask krusader to move up a directory
            return ;         // safety
         }
         //case Key_Up :
         //KListView::keyPressEvent( e );
         //break;
/*#ifndef _newSelectionHandling
         case Key_Down :
         if ( e->state() == ControlButton ) { // let the panel handle it
            e->ignore();
            break;
         } else
            KListView::keyPressEvent( e );
         break;
#endif*/
         case Key_Delete :                   // kill file
         SLOTS->deleteFiles();
         return ;
         case Key_Insert : {
            if (KrSelectionMode::getSelectionHandler()->insertMovesDown())
               KListView::keyPressEvent( e );
            else
               KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Space, 0, 0 ) );
            return ; 
         }
         case Key_Space : {
            KrDetailedViewItem * viewItem = dynamic_cast<KrDetailedViewItem *> ( getCurrentKrViewItem() );
            if ( !viewItem || viewItem->name() == ".." ) { // wrong type, just mark(unmark it)
               if (KrSelectionMode::getSelectionHandler()->spaceMovesDown())
                  KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Insert, 0, 0 ) );
               else
                  KListView::keyPressEvent( e );
               return ; 
            }
            if ( viewItem->isDir() && viewItem->size() <= 0 &&  KrSelectionMode::getSelectionHandler()->spaceCalculatesDiskSpace()) {
               //
               // NOTE: this is buggy incase somewhere down in the folder we're calculating,
               // there's a folder we can't enter (permissions). in that case, the returned
               // size will not be correct.
               //
               KIO::filesize_t totalSize = 0;
               unsigned long totalFiles = 0, totalDirs = 0;
               QStringList items;
               items.push_back( viewItem->name() );
            if ( ACTIVE_PANEL->func->calcSpace( items, totalSize, totalFiles, totalDirs ) ) {
                  // did we succeed to calcSpace? we'll fail if we don't have permissions
                  if ( totalSize == 0 ) { // just mark it, and bail out
                     goto mark;
                  }
                  viewItem->setSize( totalSize );
                  viewItem->repaintItem();
               }
            }
mark:       if (KrSelectionMode::getSelectionHandler()->spaceMovesDown())
               KListView::keyPressEvent( new QKeyEvent( QKeyEvent::KeyPress, Key_Insert, 0, 0 ) );
            else
               KListView::keyPressEvent( e );
         }
         break;
         case Key_A :                 // mark all
         if ( e->state() == ControlButton ) {
            KListView::keyPressEvent( e );
            updateView();
            break;
         }
         default:
         if ( e->key() == Key_Escape ) {
            QListView::keyPressEvent( e ); return ; // otherwise the selection gets lost??!??
         }
         // if the key is A..Z or 1..0 do quick search otherwise...
         if ( e->text().length() > 0 && e->text() [ 0 ].isPrint() )       // better choice. Otherwise non-ascii characters like  can not be the first character of a filename
            /*         if ( ( e->key() >= Key_A && e->key() <= Key_Z ) ||
                           ( e->key() >= Key_0 && e->key() <= Key_9 ) ||
                           ( e->key() == Key_Backspace ) ||
                           ( e->key() == Key_Down ) ||
                           ( e->key() == Key_Period ) ) */{ 
            // are we doing quicksearch? if not, send keys to panel
            //if ( _config->readBoolEntry( "Do Quicksearch", _DoQuicksearch ) ) {
               // are we using krusader's classic quicksearch, or wincmd style?
               {
						KConfigGroupSaver grpSvr( _config, "Look&Feel" );
						if ( !_config->readBoolEntry( "New Style Quicksearch", _NewStyleQuicksearch ) )
							KListView::keyPressEvent( e );
						else {
							// first, show the quicksearch if its hidden
							if ( ACTIVE_PANEL->quickSearch->isHidden() ) {
								ACTIVE_PANEL->quickSearch->show();
								// second, we need to disable the dirup action - hack!
								krDirUp->setEnabled( false );
							}
							// now, send the key to the quicksearch
							ACTIVE_PANEL->quickSearch->myKeyPressEvent( e );
						}
					}
            //} else
            //   e->ignore(); // send to panel
         } else {
            if ( ACTIVE_PANEL->quickSearch->isShown() ) {
               ACTIVE_PANEL->quickSearch->hide();
               ACTIVE_PANEL->quickSearch->clear();
               krDirUp->setEnabled( true );
            }
            KListView::keyPressEvent( e );
         }
   }
   // emit the new item description
   slotItemDescription( currentItem() ); // actually send the QListViewItem
}

// overridden to make sure EXTENTION won't be lost during rename
void KrDetailedView::rename( QListViewItem * item, int c ) {
   // do we have an EXT column? if so, handle differently:
   // copy the contents of the EXT column over to the name
   if ( COLUMN( Extention ) != -1 ) {
      item->setText( COLUMN( Name ), dynamic_cast<KrViewItem*>( item ) ->name() );
      item->setText( COLUMN( Extention ), QString::null );
      repaintItem( item );
   }

   renameLineEdit()->setBackgroundMode(Qt::FixedColor);
   renameLineEdit()->setPaletteBackgroundColor(Qt::white);
   renameLineEdit()->setPaletteForegroundColor(Qt::black);
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
      newName = KInputDialog::getText( i18n( "Rename" ), i18n( "Rename " ) + fileName + i18n( " to:" ),
                                       fileName, &ok, krApp );
      // if the user canceled - quit
      if ( !ok || newName == fileName )
         return ;
      emit renameItem( it->name(), newName );
   }
}

void KrDetailedView::inplaceRenameFinished( QListViewItem * it, int ) {
   if ( !it ) { // major failure - call developers
      krOut << "Major failure at inplaceRenameFinished(): item is null" << endl;
      return;
   }
   // check if the item was indeed renamed
   bool restoreView = false;
   if ( it->text( COLUMN( Name ) ) != dynamic_cast<KrDetailedViewItem*>( it ) ->name() ) { // was renamed
      emit renameItem( dynamic_cast<KrDetailedViewItem*>( it ) ->name(), it->text( COLUMN( Name ) ) );
   } else restoreView = true;

   if ( COLUMN( Extention ) != -1 && restoreView ) { // nothing happened, restore the view (if needed)
      int i;
      QString ext, name = dynamic_cast<KrDetailedViewItem*>( it ) ->name();
      if ( !dynamic_cast<KrDetailedViewItem*>( it ) ->isDir() )
         if ( ( i = name.findRev( '.' ) ) > 0 ) {
            ext = name.mid( i + 1 );
            name = name.mid( 0, i );
         }
      it->setText( COLUMN( Name ), name );
      it->setText( COLUMN( Extention ), ext );
      repaintItem( it );
   }
   setFocus();
}

void KrDetailedView::quickSearch( const QString & str, int direction ) {
   KrViewItem * item = getCurrentKrViewItem();
   KConfigGroupSaver grpSvr( _config, "Look&Feel" );
   bool caseSensitive = _config->readBoolEntry( "Case Sensitive Quicksearch", _CaseSensitiveQuicksearch );
   if ( !direction ) {
      if ( caseSensitive ? item->name().startsWith( str ) : item->name().lower().startsWith( str.lower() ) )
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
      if ( caseSensitive ? item->name().startsWith( str ) : item->name().lower().startsWith( str.lower() ) ) {
			setCurrentItem( item->name() );
			makeItemVisible( item );
         return ;
      }
   }
}

void KrDetailedView::stopQuickSearch( QKeyEvent * e ) {
   ACTIVE_PANEL->quickSearch->hide();
   ACTIVE_PANEL->quickSearch->clear();
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

void KrDetailedView::refreshColors() {
   if ( !KrColorCache::getColorCache().isKDEDefault() ) {
      // KDE deafult is not choosen: set the background color (as this paints the empty areas) and the alternate color
      bool isActive = hasFocus();
      if ( MAIN_VIEW && ACTIVE_PANEL && ACTIVE_PANEL->view )
         isActive = ( dynamic_cast<KrView *>( this ) == ACTIVE_PANEL->view );
      QColor color = KrColorCache::getColorCache().getBackgroundColor( isActive );
      setPaletteBackgroundColor( color );
      color = KrColorCache::getColorCache().getAlternateBackgroundColor( isActive );
      setAlternateBackground( color );
   } else {
      // KDE deaful tis choosen: set back the background color
      setPaletteBackgroundColor( KGlobalSettings::baseColor() );
      // Set the alternate color to its default or to an invalid color, to turn alternate the background off.
      setAlternateBackground( KrColorCache::getColorCache().isAlternateBackgroundEnabled() ? KGlobalSettings::alternateBackgroundColor() : QColor() );
   }
}

bool KrDetailedView::event( QEvent *e ) {
   modifierPressed = false;

   switch ( e->type() ) {
         case QEvent::Timer:
         case QEvent::MouseMove:
         case QEvent::MouseButtonPress:
         case QEvent::MouseButtonRelease:
         break;
         default:
         CANCEL_TWO_CLICK_RENAME;
   }
   return KListView::event( e );
}

void KrDetailedView::makeItemVisible( const KrViewItem *item ) {
	qApp->processEvents();
	ensureItemVisible( dynamic_cast<const QListViewItem*>( item ) ); 
}

void KrDetailedView::initProperties() {
	_properties = new KrDetailedViewProperties;
	PROPS->displayIcons = _config->readBoolEntry( "With Icons", _WithIcons );
	PROPS->sortMode = static_cast<KrViewProperties::SortSpec>( KrViewProperties::Name |
		KrViewProperties::Descending | KrViewProperties::DirsFirst );
	{
	KConfigGroupSaver grpSvr( _config, "Look&Feel" );
	if ( !_config->readBoolEntry( "Case Sensative Sort", _CaseSensativeSort ) )
      	PROPS->sortMode = static_cast<KrViewProperties::SortSpec>( _properties->sortMode |
				 KrViewProperties::IgnoreCase );
	PROPS->humanReadableSize = krConfig->readBoolEntry("Human Readable Size", _HumanReadableSize);
	}
}

#include "krdetailedview.moc"
