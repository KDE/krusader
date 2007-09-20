/***************************************************************************
                                 krview.cpp
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
#include "krview.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../VFS/krpermhandler.h"
#include "krviewitem.h"
#include <qnamespace.h>
#include <qpixmapcache.h>
#include <qdir.h>
#include <qbitmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <kmimetype.h>
#include <klocale.h>
#include <kinputdialog.h>


#define VF	getVfile()


// ----------------------------- operator
KrViewOperator::KrViewOperator(KrView *view, QWidget *widget): _view(view), _widget(widget) {
}

KrViewOperator::~KrViewOperator() {
}

void KrViewOperator::startDrag() {
   QStringList items;
   _view->getSelectedItems( &items );
   if ( items.empty() )
      return ; // don't drag an empty thing
   QPixmap px;
   if ( items.count() > 1 )
      px = FL_LOADICON( "queue" ); // how much are we dragging
   else
      px = _view->getCurrentKrViewItem() ->icon();
   emit letsDrag( items, px );
}

// ----------------------------- krview

KrView::KrView( KConfig *cfg ) : _config( cfg ), _widget(0), _nameToMakeCurrent( QString() ), _nameToMakeCurrentIfAdded( QString() ),
_numSelected( 0 ), _count( 0 ), _numDirs( 0 ), _countSize( 0 ), _selectedSize( 0 ), _properties(0), _focused( false ), _nameInKConfig(QString()) {
}

KrView::~KrView() {
	if (_properties)
		qFatal("A class inheriting KrView didn't delete _properties!");
	if (_operator) 
		qFatal("A class inheriting KrView didn't delete _operator!");
}

void KrView::init() {
	// sanity checks:
	if (_nameInKConfig.isEmpty())
		qFatal("_nameInKConfig must be set during construction of KrView inheritors");
	if (!_widget)
		qFatal("_widget must be set during construction of KrView inheritors");
	// ok, continue
	initProperties();
	initOperator();
	setup();
}

QPixmap KrView::getIcon( vfile *vf /*, KRListItem::cmpColor color*/ ) {
   // KConfigGroup ag( krConfig, "Advanced");
   //////////////////////////////
   QPixmap icon;
   QString icon_name = vf->vfile_getIcon();
   //QPixmapCache::setCacheLimit( ag.readEntry("Icon Cache Size",_IconCacheSize) );

   if( icon_name.isNull() )
     icon_name="";
   
   // first try the cache
   if ( !QPixmapCache::find( icon_name, icon ) ) {
      icon = FL_LOADICON( icon_name );
      // insert it into the cache
      QPixmapCache::insert( icon_name, icon );
   }
   // if it's a symlink - add an arrow overlay
   if ( vf->vfile_isSymLink() ) {
      QPixmap link( link_xpm );
      // bitBlt ( &icon, 0, icon.height() - 11, &link, 0, 21, 10, 11, Qt::CopyROP, false );
      QPainter( &icon ).drawPixmap( 0, icon.height() - 11, link, 0, 21, 10, 11 );
      icon.setMask( icon.createHeuristicMask( false ) );
   }

   return icon;
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getItemsByMask( QString mask, QStringList* names, bool dirs, bool files ) {
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
      if ( ( it->name() == ".." ) || !QDir::match( mask, it->name() ) ) continue;
      // if we got here, than the item fits the mask
      if ( it->getVfile()->vfile_isDir() && !dirs ) continue; // do we need to skip folders?
      if ( !it->getVfile()->vfile_isDir() && !files ) continue; // do we need to skip files
      names->append( it->name() );
   }
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getSelectedItems( QStringList *names ) {
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) )
      if ( it->isSelected() && ( it->name() != ".." ) ) names->append( it->name() );

   // if all else fails, take the current item
	QString item = getCurrentItem();
   if ( names->empty() && item!=QString() && item!=".." ) names->append( item );
}

void KrView::getSelectedKrViewItems( KrViewItemList *items ) {
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) )
      if ( it->isSelected() && ( it->name() != ".." ) ) items->append( it );

   // if all else fails, take the current item
	QString item = getCurrentItem();
   if ( items->empty() && item!=QString() && item!=".." ) items->append( getCurrentKrViewItem() );
}

QString KrView::statistics() {
    _countSize = _numSelected = _selectedSize = 0;

    for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ){
      if ( it->isSelected() ) {
         ++_numSelected;
         _selectedSize += it->getVfile()->vfile_getSize();
      }
    if (it->getVfile()->vfile_getSize() > 0)
       _countSize += it->getVfile()->vfile_getSize();
   }
   QString tmp = QString(i18n("%1 out of %2, %3 (%4) out of %5 (%6)"))
                 .arg( _numSelected ).arg( _count ).arg( KIO::convertSize( _selectedSize ) )
                 .arg( KRpermHandler::parseSize(_selectedSize) )
					  .arg( KIO::convertSize( _countSize ) ).arg( KRpermHandler::parseSize(_countSize) );
	// notify if we're running a filtered view
	if (filter() != KrViewProperties::All)
		tmp = ">> [ " + filterMask().nameFilter() + " ]  "+tmp;
   return tmp;
}

void KrView::changeSelection( const KRQuery& filter, bool select, bool includeDirs ) {
   KConfigGroup grpSvr( _config, "Look&Feel" );
   bool markDirs = grpSvr.readEntry( "Mark Dirs", _MarkDirs ) || includeDirs;

   KrViewItem *temp = getCurrentKrViewItem();
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
      if ( it->name() == ".." ) continue;
      if ( it->getVfile()->vfile_isDir() && !markDirs ) continue;
      
      vfile * file = it->getMutableVfile(); // filter::match calls getMimetype which isn't const
      if( file == 0 ) continue;
      
      if( filter.match( file ) ) {
         // we're increasing/decreasing the number of selected files
         if ( select ) {
            if ( !it->isSelected() ) {
               ++_numSelected;
               _selectedSize += it->getVfile()->vfile_getSize();
            }
         } else {
            if ( it->isSelected() ) {
               --_numSelected;
               _selectedSize -= it->getVfile()->vfile_getSize();
            }
         }
         it->setSelected( select );
      }
   }
   updateView();
   makeItemVisible( temp );
}

void KrView::invertSelection() {
   KConfigGroup grpSvr( _config, "Look&Feel" );
   bool markDirs = grpSvr.readEntry( "Mark Dirs", _MarkDirs );

   KrViewItem *temp = getCurrentKrViewItem();
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
      if ( it->name() == ".." ) continue;
      if ( it->getVfile()->vfile_isDir() && !markDirs && !it->isSelected() ) continue;
      if ( it->isSelected() ) {
         --_numSelected;
         _selectedSize -= it->getVfile()->vfile_getSize();
      } else {
         ++_numSelected;
         _selectedSize += it->getVfile()->vfile_getSize();
      }
      it->setSelected( !it->isSelected() );
   }
   updateView();
   makeItemVisible( temp );
}

QString KrView::firstUnmarkedBelowCurrent() {
   KrViewItem * iterator = getNext( getCurrentKrViewItem() );
   while ( iterator && iterator->isSelected() )
      iterator = getNext( iterator );
   if ( !iterator ) {
      iterator = getPrev( getCurrentKrViewItem() );
      while ( iterator && iterator->isSelected() )
         iterator = getPrev( iterator );
   }
   if ( !iterator ) return QString();
   return iterator->name();
}

void KrView::delItem(const QString &name) {
	KrViewItem * it = _dict[ name ];
   if ( !it ) {
      krOut << "got signal deletedVfile(" << name << ") but can't find KrViewItem" << endl;
		return;
	}	
	if (!preDelItem(it)) return; // do not delete this after all
	
	// remove from dict
	if (it->VF->vfile_isDir()) {
		--_numDirs;
	} else {
		_countSize -= it->VF->vfile_getSize();
	}
	--_count;	
	_dict.remove( name );
	delete it;
	op()->emitSelectionChanged();
}

KrViewItem *KrView::addItem( vfile *vf ) {
	KrViewItem *item = preAddItem(vf);
	if (!item) return 0; // don't add it after all
	
	// add to dictionary
   _dict.insert( vf->vfile_getName(), item );
   if ( vf->vfile_isDir() )
      ++_numDirs;
   else _countSize += vf->vfile_getSize();
   ++_count;
   
	if (item->name() == nameToMakeCurrent() ) {
      setCurrentItem(item->name()); // dictionary based - quick
		makeItemVisible( item );
	}
   if (item->name() == nameToMakeCurrentIfAdded() ) {
		setCurrentItem(item->name());
		setNameToMakeCurrentIfAdded(QString());
		makeItemVisible( item );
	}
	

   op()->emitSelectionChanged();
	return item;
}

void KrView::updateItem(vfile *vf) {
   // since we're deleting the item, make sure we keep
   // it's properties first and repair it later
   KrViewItem * it = _dict[ vf->vfile_getName() ];
   if ( !it ) {
      krOut << "got signal updatedVfile(" << vf->vfile_getName() << ") but can't find KrViewItem" << endl;
   } else {
		bool selected = it->isSelected();
      bool current = ( getCurrentKrViewItem() == it );
      delItem( vf->vfile_getName() );
      KrViewItem *updatedItem = addItem( vf );
      // restore settings
      ( _dict[ vf->vfile_getName() ] ) ->setSelected( selected );
		if ( current ) {
         setCurrentItem( vf->vfile_getName() );
			makeItemVisible( updatedItem );
		}
   }
	op()->emitSelectionChanged();
}

void KrView::clear() {
   _count = _numSelected = _numDirs = _selectedSize = _countSize = 0;
   _dict.clear();
}

// good old dialog box
void KrView::renameCurrentItem() {
   QString newName, fileName;

   KrViewItem *it = getCurrentKrViewItem();
   if ( it ) fileName = it->name();
   else return ; // quit if no current item available
   
   // don't allow anyone to rename ..
   if ( fileName == ".." ) return ;

	bool ok = false;
	newName = KInputDialog::getText( i18n( "Rename" ), i18n( "Rename " ) + fileName + i18n( " to:" ),
												fileName, &ok, krApp );
	// if the user canceled - quit
	if ( !ok || newName == fileName )
		return ;
	op()->emitRenameItem(it->name(), newName);
}

