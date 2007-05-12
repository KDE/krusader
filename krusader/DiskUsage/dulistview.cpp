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
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>
#include <q3header.h>
#include <time.h>

DUListView::DUListView( DiskUsage *usage, const char *name ) 
    : Q3ListView( usage, name ), diskUsage( usage )
{  
  setAllColumnsShowFocus(true);
  setVScrollBarMode(Q3ScrollView::Auto);
  setHScrollBarMode(Q3ScrollView::Auto);
  setShowSortIndicator(true);
  setRootIsDecorated( true );
  setTreeStepSize( 10 );

  int defaultSize = QFontMetrics(font()).width("W");
  
  krConfig->setGroup( diskUsage->getConfigGroup() ); 
  int nameWidth  = krConfig->readNumEntry("D Name Width",  defaultSize * 20 );    
  addColumn( i18n("Name"), nameWidth );
  setColumnWidthMode(0,Q3ListView::Manual);
  int percentWidth  = krConfig->readNumEntry("D Percent Width",  defaultSize * 5 );    
  addColumn( i18n("Percent"), percentWidth );
  setColumnWidthMode(1,Q3ListView::Manual);
  int totalSizeWidth  = krConfig->readNumEntry("D Total Size Width",  defaultSize * 10 );    
  addColumn( i18n("Total size"), totalSizeWidth );
  setColumnWidthMode(1,Q3ListView::Manual);
  int ownSizeWidth  = krConfig->readNumEntry("D Own Size Width",  defaultSize * 10 );    
  addColumn( i18n("Own size"), ownSizeWidth );
  setColumnWidthMode(2,Q3ListView::Manual);
  int typeWidth  = krConfig->readNumEntry("D Type Width",  defaultSize * 10 );
  addColumn( i18n("Type"), typeWidth );
  setColumnWidthMode(3,Q3ListView::Manual);
  int dateWidth  = krConfig->readNumEntry("D Date Width",  defaultSize * 10 );
  addColumn( i18n("Date"), dateWidth );
  setColumnWidthMode(4,Q3ListView::Manual);
  int permissionsWidth  = krConfig->readNumEntry("D Permissions Width",  defaultSize * 6 );
  addColumn( i18n("Permissions"), permissionsWidth );
  setColumnWidthMode(5,Q3ListView::Manual);
  int ownerWidth  = krConfig->readNumEntry("D Owner Width",  defaultSize * 5 );    
  addColumn( i18n("Owner"), ownerWidth );
  setColumnWidthMode(6,Q3ListView::Manual);
  int groupWidth  = krConfig->readNumEntry("D Group Width",  defaultSize * 5 );    
  addColumn( i18n("Group"), groupWidth );
  setColumnWidthMode(7,Q3ListView::Manual);
  
  setColumnAlignment( 1, Qt::AlignRight );
  setColumnAlignment( 2, Qt::AlignRight );
  setColumnAlignment( 3, Qt::AlignRight );
  
  setSorting( 2 );

  connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
  connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
  connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
  connect( diskUsage, SIGNAL( deleted( File * ) ), this, SLOT( slotDeleted( File * ) ) );

  connect( this, SIGNAL(rightButtonPressed(Q3ListViewItem *, const QPoint &, int)),
           this, SLOT( slotRightClicked(Q3ListViewItem *) ) );
  connect( this, SIGNAL( expanded ( Q3ListViewItem * ) ), 
           this, SLOT( slotExpanded( Q3ListViewItem * ) ) ); 
}

DUListView::~ DUListView()
{
  krConfig->setGroup( diskUsage->getConfigGroup() ); 
  krConfig->writeEntry("D Name Width",        columnWidth( 0 ) );
  krConfig->writeEntry("D Percent Width",     columnWidth( 1 ) );
  krConfig->writeEntry("D Total Size Width",  columnWidth( 2 ) );
  krConfig->writeEntry("D Own Size Width",    columnWidth( 3 ) );
  krConfig->writeEntry("D Type Width",        columnWidth( 4 ) );
  krConfig->writeEntry("D Date Width",        columnWidth( 5 ) );
  krConfig->writeEntry("D Permissions Width", columnWidth( 6 ) );
  krConfig->writeEntry("D Owner Width",       columnWidth( 7 ) );
  krConfig->writeEntry("D Group Width",       columnWidth( 8 ) );
}

void DUListView::addDirectory( Directory *dirEntry, Q3ListViewItem *parent )
{
  Q3ListViewItem * lastItem = 0;
    
  if( parent == 0 && ! ( dirEntry->parent() == 0 ) )
  {
    lastItem = new Q3ListViewItem( this, ".." );
    lastItem->setPixmap( 0, FL_LOADICON( "up" ) );
    lastItem->setSelectable( false );
  }
          
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File *item = *it;
    
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
  }
  
  Q3ListViewItem *first = firstChild();
  if( first )
    setCurrentItem( first );
}

void DUListView::slotDirChanged( Directory *dirEntry )
{
  clear();  
  addDirectory( dirEntry, 0 );
}

File * DUListView::getCurrentFile()
{
  Q3ListViewItem *item = currentItem();
  
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
  duItem->setVisible( !item->isExcluded() );
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
  
void DUListView::slotRightClicked( Q3ListViewItem *item )
{
  File * file = 0;
  
  if ( item && item->text( 0 ) != ".." )
    file = ((DUListViewItem *)item)->getFile();

  diskUsage->rightClickMenu( file );
}

bool DUListView::doubleClicked( Q3ListViewItem * item )
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

void DUListView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  if ( e || e->button() == LeftButton )
  {
    QPoint vp = contentsToViewport(e->pos());
    Q3ListViewItem * item = itemAt( vp );

    if( doubleClicked( item ) )
      return;
    
  }
  Q3ListView::contentsMouseDoubleClickEvent( e );
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
    if( e->state() == ShiftButton )
    {
      e->ignore();
      return;
    }
    break;
  case Qt::Key_Delete :
    e->ignore();
    return;
  }
  Q3ListView::keyPressEvent( e );
}

void DUListView::slotExpanded( Q3ListViewItem *item )
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
