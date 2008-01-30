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
#include <QMouseEvent>
#include <QKeyEvent>
#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>
#include <qheaderview.h>
#include <time.h>

DUListView::DUListView( DiskUsage *usage ) 
    : KrTreeWidget( usage ), diskUsage( usage )
{  
  setAllColumnsShowFocus(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setRootIsDecorated( true );
  setIndentation( 10 );
  setItemsExpandable( true );

  QStringList labels;
  labels << i18n("Name");
  labels << i18n("Percent");
  labels << i18n("Total size");
  labels << i18n("Own size");
  labels << i18n("Type");
  labels << i18n("Date");
  labels << i18n("Permissions");
  labels << i18n("Owner");
  labels << i18n("Group");
  setHeaderLabels( labels );

  header()->setResizeMode( 0, QHeaderView::Interactive );
  header()->setResizeMode( 1, QHeaderView::Interactive );
  header()->setResizeMode( 2, QHeaderView::Interactive );
  header()->setResizeMode( 3, QHeaderView::Interactive );
  header()->setResizeMode( 4, QHeaderView::Interactive );
  header()->setResizeMode( 5, QHeaderView::Interactive );
  header()->setResizeMode( 6, QHeaderView::Interactive );
  header()->setResizeMode( 7, QHeaderView::Interactive );
  header()->setResizeMode( 8, QHeaderView::Interactive );

  KConfigGroup group( krConfig, diskUsage->getConfigGroup() ); 

  if( group.hasKey( "D State" ) )
    header()->restoreState( group.readEntry( "D State", QByteArray() ) );
  else
  {
    int defaultSize = QFontMetrics(font()).width("W");

    setColumnWidth(0, defaultSize * 20 );
    setColumnWidth(1, defaultSize * 5 );
    setColumnWidth(2, defaultSize * 10 );
    setColumnWidth(3, defaultSize * 10 );
    setColumnWidth(4, defaultSize * 10 );
    setColumnWidth(5, defaultSize * 10 );
    setColumnWidth(6, defaultSize * 6 );
    setColumnWidth(7, defaultSize * 5 );
    setColumnWidth(8, defaultSize * 5 );
  }

  header()->setSortIndicatorShown(true);
  sortItems( 2, Qt::AscendingOrder );

  connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
  connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
  connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
  connect( diskUsage, SIGNAL( deleted( File * ) ), this, SLOT( slotDeleted( File * ) ) );

  connect( this, SIGNAL( itemRightClicked ( QTreeWidgetItem*, int ) ),
           this, SLOT( slotRightClicked(QTreeWidgetItem *) ) );
  connect( this, SIGNAL( itemExpanded ( QTreeWidgetItem * ) ), 
           this, SLOT( slotExpanded( QTreeWidgetItem * ) ) ); 
}

DUListView::~ DUListView()
{
  KConfigGroup group( krConfig, diskUsage->getConfigGroup() ); 
  group.writeEntry( "D State", header()->saveState() );
}

void DUListView::addDirectory( Directory *dirEntry, QTreeWidgetItem *parent )
{
  QTreeWidgetItem * lastItem = 0;
    
  if( parent == 0 && ! ( dirEntry->parent() == 0 ) )
  {
    lastItem = new QTreeWidgetItem( this );
    lastItem->setText( 0, ".." );
    lastItem->setIcon( 0, FL_LOADICON( "up" ) );
    lastItem->setFlags( Qt::ItemIsEnabled );
  }
          
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File *item = *it;
    
    KMimeType::Ptr mimePtr = KMimeType::mimeType( item->mime() );
    QString mime;
    if( mimePtr )
      mime = mimePtr->comment();
       
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
      lastItem->setHidden( true );
                                    
    lastItem->setIcon( 0, diskUsage->getIcon( item->mime() ) );

    if( item->isDir() && !item->isSymLink() )
      lastItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
  }
  
  if( topLevelItemCount() > 0 )
  {
    setCurrentItem( topLevelItem( 0 ) );
  }
}

void DUListView::slotDirChanged( Directory *dirEntry )
{
  clear();  
  addDirectory( dirEntry, 0 );
}

File * DUListView::getCurrentFile()
{
  QTreeWidgetItem *item = currentItem();
  
  if( item == 0 || item->text( 0 ) == ".." )
    return 0;
  
  return ((DUListViewItem *)item)->getFile();
}

void DUListView::slotChanged( File * item )
{
  void * itemPtr = diskUsage->getProperty( item, "ListView-Ref" );
  if( itemPtr == 0 )
    return;
    
  DUListViewItem *duItem = (DUListViewItem *)itemPtr;
  duItem->setHidden( item->isExcluded() );
  duItem->setText( 1, item->percent() );
  duItem->setText( 2, KRpermHandler::parseSize( item->size() ) + " " );
  duItem->setText( 3, KRpermHandler::parseSize( item->ownSize() ) + " " );
}

void DUListView::slotDeleted( File * item )
{
  void * itemPtr = diskUsage->getProperty( item, "ListView-Ref" );
  if( itemPtr == 0 )
    return;
    
  DUListViewItem *duItem = (DUListViewItem *)itemPtr;
  delete duItem;
}
  
void DUListView::slotRightClicked( QTreeWidgetItem *item )
{
  File * file = 0;
  
  if ( item && item->text( 0 ) != ".." )
    file = ((DUListViewItem *)item)->getFile();

  diskUsage->rightClickMenu( file );
}

bool DUListView::doubleClicked( QTreeWidgetItem * item )
{
  if( item )
  {
    if( item->text( 0 ) != ".." )
    {
      File *fileItem = ((DUListViewItem *)item)->getFile();
      if( fileItem->isDir() )
        diskUsage->changeDirectory( dynamic_cast<Directory *> ( fileItem ) );
      return true;
    }
    else
    {
      Directory *upDir = (Directory *)diskUsage->getCurrentDir()->parent();
    
      if( upDir )
        diskUsage->changeDirectory( upDir );
      return true;
    }
  }
  return false;
}

void DUListView::mouseDoubleClickEvent ( QMouseEvent * e )
{
  if ( e || e->button() == Qt::LeftButton )
  {
    QPoint vp = viewport()->mapFromGlobal( e->globalPos());
    QTreeWidgetItem * item = itemAt( vp );

    if( doubleClicked( item ) )
      return;
    
  }
  KrTreeWidget::mouseDoubleClickEvent( e );
}

void DUListView::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
  case Qt::Key_Return :
  case Qt::Key_Enter :
    if( doubleClicked( currentItem() ) )
      return;
    break;
  case Qt::Key_Left :
  case Qt::Key_Right :
  case Qt::Key_Up :
  case Qt::Key_Down :
    if( e->modifiers() == Qt::ShiftModifier )
    {
      e->ignore();
      return;
    }
    break;
  case Qt::Key_Delete :
    e->ignore();
    return;
  }
  KrTreeWidget::keyPressEvent( e );
}

void DUListView::slotExpanded( QTreeWidgetItem * item )
{
  if( item == 0 || item->text( 0 ) == ".." )
    return;
 
  if( item->childCount() == 0 )
  {
    File *fileItem = ((DUListViewItem *)item)->getFile();
    if( fileItem->isDir() )
      addDirectory( dynamic_cast<Directory *>( fileItem ), item );   
  }
}

#include "dulistview.moc"
