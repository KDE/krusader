/***************************************************************************
                        dulistview.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "dulistview.h"
#include "../krusader.h"
#include "../kicons.h"
#include "../VFS/krpermhandler.h"
#include <qfontmetrics.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>
#include <qheader.h>
#include <time.h>

DUListView::DUListView( DiskUsage *usage, QWidget *parent, const char *name ) 
    : QListView( parent, name ), diskUsage( usage )
{  
  setAllColumnsShowFocus(true);
  setVScrollBarMode(QScrollView::Auto);
  setHScrollBarMode(QScrollView::Auto);
  setShowSortIndicator(true);
  setRootIsDecorated( true );
  setTreeStepSize( 10 );

  int defaultSize = QFontMetrics(font()).width("W");
  
  addColumn( i18n("Name"), defaultSize * 20 );
  setColumnWidthMode(0,QListView::Manual);
  addColumn( i18n("Percent"), defaultSize * 5 );
  setColumnWidthMode(1,QListView::Manual);
  addColumn( i18n("Total size"), defaultSize * 10 );
  setColumnWidthMode(1,QListView::Manual);
  addColumn( i18n("Own size"), defaultSize * 10 );
  setColumnWidthMode(2,QListView::Manual);
  addColumn( i18n("Type"), defaultSize * 10 );
  setColumnWidthMode(3,QListView::Manual);
  addColumn( i18n("Date"), defaultSize * 10 );
  setColumnWidthMode(4,QListView::Manual);
  addColumn( i18n("Permissions"), defaultSize * 6 );
  setColumnWidthMode(5,QListView::Manual);
  addColumn( i18n("Owner"), defaultSize * 5 );
  setColumnWidthMode(6,QListView::Manual);
  addColumn( i18n("Group"), defaultSize * 5 );
  setColumnWidthMode(7,QListView::Manual);
  
  setColumnAlignment( 1, Qt::AlignRight );
  setColumnAlignment( 2, Qt::AlignRight );
  setColumnAlignment( 3, Qt::AlignRight );
  
  setSorting( 2 );
  
  connect( diskUsage, SIGNAL( directoryChanged( QString ) ), this, SLOT( slotDirChanged( QString ) ) );
  connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
  connect( diskUsage, SIGNAL( changed( DiskUsageItem * ) ), this, SLOT( slotChanged( DiskUsageItem * ) ) );

  connect( this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
           this, SLOT( slotRightClicked(QListViewItem *) ) );
  connect( this, SIGNAL( expanded ( QListViewItem * ) ), 
           this, SLOT( slotExpanded( QListViewItem * ) ) ); 
}

void DUListView::addDirectory( QString dirName, QListViewItem *parent )
{
  QPtrList<DiskUsageItem> * currentDir = diskUsage->getDirectory( dirName );
  
  if( currentDir == 0 )
    return;

  QListViewItem * lastItem = 0;
    
  if( parent == 0 && !dirName.isEmpty() )
  {
    lastItem = new QListViewItem( this, ".." );
    lastItem->setPixmap( 0, FL_LOADICON( "up" ) );
    lastItem->setSelectable( false );
  }
          
  DiskUsageItem *item = currentDir->first();
  while( item )
  {
    KMimeType::Ptr mimePtr = KMimeType::mimeType( item->mime() );
    QString mime = mimePtr->comment();
       
    time_t tma = item->time();
    struct tm* t=localtime((time_t *)&tma);
    QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
    QString date = KGlobal::locale()->formatDateTime(tmp);    
    
    QString totalSize = KRpermHandler::parseSize( item->size() ) + " ";
    QString ownSize = KRpermHandler::parseSize( item->ownSize() ) + " ";
    QString percent = item->percent();
    
    if( lastItem == 0 && parent == 0 )
      lastItem = new DUListViewItem( diskUsage, item, this, item->name(), percent, totalSize, ownSize, 
                                     mime, date, item->perm(), item->owner(), item->group() );
    else if ( lastItem == 0 )
      lastItem = new DUListViewItem( diskUsage, item, parent, item->name(), percent, totalSize, ownSize, 
                                     mime, date, item->perm(), item->owner(), item->group() );
    else if ( parent == 0 )
      lastItem = new DUListViewItem( diskUsage, item, this, lastItem, item->name(), percent, totalSize,
                                     ownSize, mime, date, item->perm(), item->owner(), item->group() );
    else
      lastItem = new DUListViewItem( diskUsage, item, parent, lastItem, item->name(), percent, totalSize, 
                                     ownSize, mime, date, item->perm(), item->owner(), item->group() );
   
    if( item->isExcluded() )
      lastItem->setVisible( false );
                                    
    lastItem->setPixmap( 0, diskUsage->getIcon( item->mime() ) );
    
    if( item->isDir() && !item->isSymLink() )
      lastItem->setExpandable( true );
    
    item = currentDir->next();
  }
}

void DUListView::slotDirChanged( QString dirName )
{
  clear();  
  addDirectory( dirName, 0 );
}

void DUListView::slotChanged( DiskUsageItem * item )
{
  void * itemPtr = diskUsage->getProperty( item, "ListView-Ref" );
  if( itemPtr == 0 )
    return;
    
  DUListViewItem *duItem = (DUListViewItem *)itemPtr;
  duItem->setVisible( !item->isExcluded() );
  duItem->setText( 1, item->percent() );
  duItem->setText( 2, KRpermHandler::parseSize( item->size() ) + " " );
  duItem->setText( 3, KRpermHandler::parseSize( item->ownSize() ) + " " );
}
  
void DUListView::slotRightClicked( QListViewItem *item )
{
  if ( !item )
    return;
    
  if( item->text( 0 ) == ".." )
    return;

  diskUsage->rightClickMenu( ((DUListViewItem *)item)->getDiskUsageItem() );
}

bool DUListView::doubleClicked( QListViewItem * item )
{
  if( item )
  {
    if( item->text( 0 ) != ".." )
    {
      DiskUsageItem *duItem = ((DUListViewItem *)item)->getDiskUsageItem();
      if( duItem->isDir() && !duItem->isSymLink() )
        diskUsage->changeDirectory( ( duItem->directory().isEmpty() ? "" : duItem->directory() + "/" ) +
                                    duItem->name() );
      return true;
    }
    else
    {
      QString dir = diskUsage->getCurrentDir();
      int ndx = dir.findRev( "/" );
      if( ndx != -1 )
        dir.truncate( ndx );
      else
        dir = "";
      diskUsage->changeDirectory( dir );
      return true;
    }
  }
  return false;
}

void DUListView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  if ( e || e->button() == LeftButton )
  {
    QPoint vp = contentsToViewport(e->pos());
    QListViewItem * item = itemAt( vp );

    if( doubleClicked( item ) )
      return;
    
  }
  QListView::contentsMouseDoubleClickEvent( e );
}

void DUListView::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
  case Key_Return :
  case Key_Enter :
    if( doubleClicked( currentItem() ) )
      return;
    break;
  }
  QListView::keyPressEvent( e );
}

void DUListView::slotExpanded( QListViewItem *item )
{
  if( item == 0 || item->text( 0 ) == ".." )
    return;
 
  if( item->childCount() == 0 )
  {
    DiskUsageItem *duItem = ((DUListViewItem *)item)->getDiskUsageItem();
    addDirectory( ( duItem->directory().isEmpty() ? "" : duItem->directory() + "/" ) +
                  duItem->name(), item );   
  }
}

#include "dulistview.moc"
