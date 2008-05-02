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
#include "krviewfactory.h"
#include "../krusader.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../krslots.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"
#include "../GUI/kcmdline.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../panelmanager.h"
#include <qlayout.h>
#include <qdir.h>
#include <q3header.h>
#include <qstyle.h>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <kstatusbar.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmenu.h>
#include <kcolorscheme.h>
#include <qstyle.h>

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
#define VF	getVfile()

#define COLUMN_POPUP_IDS    91

#define DETAILEDVIEW_ID 0
KrViewInstance detailedView( DETAILEDVIEW_ID, i18n( "&Detailed View" ), Qt::ALT + Qt::SHIFT + Qt::Key_D,
                             KrDetailedView::create, KrDetailedViewItem::itemHeightChanged );

QString KrDetailedView::ColumnName[ KrDetailedViewProperties::MAX_COLUMNS ];

KrDetailedView::KrDetailedView( QWidget *parent, bool &left, KConfig *cfg ) :
      K3ListView( parent ), KrView( cfg ), _currDragItem( 0L ), currentlyRenamedItem( 0 ),
      pressedItem( 0 ) {
	setWidget( this );
	_nameInKConfig=QString( "KrDetailedView" ) + QString( ( left ? "Left" : "Right" ) ) ;
	KConfigGroup group( krConfig, "Private" );
	if (group.readEntry("Enable Input Method", true))
		setInputMethodEnabled(true);
}

void KrDetailedView::setup() {
	lastSwushPosition = 0;
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
   { // use the {} so that KConfigGroup will work correctly!
      KConfigGroup grpSvr( _config, "Look&Feel" );
      setFont( grpSvr.readEntry( "Filelist Font", *_FilelistFont ) );
      // decide on single click/double click selection
      if ( grpSvr.readEntry( "Single Click Selects", _SingleClickSelects ) &&
           KGlobalSettings::singleClick() ) {
         connect( this, SIGNAL( executed( Q3ListViewItem* ) ), this, SLOT( slotExecuted( Q3ListViewItem* ) ) );
      } else {
         connect( this, SIGNAL( clicked( Q3ListViewItem* ) ), this, SLOT( slotClicked( Q3ListViewItem* ) ) );
         connect( this, SIGNAL( doubleClicked( Q3ListViewItem* ) ), this, SLOT( slotDoubleClicked( Q3ListViewItem* ) ) );
      }

      // a change in the selection needs to update totals
      connect( this, SIGNAL( onItem( Q3ListViewItem* ) ), this, SLOT( slotItemDescription( Q3ListViewItem* ) ) );
      connect( this, SIGNAL( contextMenuRequested( Q3ListViewItem*, const QPoint&, int ) ),
               this, SLOT( handleContextMenu( Q3ListViewItem*, const QPoint&, int ) ) );
		connect( this, SIGNAL( rightButtonPressed(Q3ListViewItem*, const QPoint&, int)),
			this, SLOT(slotRightButtonPressed(Q3ListViewItem*, const QPoint&, int)));
      connect( this, SIGNAL( currentChanged( Q3ListViewItem* ) ), this, SLOT( setNameToMakeCurrent( Q3ListViewItem* ) ) );
      connect( this, SIGNAL( currentChanged( Q3ListViewItem* ) ), this, SLOT( transformCurrentChanged( Q3ListViewItem* ) ) );
      connect( this, SIGNAL( mouseButtonClicked ( int, Q3ListViewItem *, const QPoint &, int ) ),
               this, SLOT( slotMouseClicked ( int, Q3ListViewItem *, const QPoint &, int ) ) );
      connect( &KrColorCache::getColorCache(), SIGNAL( colorsRefreshed() ), this, SLOT( refreshColors() ) );
		connect( header(), SIGNAL(clicked(int)), this, SLOT(sortOrderChanged(int )));
   }

   // add whatever columns are needed to the listview
   KConfigGroup group( krConfig, nameInKConfig() );
   
	newColumn( KrDetailedViewProperties::Name );  // we always have a name
   setColumnWidthMode( COLUMN(Name), Q3ListView::Manual );
   if ( group.readEntry( "Ext Column", _ExtColumn ) ) {
      newColumn( KrDetailedViewProperties::Extention );
      setColumnWidthMode( COLUMN(Extention), Q3ListView::Manual );
      setColumnWidth( COLUMN(Extention), QFontMetrics( font() ).width( "tar.bz2" ) );
   }
   if ( group.readEntry( "Mime Column", _MimeColumn ) ) {
      newColumn( KrDetailedViewProperties::Mime );
      setColumnWidthMode( COLUMN(Mime), Q3ListView::Manual );
      setColumnWidth( COLUMN(Mime), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( group.readEntry( "Size Column", _SizeColumn ) ) {
      newColumn( KrDetailedViewProperties::Size );
      setColumnWidthMode( COLUMN(Size), Q3ListView::Manual );
      setColumnWidth( COLUMN(Size), QFontMetrics( font() ).width( "9" ) * 10 );
      setColumnAlignment( COLUMN(Size), Qt::AlignRight ); // right-align numbers
   }
   if ( group.readEntry( "DateTime Column", _DateTimeColumn ) ) {
      newColumn( KrDetailedViewProperties::DateTime );
      setColumnWidthMode( COLUMN(DateTime), Q3ListView::Manual );
      //setColumnWidth( column( DateTime ), QFontMetrics( font() ).width( "99/99/99  99:99" ) );
      setColumnWidth( COLUMN(DateTime), QFontMetrics( font() ).width( KGlobal::locale() ->formatDateTime(
                         QDateTime ( QDate( 2099, 12, 29 ), QTime( 23, 59 ) ) ) ) + 3 );
   }
   if ( group.readEntry( "Perm Column", _PermColumn ) ) {
      newColumn( KrDetailedViewProperties::Permissions );
      setColumnWidthMode( COLUMN(Permissions), Q3ListView::Manual );
      setColumnWidth( COLUMN(Permissions), QFontMetrics( font() ).width( "drwxrwxrwx" ) );
   }
   if ( group.readEntry( "KrPerm Column", _KrPermColumn ) ) {
      newColumn( KrDetailedViewProperties::KrPermissions );
      setColumnWidthMode( COLUMN(KrPermissions), Q3ListView::Manual );
      setColumnWidth( COLUMN(KrPermissions), QFontMetrics( font() ).width( "RWX" ) );
   }
   if ( group.readEntry( "Owner Column", _OwnerColumn ) ) {
      newColumn( KrDetailedViewProperties::Owner );
      setColumnWidthMode( COLUMN(Owner), Q3ListView::Manual );
      setColumnWidth( COLUMN(Owner), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   if ( group.readEntry( "Group Column", _GroupColumn ) ) {
      newColumn( KrDetailedViewProperties::Group );
      setColumnWidthMode( COLUMN(Group), Q3ListView::Manual );
      setColumnWidth( COLUMN(Group), QFontMetrics( font() ).width( 'X' ) * 6 );
   }
   
   // determine basic settings for the listview
   setAcceptDrops( true );
   setDragEnabled( true );
   setTooltipColumn( COLUMN(Name) );
   setDropVisualizer(false);
   setDropHighlighter(true);
   setSelectionModeExt( K3ListView::FileManager );
   setAllColumnsShowFocus( true );
   setShowSortIndicator( true );
   header() ->setStretchEnabled( true, COLUMN(Name) );

   //---- don't enable these lines, as it causes an ugly bug with inplace renaming
   //-->  setItemsRenameable( true );
   //-->  setRenameable( column( Name ), true );
   //-------------------------------------------------------------------------------

   header()->installEventFilter( this );
   renameLineEdit()->installEventFilter( this );
   
   // allow in-place renaming
   connect( renameLineEdit(), SIGNAL( done( Q3ListViewItem *, int ) ),
            this, SLOT( inplaceRenameFinished( Q3ListViewItem*, int ) ) );
   connect( &renameTimer, SIGNAL( timeout() ), this, SLOT( renameCurrentItem() ) );
   connect( &contextMenuTimer, SIGNAL (timeout()), this, SLOT (showContextMenu()));

	connect( header(), SIGNAL(clicked(int)), this, SLOT(slotSortOrderChanged(int )));

   setFocusPolicy( Qt::StrongFocus );
   restoreSettings();
   refreshColors();

   CANCEL_TWO_CLICK_RENAME;

   // connect quicksearch
   connect( op(), SIGNAL( quickSearch( const QString& ) ),
     this, SLOT( quickSearch( const QString& ) ) );
   connect( op(), SIGNAL( quickSearch( const QString& , int ) ),
     this, SLOT( quickSearch( const QString& , int ) ) );
   connect( op(), SIGNAL( stopQuickSearch( QKeyEvent* ) ),
     this, SLOT( stopQuickSearch( QKeyEvent* ) ) );
   connect( op(), SIGNAL( handleQuickSearchEvent( QKeyEvent* ) ),
     this, SLOT( handleQuickSearchEvent( QKeyEvent* ) ) );
}

KrDetailedView::~KrDetailedView() {
	delete _properties; _properties = 0;
	delete _operator; _operator = 0;
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

// if vfile passes the filter, create an item, otherwise, drop it
KrViewItem *KrDetailedView::preAddItem( vfile *vf ) {
   QString size = KRpermHandler::parseSize( vf->vfile_getSize() );
   QString name = vf->vfile_getName();
   bool isDir = vf->vfile_isDir();
   if ( !isDir || ( isDir && ( _properties->filter & KrViewProperties::ApplyToDirs ) ) ) {
      switch ( _properties->filter ) {
            case KrViewProperties::All :
               break;
            case KrViewProperties::Custom :
            if ( !_properties->filterMask.match( vf ) ) return 0;
            break;
            case KrViewProperties::Dirs:
            if ( !vf->vfile_isDir() ) return 0;
            break;
            case KrViewProperties::Files:
            if ( vf->vfile_isDir() ) return 0;
            break;
            case KrViewProperties::ApplyToDirs :
            break; // no-op, stop compiler complaints
      }
   }
   // passed the filter ...
	return new KrDetailedViewItem( this, lastItem(), vf );
}

bool KrDetailedView::preDelItem(KrViewItem *item) {
   /* KDE HACK START - the renaming item is not disappeared after delete */
   /* solution: we send an ESC key event to terminate the rename */
   if( item ) {
      Q3ListViewItem * viewItem = dynamic_cast<Q3ListViewItem*>( item );
      if( viewItem == currentlyRenamedItem ) {
         currentlyRenamedItem = 0;
         QKeyEvent escEvent( QEvent::KeyPress, Qt::Key_Escape, 27, 0 );
         QApplication::sendEvent( renameLineEdit(), &escEvent );
      }

      // HACK: there's a bug in Q3ListViewItem, which causes crash at rename
      // here we remove the item from the list to avoid crash
      takeItem( viewItem );
   }
   /* KDE HACK END */
   return true;
}

void KrDetailedView::addItems( vfs *v, bool addUpDir ) {
   Q3ListViewItem * item = firstChild();
   Q3ListViewItem *currentItem = item;
   QString size, name;

   // add the up-dir arrow if needed
   if ( addUpDir ) {
      new KrDetailedViewItem( this, ( Q3ListViewItem* ) 0L, ( vfile* ) 0L );
   }

   // text for updating the status bar
   QString statusText = QString("%1/  ").arg( v->vfs_getOrigin().fileName() ) + i18n("Directory");

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
               if ( !_properties->filterMask.match( vf ) )
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

      KrDetailedViewItem *dvitem = new KrDetailedViewItem( this, item, vf );
      _dict.insert( vf->vfile_getName(), dvitem );
      if ( isDir )
         ++_numDirs;
      else
         _countSize += dvitem->VF->vfile_getSize();
      ++_count;
      // if the item should be current - make it so
      if ( dvitem->name() == nameToMakeCurrent() )
      {
         currentItem = static_cast<Q3ListViewItem*>(dvitem);
         statusText = dvitem->description();
      }

      cnt++;
   }


   // re-enable sorting
   setSorting( cl, as );
   sort();

   if ( !currentItem )
      currentItem = firstChild();
   K3ListView::setCurrentItem( currentItem );
   ensureItemVisible( currentItem );

   op()->emitItemDescription( statusText );
}

QString KrDetailedView::getCurrentItem() const {
   Q3ListViewItem * it = currentItem();
   if ( !it )
      return QString();
   else
      return dynamic_cast<KrViewItem*>( it ) ->name();
}

void KrDetailedView::setCurrentItem( const QString& name ) {
   QHash<QString, KrViewItem*>::iterator itr = _dict.find( name );
   if ( itr != _dict.end() )
      K3ListView::setCurrentItem( dynamic_cast<KrDetailedViewItem*>( *itr ) );
}

void KrDetailedView::clear() {
   /* KDE HACK START - the renaming item is not disappeared after clear */
   /* solution: we send an ESC key event to terminate the rename */
   if( currentlyRenamedItem ) {
      currentlyRenamedItem = 0;
      QKeyEvent escEvent( QEvent::KeyPress, Qt::Key_Escape, 27, 0 );
      QApplication::sendEvent( renameLineEdit(), &escEvent );
   }
   /* KDE HACK END */

   op()->emitSelectionChanged(); /* to avoid rename crash at refresh */
   K3ListView::clear();
   KrView::clear();
}

void KrDetailedView::setSortMode( KrViewProperties::SortSpec mode ) {
   KrView::setSortMode(mode); // the KrViewItems will check it by themselves
   bool ascending = !( mode & KrViewProperties::Descending );
   int cl = -1;
   if ( mode & KrViewProperties::Name )
      cl = COLUMN( Name );
   else
	if ( mode & KrViewProperties::Ext )
      cl = COLUMN( Extention );
	else
      if ( mode & KrViewProperties::Size )
         cl = COLUMN( Size );
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
   K3ListView::sort();
}

void KrDetailedView::slotClicked( Q3ListViewItem *item ) {
   if ( !item ) return ;

   if ( !modifierPressed ) {
      if ( singleClicked && !renameTimer.isActive() ) {
         KSharedConfigPtr config = KGlobal::config();
         KConfigGroup group( krConfig, "KDE" );
         int doubleClickInterval = group.readEntry( "DoubleClickInterval", 400 );

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

void KrDetailedView::slotDoubleClicked( Q3ListViewItem *item ) {
   CANCEL_TWO_CLICK_RENAME;
   if ( !item )
      return ;
   QString tmp = dynamic_cast<KrViewItem*>( item ) ->name();
   op()->emitExecuted(tmp);
}

void KrDetailedView::prepareForActive() {
	KrView::prepareForActive();
   setFocus();
   slotItemDescription( currentItem() );
}

void KrDetailedView::prepareForPassive() {
	KrView::prepareForPassive();
   CANCEL_TWO_CLICK_RENAME;
   if ( renameLineEdit() ->isVisible() )
      renameLineEdit() ->clearFocus();
   KConfigGroup grpSvr( _config, "Look&Feel" );
   if ( grpSvr.readEntry( "New Style Quicksearch", _NewStyleQuicksearch ) ) {
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
}

void KrDetailedView::slotItemDescription( Q3ListViewItem * item ) {
   KrViewItem * it = static_cast<KrDetailedViewItem*>( item );
   if ( !it )
      return ;
   QString desc = it->description();
   op()->emitItemDescription(desc);
}

void KrDetailedView::handleQuickSearchEvent( QKeyEvent * e ) {
   switch ( e->key() ) {
         case Qt::Key_Insert:
         {
            QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Space, 0, 0 );
            K3ListView::keyPressEvent( & ev );
            ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Down, 0, 0 );
            keyPressEvent( & ev );
            break;
         }
         case Qt::Key_Home:
         {
            Q3ListView::setCurrentItem( firstChild() );
            QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Down, 0, 0 );
            keyPressEvent( & ev );
            break;
         }
         case Qt::Key_End:
         {
            Q3ListView::setCurrentItem( firstChild() );
            QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Up, 0, 0 );
            keyPressEvent( & ev );
            break;
         }
   }
}


void KrDetailedView::slotCurrentChanged( Q3ListViewItem * item ) {
   CANCEL_TWO_CLICK_RENAME;
   if ( !item )
      return ;
   _nameToMakeCurrent = static_cast<KrDetailedViewItem*>( item ) ->name();
}

void KrDetailedView::contentsMousePressEvent( QMouseEvent * e ) {
   bool callDefaultHandler = true, processEvent = true, selectionChanged = false;
   pressedItem = 0;

   Q3ListViewItem * oldCurrent = currentItem();
   Q3ListViewItem *newCurrent = itemAt( contentsToViewport( e->pos() ) );
   if (e->button() == Qt::RightButton)
   {
	if (KrSelectionMode::getSelectionHandler()->rightButtonSelects() || 
		(((e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::ControlModifier))) && KrSelectionMode::getSelectionHandler()->shiftCtrlRightButtonSelects())
     {
       if (KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() && !(e->modifiers() & Qt::ShiftModifier)
          && !(e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::AltModifier))
       {
         if (newCurrent)
         {
           if (KrSelectionMode::getSelectionHandler()->showContextMenu() >= 0)
           {
             swushSelects = !newCurrent->isSelected();
             lastSwushPosition = newCurrent;
           }
           newCurrent->setSelected(!newCurrent->isSelected());
           newCurrent->repaint();
			  selectionChanged = true;
         }
         callDefaultHandler = false;
         processEvent = false;
         e->accept();
       }

       if( !KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() && KrSelectionMode::getSelectionHandler()->showContextMenu() >= 0)
       {
         if( (e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::AltModifier) )
         {
            if( newCurrent )
            {
               newCurrent->setSelected(!newCurrent->isSelected());
               newCurrent->repaint();
               selectionChanged = true;
               callDefaultHandler = false;
               e->accept();
            }
         }
         else if( !(e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::AltModifier) )
         {
            clearSelection();
            if( newCurrent )
            {
               newCurrent->setSelected( true );
               newCurrent->repaint();
            }
            selectionChanged = true;
            callDefaultHandler = false;
            e->accept();
         }
       }
     }
     else
     {
       callDefaultHandler = false;
       processEvent = false;
       e->accept();
     }
   }
   if (e->button() == Qt::LeftButton)
   {
     dragStartPos = e->pos();
	  if (KrSelectionMode::getSelectionHandler()->leftButtonSelects() || 
	  		(((e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::ControlModifier))) &&
			KrSelectionMode::getSelectionHandler()->shiftCtrlLeftButtonSelects())
     {
       if (KrSelectionMode::getSelectionHandler()->leftButtonPreservesSelection() && !(e->modifiers() & Qt::ShiftModifier)
          && !(e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::AltModifier))
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
   if ( (e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::AltModifier) ) {
      CANCEL_TWO_CLICK_RENAME;
      modifierPressed = true;
   }

   // stop quick search in case a mouse click occured
   KConfigGroup grpSvr( _config, "Look&Feel" );
    if ( grpSvr.readEntry( "New Style Quicksearch", _NewStyleQuicksearch ) ) {
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
   	op()->emitNeedFocus();
   if (processEvent && ( (e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::ControlModifier) || (e->modifiers() & Qt::AltModifier) ) && !KrSelectionMode::getSelectionHandler()->useQTSelection()){
      if ( oldCurrent && newCurrent && oldCurrent != newCurrent && e->modifiers() & Qt::ShiftModifier ) {
         int oldPos = oldCurrent->itemPos();
         int newPos = newCurrent->itemPos();
         Q3ListViewItem *top = 0, *bottom = 0;
         if ( oldPos > newPos ) {
            top = newCurrent;
            bottom = oldCurrent;
         } else {
            top = oldCurrent;
            bottom = newCurrent;
         }
         Q3ListViewItemIterator it( top );
         for ( ; it.current(); ++it ) {
            if ( !it.current() ->isSelected() ) {
               it.current() ->setSelected( true );
               selectionChanged = true;
            }
            if ( it.current() == bottom )
               break;
         }
         Q3ListView::setCurrentItem( newCurrent );
         callDefaultHandler = false;
      }
   }
	
	if (selectionChanged)
		updateView(); // don't call triggerUpdate directly!
	
   //   QListViewItem * i = itemAt( contentsToViewport( e->pos() ) );
   if (callDefaultHandler)
   {
     dragStartPos = QPoint( -1, -1 );

     QString name = QString();    // will the file be deleted by the mouse event?
     if( newCurrent )                 // save the name of the file
       name = static_cast<KrDetailedViewItem*>( newCurrent ) ->name();

     K3ListView::contentsMousePressEvent( e );

     if( name.isEmpty() || _dict.find( name ) == _dict.end() ) // is the file still valid?
       newCurrent = 0;                // if not, don't do any crash...
   } else {
     // emitting the missing signals from QListView::contentsMousePressEvent();
     // the right click signal is not emitted as it is used for selection

     QPoint vp = contentsToViewport( e->pos() );

     if( !newCurrent || ( newCurrent && newCurrent->isEnabled() ) ) {
       emit pressed( pressedItem = newCurrent );
       emit pressed( newCurrent, viewport()->mapToGlobal( vp ), 0 );
     }

     emit mouseButtonPressed( e->button(), newCurrent, viewport()->mapToGlobal( vp ), 0 );
   }

   //   if (i != 0) // comment in, if click sould NOT select
   //     setSelected(i, FALSE);
   if (newCurrent) Q3ListView::setCurrentItem(newCurrent);

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
  if (e->button() == Qt::RightButton)
    contextMenuTimer.stop();
  K3ListView::contentsMouseReleaseEvent( e );

  if( pressedItem ) {
    QPoint vp = contentsToViewport( e->pos() );
    Q3ListViewItem *newCurrent = itemAt( vp );

    if( pressedItem == newCurrent ) {
      // emitting the missing signals from QListView::contentsMouseReleaseEvent();
      // the right click signal is not emitted as it is used for selection

      if( !newCurrent || ( newCurrent && newCurrent->isEnabled() ) ) {
        emit clicked( newCurrent );
        emit clicked( newCurrent, viewport()->mapToGlobal( vp ), 0 );
      }

      emit mouseButtonClicked( e->button(), newCurrent, viewport()->mapToGlobal( vp ), 0 );
    }

    pressedItem = 0;
  }
}

void KrDetailedView::contentsMouseMoveEvent ( QMouseEvent * e ) {
   if ( ( singleClicked || renameTimer.isActive() ) && itemAt( contentsToViewport( e->pos() ) ) != clickedItem )
      CANCEL_TWO_CLICK_RENAME;
   if ( dragStartPos != QPoint( -1, -1 ) &&
        e->buttons() & Qt::LeftButton && ( dragStartPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
      startDrag();
   if (KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() 
      && KrSelectionMode::getSelectionHandler()->rightButtonSelects() 
      && KrSelectionMode::getSelectionHandler()->showContextMenu() >= 0 && e->modifiers() == Qt::RightButton)
      {
         Q3ListViewItem *newItem = itemAt( contentsToViewport( e->pos() ) );
         e->accept();
         if (newItem != lastSwushPosition && newItem)
         {
           // is the new item above or below the previous one?
           Q3ListViewItem * above = newItem;
           Q3ListViewItem * below = newItem;
           for (;(above || below) && above != lastSwushPosition && below != lastSwushPosition;)
           {
             if (above)
               above = above->itemAbove();
             if (below)
               below = below->itemBelow();
           }
           if (above && (above == lastSwushPosition))
           {
             for (; above != newItem; above = above->itemBelow())
               above->setSelected(swushSelects);
             newItem->setSelected(swushSelects);
             lastSwushPosition = newItem;
             updateView();
           }
           else if (below && (below == lastSwushPosition))
           {
             for (; below != newItem; below = below->itemAbove())
               below->setSelected(swushSelects);
             newItem->setSelected(swushSelects);
             lastSwushPosition = newItem;
             updateView();
           }
           contextMenuTimer.stop();
         }
         // emitting the missing signals from QListView::contentsMouseMoveEvent();
         if( newItem )
           emit onItem( newItem );
         else
           emit onViewport();
      }
      else
         K3ListView::contentsMouseMoveEvent( e );
}

void KrDetailedView::contentsWheelEvent( QWheelEvent * e ) {
   if ( !_focused )
      op()->emitNeedFocus();
   K3ListView::contentsWheelEvent( e );
}

void KrDetailedView::handleContextMenu( Q3ListViewItem * it, const QPoint & pos, int ) {
   if ( !_focused )
      op()->emitNeedFocus();
   if ( !it )
      return ;
   if ( static_cast<KrDetailedViewItem*>( it ) ->
         name() == ".." )
      return ;
   int i = KrSelectionMode::getSelectionHandler()->showContextMenu();
   contextMenuPoint = QPoint( pos.x(), pos.y() - header() ->height() );
   if (i < 0)
     showContextMenu();
   else if (i > 0) {
     contextMenuTimer.setSingleShot( true );
     contextMenuTimer.start(i);
   }
}

void KrDetailedView::showContextMenu()
{
	if (lastSwushPosition)
		lastSwushPosition->setSelected(true);
	op()->emitContextMenu( contextMenuPoint );
}

KrViewItem *KrDetailedView::getKrViewItemAt( const QPoint & vp ) {
   return dynamic_cast<KrViewItem*>( K3ListView::itemAt( vp ) );
}

bool KrDetailedView::acceptDrag( QDropEvent* ) const {
   return true;
}

QRect KrDetailedView::drawItemHighlighter(QPainter *painter, Q3ListViewItem *item)
{
  QRect r;
  if( _currDragItem && item ) {
    r = itemRect(item);

    if (painter)
    {
//      style().drawPrimitive(QStyle::PE_FrameFocusRect, painter, r, colorGroup(),
//                            QStyle::State_FocusAtBorder, colorGroup().highlight());
      QStyleOptionFocusRect option;
      option.initFrom(this);
      option.backgroundColor = colorGroup().highlight();

      style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, this);
    }
  }
  return r;
}

void KrDetailedView::contentsDropEvent( QDropEvent * e ) {
   e->setPoint( contentsToViewport( e->pos() ) );
   op()->emitGotDrop(e);
   e->ignore();
   K3ListView::contentsDropEvent( e );
}

void KrDetailedView::contentsDragMoveEvent( QDragMoveEvent * e ) {
   _currDragItem = getKrViewItemAt(contentsToViewport(e->pos()));
   if( _currDragItem && !_currDragItem->VF->vfile_isDir() )
     _currDragItem = 0;
   
   K3ListView::contentsDragMoveEvent( e );
}

void KrDetailedView::inputMethodEvent(QInputMethodEvent * /* e */ ) {
	// TODO: the following 3 functions should somehow fit into this one. Csaba, did you implement this one?
}

#if 0
void KrDetailedView::imStartEvent(QIMEvent* e)
{
  if ( ACTIVE_PANEL->quickSearch->isShown() ) {
    ACTIVE_PANEL->quickSearch->myIMStartEvent( e );
    return ;
  }else {
    KConfigGroup grpSvr( _config, "Look&Feel" );
    if ( !grpSvr.readEntry( "New Style Quicksearch", _NewStyleQuicksearch ) )
      K3ListView::imStartEvent( e );
    else {
							// first, show the quicksearch if its hidden
      if ( ACTIVE_PANEL->quickSearch->isHidden() ) {
        ACTIVE_PANEL->quickSearch->show();
								// hack: if the pressed key requires a scroll down, the selected
								// item is "below" the quick search window, as the list view will
								// realize its new size after the key processing. The following line
								// will resize the list view immediately.
        ACTIVE_PANEL->layout->activate();
								// second, we need to disable the dirup action - hack!
        krDirUp->setEnabled( false );
      }
							// now, send the key to the quicksearch
      ACTIVE_PANEL->quickSearch->myIMStartEvent( e );
    }
  }
}

void KrDetailedView::imEndEvent(QIMEvent* e)
{
  if ( ACTIVE_PANEL->quickSearch->isShown() ) {
    ACTIVE_PANEL->quickSearch->myIMEndEvent( e );
    return ;
  }
}

void KrDetailedView::imComposeEvent(QIMEvent* e)
{
  if ( ACTIVE_PANEL->quickSearch->isShown() ) {
    ACTIVE_PANEL->quickSearch->myIMComposeEvent( e );
    return ;
  }
}

#endif

// TODO: for brief mode, move as much of this as possible to the viewOperator
void KrDetailedView::keyPressEvent( QKeyEvent * e ) {
   if ( !e || !firstChild() )
      return ; // subclass bug
   if ( ACTIVE_PANEL->quickSearch->isShown() ) {
      ACTIVE_PANEL->quickSearch->myKeyPressEvent( e );
      return ;
   }
   switch ( e->key() ) {
         case Qt::Key_Up :
         if ( e->modifiers() == Qt::ControlModifier ) { // let the panel handle it - jump to the Location Bar
            e->ignore();
            break;
         } else if (!KrSelectionMode::getSelectionHandler()->useQTSelection()) {
            Q3ListViewItem * i = currentItem();
            if ( !i ) break;
            if ( e->modifiers() == Qt::ShiftModifier ) setSelected( i, !i->isSelected() );
            i = i->itemAbove();
         	if ( i ) {
					Q3ListView::setCurrentItem( i );
					Q3ListView::ensureItemVisible( i );
				}
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_Down :
         if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == ( Qt::ControlModifier | Qt::ShiftModifier ) ) { // let the panel handle it - jump to command line
            e->ignore();
            break;
         } else if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            Q3ListViewItem * i = currentItem();
            if ( !i ) break;
            if ( e->modifiers() == Qt::ShiftModifier ) setSelected( i, !i->isSelected() );
            i = i->itemBelow();
         if ( i ) {Q3ListView::setCurrentItem( i ); Q3ListView::ensureItemVisible( i ); }
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_Next:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            Q3ListViewItem * i = currentItem(), *j;
            if ( !i ) break;
            QRect r( itemRect( i ) );
            if ( !r.height() ) break;
            for ( int page = visibleHeight() / r.height() - 1; page > 0 && ( j = i->itemBelow() ); --page )
               i = j;
            if ( i ) {Q3ListView::setCurrentItem( i ); Q3ListView::ensureItemVisible( i ); }
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_Prior:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            Q3ListViewItem * i = currentItem(), *j;
            if ( !i ) break;
            QRect r( itemRect( i ) );
            if ( !r.height() ) break;
            for ( int page = visibleHeight() / r.height() - 1; page > 0 && ( j = i->itemAbove() ); --page )
               i = j;
            if ( i ) {Q3ListView::setCurrentItem( i ); Q3ListView::ensureItemVisible( i ); }
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_Home:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            if ( e->modifiers() & Qt::ShiftModifier )  /* Shift+Home */
            {
               clearSelection();
               K3ListView::keyPressEvent( e );
               op()->emitSelectionChanged();
               triggerUpdate();
               break;
            } else {
               Q3ListViewItem * i = firstChild();
               if ( i ) {Q3ListView::setCurrentItem( i ); Q3ListView::ensureItemVisible( i ); }
            }
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_End:  if (!KrSelectionMode::getSelectionHandler()->useQTSelection()){
            if ( e->modifiers() & Qt::ShiftModifier )  /* Shift+End */
            {
               clearSelection();
               K3ListView::keyPressEvent( e );
               op()->emitSelectionChanged();
               triggerUpdate();
               break;
            } else {
               Q3ListViewItem *i = firstChild(), *j;
               while ( ( j = i->nextSibling() ) )
                  i = j;
               while ( ( j = i->itemBelow() ) )
                  i = j;
            if ( i ) {Q3ListView::setCurrentItem( i ); Q3ListView::ensureItemVisible( i ); }
               break;
            }
         } else K3ListView::keyPressEvent(e);
         break;
         case Qt::Key_Enter :
         case Qt::Key_Return : {
            if ( e->modifiers() & Qt::ControlModifier )         // let the panel handle it
               e->ignore();
            else {
               KrViewItem * i = getCurrentKrViewItem();
               QString tmp = i->name();
               op()->emitExecuted(tmp);
            }
            break;
         }
         case Qt::Key_QuoteLeft :          // Terminal Emulator bugfix
         if ( e->modifiers() == Qt::ControlModifier ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->home(); // ask krusader to move up a directory
            return ;         // safety
         }
         break;
         case Qt::Key_Right :
         if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ) { // let the panel handle it
            e->ignore();
            break;
         } else { // just a normal click - do a lynx-like moving thing
            KrViewItem *i = getCurrentKrViewItem();
            if ( i->name() == ".." ) { // if clicking on the ".." entry
               SLOTS->dirUp(); // ask krusader to move up a directory
               return ;
            }
            if ( i->VF->vfile_isDir() ) {             // we create a return-pressed event,
               QString tmp = i->name();
               op()->emitExecuted(tmp); // thereby emulating a chdir
            } else if( i->VF->vfile_getUrl().isLocalFile() ) {
               bool encrypted; 
               KUrl url = i->VF->vfile_getUrl();
               QString mime = ((vfile *)(i->VF))->vfile_getMime();
               QString type = KRarcHandler::getType( encrypted, url.path(), mime, false );

               if( KRarcHandler::arcSupported( type ) ) {
                 KUrl url = i->VF->vfile_getUrl();
                 if( type == "-tar" || type == "-tgz" || type == "-tbz" )
                   url.setProtocol( "tar" );
                 else
                   url.setProtocol( "krarc" );
                 ACTIVE_FUNC->openUrl( url );
               }
            }
            return ; // safety
         }
         case Qt::Key_Backspace :                         // Terminal Emulator bugfix
         case Qt::Key_Left :
         if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ) { // let the panel handle it
            e->ignore();
            break;
         } else {          // a normal click - do a lynx-like moving thing
            SLOTS->dirUp(); // ask krusader to move up a directory
            return ;         // safety
         }
         //case Qt::Key_Up :
         //K3ListView::keyPressEvent( e );
         //break;
/*#ifndef _newSelectionHandling
         case Qt::Key_Down :
         if ( e->modifiers() == Qt::ControlModifier ) { // let the panel handle it
            e->ignore();
            break;
         } else
            K3ListView::keyPressEvent( e );
         break;
#endif*/
         case Qt::Key_Delete :                   // kill file
         SLOTS->deleteFiles( e->modifiers() == Qt::ShiftModifier || e->modifiers() == Qt::ControlModifier );
				
         break ;
         case Qt::Key_Insert : {
            if (KrSelectionMode::getSelectionHandler()->insertMovesDown())
               K3ListView::keyPressEvent( e );
            else
            {
               QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Space, 0, 0 );
               K3ListView::keyPressEvent( & ev );
            }
            break ; 
         }
         case Qt::Key_Space : {
            KrDetailedViewItem * viewItem = static_cast<KrDetailedViewItem *> ( getCurrentKrViewItem() );
            if ( !viewItem || viewItem->name() == ".." ) { // wrong type, just mark(unmark it)
               if (KrSelectionMode::getSelectionHandler()->spaceMovesDown())
               {
                  QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Insert, 0, 0 );
                  K3ListView::keyPressEvent( & ev );
               }
               else
                  K3ListView::keyPressEvent( e );
               break ; 
            }
            if ( viewItem->VF->vfile_isDir() && viewItem->VF->vfile_getSize() <= 0 && 
					KrSelectionMode::getSelectionHandler()->spaceCalculatesDiskSpace()) {
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
            {
               QKeyEvent ev = QKeyEvent( QKeyEvent::KeyPress, Qt::Key_Insert, 0, 0 );
               K3ListView::keyPressEvent( & ev );
            }
            else
               K3ListView::keyPressEvent( e );
         }
         break;
         case Qt::Key_A :                 // mark all
         if ( e->modifiers() == Qt::ControlModifier ) {
            K3ListView::keyPressEvent( e );
            updateView();
            break;
         }
         default:
         if ( e->key() == Qt::Key_Escape ) {
            Q3ListView::keyPressEvent( e ); return ; // otherwise the selection gets lost??!??
         }
         // if the key is A..Z or 1..0 do quick search otherwise...
         if ( e->text().length() > 0 && e->text() [ 0 ].isPrint() )       // better choice. Otherwise non-ascii characters like  can not be the first character of a filename
            /*         if ( ( e->key() >= Qt::Key_A && e->key() <= Qt::Key_Z ) ||
                           ( e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9 ) ||
                           ( e->key() == Qt::Key_Backspace ) ||
                           ( e->key() == Qt::Key_Down ) ||
                           ( e->key() == Qt::Key_Period ) ) */{ 
            // are we doing quicksearch? if not, send keys to panel
            //if ( _config->readBoolEntry( "Do Quicksearch", _DoQuicksearch ) ) {
               // are we using krusader's classic quicksearch, or wincmd style?
               {
						KConfigGroup grpSvr( _config, "Look&Feel" );
						if ( !grpSvr.readEntry( "New Style Quicksearch", _NewStyleQuicksearch ) )
							K3ListView::keyPressEvent( e );
						else {
							// first, show the quicksearch if its hidden
							if ( ACTIVE_PANEL->quickSearch->isHidden() ) {
								ACTIVE_PANEL->quickSearch->show();
								// hack: if the pressed key requires a scroll down, the selected
								// item is "below" the quick search window, as the list view will
								// realize its new size after the key processing. The following line
								// will resize the list view immediately.
								ACTIVE_PANEL->layout->activate();
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
            K3ListView::keyPressEvent( e );
         }
   }
   // emit the new item description
   slotItemDescription( currentItem() ); // actually send the QListViewItem
}

// overridden to make sure EXTENTION won't be lost during rename
void KrDetailedView::rename( Q3ListViewItem * item, int c ) {
   // do we have an EXT column? if so, handle differently:
   // copy the contents of the EXT column over to the name
   if ( COLUMN( Extention ) != -1 ) {
      item->setText( COLUMN( Name ), static_cast<KrDetailedViewItem*>( item ) ->name() );
      item->setText( COLUMN( Extention ), QString() );
      repaintItem( item );
   }

   currentlyRenamedItem = item;
   renameLineEdit()->setBackgroundMode(Qt::FixedColor);

   QPalette pal = renameLineEdit()->palette();
   pal.setColor( renameLineEdit()->backgroundRole(), Qt::white);
   pal.setColor( renameLineEdit()->foregroundRole(), Qt::black);
   renameLineEdit()->setPalette( pal );

   K3ListView::rename( item, c );
   renameLineEdit() ->selectAll();
}

void KrDetailedView::renameCurrentItem() {
   int c;
   QString newName, fileName;

	// handle inplace renaming, if possible
	
   KrDetailedViewItem *it = static_cast<KrDetailedViewItem*>(getCurrentKrViewItem());
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
      rename( static_cast<Q3ListViewItem*>( it ), c );
      // if applicable, select only the name without extension
      KConfigGroup svr( krConfig, "Look&Feel" );
      if (!svr.readEntry("Rename Selects Extension", true)) {
	if (it->hasExtension() && !it->VF->vfile_isDir() ) 
		renameLineEdit()->setSelection(0, it->name().lastIndexOf(it->extension())-1);
      }
      // signal will be emited when renaming is done, and finalization
      // will occur in inplaceRenameFinished()
   } else {
   	// do this in case inplace renaming is disabled
   	// this actually does the good old dialog box rename
   	KrView::renameCurrentItem();
   }
}

void KrDetailedView::inplaceRenameFinished( Q3ListViewItem * it, int ) {
   if( currentlyRenamedItem == 0 )
      return;

   if ( !it ) { // major failure - call developers
      krOut << "Major failure at inplaceRenameFinished(): item is null" << endl;
      return;
   }
   
   if( COLUMN( Extention ) != -1 && !currentlyRenamedItem )
     return; /* does the event filter restored the original state? */
   
   // check if the item was indeed renamed
   bool restoreView = false;
   if ( it->text( COLUMN( Name ) ) != static_cast<KrDetailedViewItem*>( it ) ->name() ) { // was renamed
      op()->emitRenameItem( static_cast<KrDetailedViewItem*>( it ) ->name(), it->text( COLUMN( Name ) ) );
   } else restoreView = true;

   // restore the view always! if the file was indeed renamed, we'll get a signal from the vfs about
   // it, and update the view when needed
#if 0
   if ( COLUMN( Extention ) != -1 && restoreView ) { // nothing happened, restore the view (if needed)
#endif
      
      QString ext, name = static_cast<KrDetailedViewItem*>( it ) ->name();
      if ( !static_cast<KrDetailedViewItem*>( it ) ->VF->vfile_isDir() && COLUMN( Extention ) != -1 ) {
		ext = static_cast<KrDetailedViewItem*>( it ) ->extension();
		name = static_cast<KrDetailedViewItem*>( it ) ->name( false );
      }
      it->setText( COLUMN( Name ), name );
      it->setText( COLUMN( Extention ), ext );
      repaintItem( it );
#if 0
   }
#endif

   setFocus();
   
   currentlyRenamedItem = 0;
}

// TODO: move the whole quicksearch mess out of here and into krview
void KrDetailedView::quickSearch( const QString & str, int direction ) {
   KrViewItem * item = getCurrentKrViewItem();
   if (!item)
      return;
   KConfigGroup grpSvr( _config, "Look&Feel" );
   bool caseSensitive = grpSvr.readEntry( "Case Sensitive Quicksearch", _CaseSensitiveQuicksearch );
   if ( !direction ) {
      if ( caseSensitive ? item->name().startsWith( str ) : item->name().toLower().startsWith( str.toLower() ) )
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
      if ( caseSensitive ? item->name().startsWith( str ) : item->name().toLower().startsWith( str.toLower() ) ) {
			setCurrentItem( item->name() );
			makeItemVisible( item );
         return ;
      }
   }
}

void KrDetailedView::stopQuickSearch( QKeyEvent * e ) {
   if( ACTIVE_PANEL && ACTIVE_PANEL->quickSearch ) {
     ACTIVE_PANEL->quickSearch->hide();
     ACTIVE_PANEL->quickSearch->clear();
     krDirUp->setEnabled( true );
     if ( e )
        keyPressEvent( e );
   }
}

// internal: converts signal from qlistview to krview
void KrDetailedView::setNameToMakeCurrent( Q3ListViewItem * it ) {
	if (!it) return;
   KrView::setNameToMakeCurrent( static_cast<KrDetailedViewItem*>( it ) ->name() );
}

void KrDetailedView::slotMouseClicked( int button, Q3ListViewItem * item, const QPoint&, int ) {
   pressedItem = 0; // if the signals are emitted, don't emit twice at contentsMouseReleaseEvent
   if ( button == Qt::MidButton )
      op()->emitMiddleButtonClicked( dynamic_cast<KrViewItem *>( item ) );
}

void KrDetailedView::refreshColors() {
   KConfigGroup group( krConfig, "Colors" );
   bool kdeDefault = group.readEntry("KDE Default", _KDEDefaultColors);
   bool alternateBackgroundEnabled = group.readEntry("Enable Alternate Background", _AlternateBackground); 
   if ( !kdeDefault ) {
      // KDE default is not choosen: set the background color (as this paints the empty areas) and the alternate color
      bool isActive = hasFocus();
      if ( MAIN_VIEW && ACTIVE_PANEL && ACTIVE_PANEL->view )
         isActive = ( static_cast<KrView *>( this ) == ACTIVE_PANEL->view );
      KrColorGroup cg;
      KrColorCache::getColorCache().getColors(cg, KrColorItemType(KrColorItemType::File, false, isActive, false, false));
      setPaletteBackgroundColor( cg.background() );
      QPalette pal = palette();
      pal.setColor( QPalette::Base, cg.background() );
      setPalette( pal );

      KrColorCache::getColorCache().getColors(cg, KrColorItemType(KrColorItemType::File, true, isActive, false, false));
      setAlternateBackground( cg.background() );
   } else {
      // KDE default is choosen: set back the background color
      setPaletteBackgroundColor( KColorScheme(QPalette::Active, KColorScheme::View).background().color() );
      // Set the alternate color to its default or to an invalid color, to turn alternate the background off.
      setAlternateBackground( alternateBackgroundEnabled ? KColorScheme(QPalette::Active, KColorScheme::View).background(KColorScheme::AlternateBackground).color() : QColor() );
   }
   updateView();
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
   return K3ListView::event( e );
}

bool KrDetailedView::eventFilter( QObject * watched, QEvent * e )
{
  if( watched == renameLineEdit() )
  {
    if( currentlyRenamedItem && e->type() == QEvent::Hide )
    {
      /* checking if the currentlyRenamedItem pointer is valid (vfs_refresh can delete this item) */
      for( Q3ListViewItem *it = firstChild(); it; it = it->nextSibling() )
        if( it == currentlyRenamedItem )
        {
          if ( it->text( COLUMN( Name ) ) == dynamic_cast<KrDetailedViewItem*>( it ) ->name()  && COLUMN( Extention ) != -1 ) 
            inplaceRenameFinished( it, COLUMN( Name ) );
          break;
        }
    }
    return FALSE;
  }
  else if( watched == header() )
  {
    if( e->type() == QEvent::MouseButtonPress && ((QMouseEvent *)e )->button() == Qt::RightButton )
    {
      selectColumns();
      return TRUE;
    }
    return FALSE;
  }
  return K3ListView::eventFilter( watched, e );
}

void KrDetailedView::makeItemVisible( const KrViewItem *item ) {
//	qApp->processEvents();  // Please don't remove the comment. Causes crash if it is inserted!
	ensureItemVisible( static_cast<const KrDetailedViewItem*>( item ) ); 
}

void KrDetailedView::initOperator() {
	_operator = new KrViewOperator(this, this);
	// klistview emits selection changed, so chain them to operator
	connect(this, SIGNAL(selectionChanged()), _operator, SIGNAL(selectionChanged()));
}

void KrDetailedView::initProperties() {
	_properties = new KrDetailedViewProperties;
	KConfigGroup grpSvr( _config, "Look&Feel" );
	for (int i=0; i<KrDetailedViewProperties::MAX_COLUMNS;++i)
		PROPS->column[i]=-1;	
	PROPS->displayIcons = grpSvr.readEntry( "With Icons", _WithIcons );
	bool dirsByNameAlways = grpSvr.readEntry("Always sort dirs by name", false);
	PROPS->sortMode = static_cast<KrViewProperties::SortSpec>( KrViewProperties::Name |
			KrViewProperties::Descending | KrViewProperties::DirsFirst | 
			(dirsByNameAlways ? KrViewProperties::AlwaysSortDirsByName : 0) );
	PROPS->numericPermissions = grpSvr.readEntry("Numeric permissions", _NumericPermissions);
	if ( !grpSvr.readEntry( "Case Sensative Sort", _CaseSensativeSort ) )
      	PROPS->sortMode = static_cast<KrViewProperties::SortSpec>( _properties->sortMode |
				 KrViewProperties::IgnoreCase );
	PROPS->humanReadableSize = grpSvr.readEntry("Human Readable Size", _HumanReadableSize);
	PROPS->localeAwareCompareIsCaseSensitive = QString( "a" ).localeAwareCompare( "B" ) > 0; // see KDE bug #40131
	QStringList defaultAtomicExtensions;
	defaultAtomicExtensions += ".tar.gz";
	defaultAtomicExtensions += ".tar.bz2";
	defaultAtomicExtensions += ".moc.cpp";
	QStringList atomicExtensions = grpSvr.readEntry("Atomic Extensions", defaultAtomicExtensions);
	for (QStringList::iterator i = atomicExtensions.begin(); i != atomicExtensions.end(); )
	{
		QString & ext = *i;
		ext = ext.trimmed();
		if (!ext.length())
		{
			i = atomicExtensions.remove(i);
			continue;
		}
		if (!ext.startsWith("."))
			ext.insert(0, '.');
		++i;
	}
	PROPS->atomicExtensions = atomicExtensions;
}

void KrDetailedView::selectColumns()
{
  KMenu popup( this );
  popup.setTitle( i18n("Columns"));
  
  bool refresh = false;
  
  bool hasExtention = COLUMN( Extention ) != -1;
  bool hasMime      = COLUMN( Mime ) != -1;
  bool hasSize      = COLUMN( Size ) != -1;
  bool hasDate      = COLUMN( DateTime ) != -1;
  bool hasPerms     = COLUMN( Permissions ) != -1;
  bool hasKrPerms   = COLUMN( KrPermissions ) != -1;
  bool hasOwner     = COLUMN( Owner ) != -1;
  bool hasGroup     = COLUMN( Group ) != -1;
  
  QAction *extAct = popup.addAction( i18n( "Ext" ) );
  extAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Extention ) );
  extAct->setCheckable( true );
  extAct->setChecked( hasExtention );

  QAction *typeAct = popup.addAction( i18n( "Type" ) );
  typeAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Mime ) );
  typeAct->setCheckable( true );
  typeAct->setChecked( hasMime );

  QAction *sizeAct = popup.addAction( i18n( "Size" ) );
  sizeAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Size ) );
  sizeAct->setCheckable( true );
  sizeAct->setChecked( hasSize );

  QAction *modifAct = popup.addAction( i18n( "Modified" ) );
  modifAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::DateTime ) );
  modifAct->setCheckable( true );
  modifAct->setChecked( hasDate );

  QAction *permAct = popup.addAction( i18n( "Perms" ) );
  permAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Permissions ) );
  permAct->setCheckable( true );
  permAct->setChecked( hasPerms );

  QAction *rwxAct = popup.addAction( i18n( "rwx" ) );
  rwxAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::KrPermissions ) );
  rwxAct->setCheckable( true );
  rwxAct->setChecked( hasKrPerms );

  QAction *ownerAct = popup.addAction( i18n( "Owner" ) );
  ownerAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Owner ) );
  ownerAct->setCheckable( true );
  ownerAct->setChecked( hasOwner );

  QAction *groupAct = popup.addAction( i18n( "Group" ) );
  groupAct->setData( QVariant( COLUMN_POPUP_IDS + KrDetailedViewProperties::Group ) );
  groupAct->setCheckable( true );
  groupAct->setChecked( hasGroup );
  
  QAction *res = popup.exec(QCursor::pos());
  int result= -1;
  if( res->data().canConvert<int>() )
    result = res->data().toInt();

  KConfigGroup group( krConfig, nameInKConfig() );
  
  switch( result - COLUMN_POPUP_IDS )
  {
  case KrDetailedViewProperties::Extention:
    group.writeEntry( "Ext Column", !hasExtention );
    refresh = true;
    break;
  case KrDetailedViewProperties::Mime:
    group.writeEntry( "Mime Column", !hasMime );
    refresh = true;
    break;
  case KrDetailedViewProperties::Size:
    group.writeEntry( "Size Column", !hasSize );
    refresh = true;
    break;
  case KrDetailedViewProperties::DateTime:
    group.writeEntry( "DateTime Column", !hasDate );
    refresh = true;
    break;
  case KrDetailedViewProperties::Permissions:
    group.writeEntry( "Perm Column", !hasPerms );
    refresh = true;
    break;
  case KrDetailedViewProperties::KrPermissions:
    group.writeEntry( "KrPerm Column", !hasKrPerms );
    refresh = true;
    break;
  case KrDetailedViewProperties::Owner:
    group.writeEntry( "Owner Column", !hasOwner );
    refresh = true;
    break;
  case KrDetailedViewProperties::Group:
    group.writeEntry( "Group Column", !hasGroup );
    refresh = true;
    break;
  }
  
  if( refresh )
  {
	 PanelManager *p = ACTIVE_PANEL->view == this ? ACTIVE_MNG : OTHER_MNG;
    QTimer::singleShot( 0, p, SLOT( slotRecreatePanels() ) );
  }
}

void KrDetailedView::sortOrderChanged(int) {
	ensureItemVisible(currentItem());
}

void KrDetailedView::updateView() {
	triggerUpdate(); 
	op()->emitSelectionChanged();
}

void KrDetailedView::updateItem(KrViewItem* item) {
	dynamic_cast<KrDetailedViewItem*>(item)->repaintItem();
}

void KrDetailedView::slotRightButtonPressed(Q3ListViewItem*, const QPoint& point, int) {
	op()->emitEmptyContextMenu(point);
}

// hack: this needs to be done in a more cross-view way
void KrDetailedView::slotSortOrderChanged(int col) {
	// map the column to a sort specification
	KrViewProperties::SortSpec sp = KrViewProperties::Name;
	int i;
	for (i = 0; i < KrDetailedViewProperties::MAX_COLUMNS; ++i) {
		if (PROPS->column[i] == col) break;
	}
	switch (i) {
		case KrDetailedViewProperties::Name:
			sp = KrViewProperties::Name; break;
		case KrDetailedViewProperties::Extention:
			sp = KrViewProperties::Ext; break;
		case KrDetailedViewProperties::Mime:
			sp = KrViewProperties::Type; break;
		case KrDetailedViewProperties::Size:
			sp = KrViewProperties::Size; break;
		case KrDetailedViewProperties::DateTime:
			sp = KrViewProperties::Modified; break;
		case KrDetailedViewProperties::Permissions:
			sp = KrViewProperties::Permissions; break;
		case KrDetailedViewProperties::KrPermissions:
			sp = KrViewProperties::KrPermissions; break;
		case KrDetailedViewProperties::Owner:
			sp = KrViewProperties::Owner; break;
		case KrDetailedViewProperties::Group:
			sp = KrViewProperties::Group; break;
		default: qFatal("slotSortOrderChanged: unknown column");
	}
	if (sortMode() & KrViewProperties::DirsFirst) 
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::DirsFirst);
	if (sortMode() & KrViewProperties::IgnoreCase)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::IgnoreCase);
	if (sortMode() & KrViewProperties::Descending)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::Descending);
	if (sortMode() & KrViewProperties::AlwaysSortDirsByName)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::AlwaysSortDirsByName);
	// fix the ascending/decending stuff
	if (sortMode() == sp) {
		if (sp & KrViewProperties::Descending) 
			sp = static_cast<KrViewProperties::SortSpec>(sp &~ KrViewProperties::Descending);
		else sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::Descending);
	}
	PROPS->sortMode = sp;
	
	if( !_focused )
		op()->emitNeedFocus();
}

#include "krdetailedview.moc"
