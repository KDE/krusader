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
#include <kmimetype.h>
#include <klocale.h>

KrView::KrView( KConfig *cfg ) : _config( cfg ), _nameToMakeCurrent( QString::null ),
_numSelected( 0 ), _count( 0 ), _numDirs( 0 ), _countSize( 0 ), _selectedSize( 0 ), _properties(0) {}

KrView::~KrView() {
	if (_properties)
		qFatal("A class inheriting KrView didn't delete _properties!");
}

QPixmap KrView::getIcon( vfile *vf /*, KRListItem::cmpColor color*/ ) {
   //krConfig->setGroup("Advanced");
   //////////////////////////////
   QPixmap icon;
   QString mime = vf->vfile_getMime();
   //QPixmapCache::setCacheLimit( krConfig->readNumEntry("Icon Cache Size",_IconCacheSize) );

   // first try the cache
   if ( !QPixmapCache::find( mime, icon ) ) {
      // get the icon.
      if ( mime == "Broken Link !" )
         icon = FL_LOADICON( "file_broken" );
      else {
         icon = FL_LOADICON( KMimeType::mimeType( mime ) ->icon( QString::null, true ) );
      }
      // insert it into the cache
      QPixmapCache::insert( mime, icon );
   }
   // if it's a symlink - add an arrow overlay
   if ( vf->vfile_isSymLink() ) {
      QPixmap link( link_xpm );
      bitBlt ( &icon, 0, icon.height() - 11, &link, 0, 21, 10, 11, Qt::CopyROP, false );
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
      if ( it->isDir() && !dirs ) continue; // do we need to skip folders?
      if ( !it->isDir() && !files ) continue; // do we need to skip files
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
   if ( names->empty() && ( getCurrentItem() != ".." ) ) names->append( getCurrentItem() );
}

void KrView::getSelectedKrViewItems( KrViewItemList *items ) {
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) )
      if ( it->isSelected() && ( it->name() != ".." ) ) items->append( it );

   // if all else fails, take the current item
   if ( items->empty() && ( getCurrentItem() != ".." ) ) items->append( getCurrentKrViewItem() );
}

QString KrView::statistics() {
    _countSize = _numSelected = _selectedSize = 0;

    for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ){
      if ( it->isSelected() ) {
         ++_numSelected;
         _selectedSize += it->size();
      }
    if (it->size() > 0)
       _countSize += it->size();
   }
   QString tmp = QString(i18n("%1 out of %2, %3 (%4) out of %5 (%6)"))
                 .arg( _numSelected ).arg( _count ).arg( KIO::convertSize( _selectedSize ) )
                 .arg( KRpermHandler::parseSize(_selectedSize) )
					  .arg( KIO::convertSize( _countSize ) ).arg( KRpermHandler::parseSize(_countSize) );
   return tmp;
}

void KrView::changeSelection( const KRQuery& filter, bool select, bool includeDirs ) {
   KConfigGroupSaver grpSvr( _config, "Look&Feel" );
   bool markDirs = _config->readBoolEntry( "Mark Dirs", _MarkDirs ) || includeDirs;

   KrViewItem *temp = getCurrentKrViewItem();
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
      if ( it->name() == ".." ) continue;
      if ( it->isDir() && !markDirs ) continue;
      
      vfile * file = it->getVfile();
      if( file == 0 ) continue;
      
      if( filter.match( file ) ) {
         // we're increasing/decreasing the number of selected files
         if ( select ) {
            if ( !it->isSelected() ) {
               ++_numSelected;
               _selectedSize += it->size();
            }
         } else {
            if ( it->isSelected() ) {
               --_numSelected;
               _selectedSize -= it->size();
            }
         }
         it->setSelected( select );
      }
   }
   updateView();
   makeItemVisible( temp );
}

void KrView::invertSelection() {
   KConfigGroupSaver grpSvr( _config, "Look&Feel" /*nameInKConfig()*/ );
   bool markDirs = _config->readBoolEntry( "Mark Dirs", _MarkDirs );

   KrViewItem *temp = getCurrentKrViewItem();
   for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
      if ( it->name() == ".." ) continue;
      if ( it->isDir() && !markDirs && !it->isSelected() ) continue;
      if ( it->isSelected() ) {
         --_numSelected;
         _selectedSize -= it->size();
      } else {
         ++_numSelected;
         _selectedSize += it->size();
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
   if ( !iterator ) return QString::null;
   return iterator->name();
}
