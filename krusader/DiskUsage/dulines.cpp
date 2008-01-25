/***************************************************************************
                         dulines.cpp  -  description
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

#include "dulines.h"
#include "../kicons.h"
#include "../krusader.h"
#include "../VFS/krpermhandler.h"
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <klocale.h>
#include <qpen.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qheaderview.h>
#include <kmenu.h>
#include <QItemDelegate>
#include <qtooltip.h>

class DULinesItemDelegate : public QItemDelegate
{
public:

   DULinesItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) {}

   virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
   {
     QItemDelegate::paint( painter, option, index );

     QVariant value = index.data(Qt::UserRole);
     if( value.isValid() )
     {
       QString text = value.toString();

       value = index.data(Qt::DisplayRole);
       QString display;
       if( value.isValid() )
         display = value.toString();

       QSize iconSize;
       value = index.data(Qt::DecorationRole);
       if (value.isValid())
         iconSize = qvariant_cast<QIcon>(value).actualSize(option.decorationSize);

       painter->save();
       painter->setClipRect( option.rect );

       QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                 ? QPalette::Normal : QPalette::Disabled;
       if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
          cg = QPalette::Inactive;
       if (option.state & QStyle::State_Selected) {
          painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
       } else {
          painter->setPen(option.palette.color(cg, QPalette::Text));
       }

       QFont fnt = option.font;
       fnt.setItalic( true );
       painter->setFont( fnt );

       QFontMetrics fm( fnt );
       QString renderedText = text;

       int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
       int pos = 3*textMargin + option.fontMetrics.width( display ) + iconSize.width();

       bool truncd = false;

       QRect rct = option.rect;
       if( rct.width() > pos )
       {
         rct.setX( rct.x() + pos );

         if( fm.width( renderedText ) > rct.width() )
         {
           truncd = true;

           int points = fm.width( "..." );

           while( !renderedText.isEmpty() && ( fm.width( renderedText ) + points > rct.width() ) )
             renderedText.truncate( renderedText.length() -1 );

           renderedText += "...";
         }

         painter->drawText( rct, Qt::AlignLeft, renderedText );
       } else
         truncd = true;

       if( truncd )
         ((QAbstractItemModel *)index.model())->setData( index, QVariant( display + "  " + text ), Qt::ToolTipRole );
       else
         ((QAbstractItemModel *)index.model())->setData( index, QVariant(), Qt::ToolTipRole );

       painter->restore();
     }
   }
};

class DULinesItem : public QTreeWidgetItem
{
public:
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, QTreeWidget * parent, QString label1, 
               QString label2, QString label3 ) : QTreeWidgetItem( parent ), 
               diskUsage( diskUsageIn ), file( fileItem )
  {
     setText( 0, label1 );
     setText( 1, label2 );
     setText( 2, label3 );

     setTextAlignment( 1, Qt::AlignRight );
  }
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, QTreeWidget * parent, QTreeWidgetItem * after, 
               QString label1, QString label2, QString label3 ) : QTreeWidgetItem( parent, after ), 
               diskUsage( diskUsageIn ), file( fileItem )
  {
     setText( 0, label1 );
     setText( 1, label2 );
     setText( 2, label3 );

     setTextAlignment( 1, Qt::AlignRight );
  }
  
  virtual bool operator<(const QTreeWidgetItem &other) const
  {
      int column = treeWidget() ? treeWidget()->sortColumn() : 0;

      if( text( 0 ) == ".." )
        return true;

      const DULinesItem *compWith = dynamic_cast< const DULinesItem * >( &other );
      if( compWith == 0 )
        return false;

      switch( column )
      {
      case 0:
      case 1:
        return file->size() > compWith->file->size();
      default:
        return text(column) < other.text(column);
      }
  }
    
  inline File * getFile() { return file; }
  
private:
  DiskUsage *diskUsage;
  File *file;
};

DULines::DULines( DiskUsage *usage )
  : KrTreeWidget( usage ), diskUsage( usage ), refreshNeeded( false ), started( false )
{
  setItemDelegate( itemDelegate = new DULinesItemDelegate() );

  setAllColumnsShowFocus(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setIndentation( 10 );

  int defaultSize = QFontMetrics(font()).width("W");
  
  QStringList labels;
  labels << i18n("Line View");
  labels << i18n("Percent");
  labels << i18n("Name");
  setHeaderLabels( labels );

  header()->setResizeMode( 0, QHeaderView::Interactive );
  header()->setResizeMode( 1, QHeaderView::Interactive );
  header()->setResizeMode( 2, QHeaderView::Interactive );

  KConfigGroup group( krConfig, diskUsage->getConfigGroup() ); 

  showFileSize = group.readEntry( "L Show File Size", true );

  if( group.hasKey( "L State" ) )
    header()->restoreState( group.readEntry( "L State", QByteArray() ) );
  else
  {
    setColumnWidth(0, defaultSize * 20 );
    setColumnWidth(1, defaultSize * 6 );
    setColumnWidth(2, defaultSize * 20 );
  }

  setStretchingColumn( 0 );
  
  header()->setSortIndicatorShown(true);
  sortItems( 1, Qt::AscendingOrder );

//  toolTip = new DULinesToolTip( diskUsage, viewport(), this );

  connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
  connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
  
  connect( header(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( sectionResized( int ) ) );
  connect( header(), SIGNAL( sectionAutoResize ( int, QHeaderView::ResizeMode ) ), this, SLOT( sectionResized( int ) ) );

  connect( this, SIGNAL( itemRightClicked ( QTreeWidgetItem*, int ) ),
           this, SLOT( slotRightClicked(QTreeWidgetItem *) ) );
  connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
  connect( diskUsage, SIGNAL( deleted( File * ) ), this, SLOT( slotDeleted( File * ) ) );

  started = true;
}

DULines::~DULines()
{
  KConfigGroup group( krConfig, diskUsage->getConfigGroup() ); 
  group.writeEntry( "L State", header()->saveState() );
  
  delete itemDelegate;
}

bool DULines::event ( QEvent * event )
{
  switch (event->type()) {
  case QEvent::ToolTip:
    {
      QHelpEvent *he = static_cast<QHelpEvent*>(event);

      if( viewport() )
      {
        QPoint pos = viewport()->mapFromGlobal( he->globalPos() );

        QTreeWidgetItem * item = itemAt( pos );

        int column = columnAt( pos.x() );

        if ( item && column == 1 ) {
            File *fileItem = ((DULinesItem *)item)->getFile();
            QToolTip::showText(he->globalPos(), diskUsage->getToolTip( fileItem ), this);
            return true;
        }
      }
    }
    break;
  default:
    break;
  }
  return KrTreeWidget::event( event );
}

void DULines::slotDirChanged( Directory *dirEntry )
{
  clear();  
  
  QTreeWidgetItem * lastItem = 0;
    
  if( ! ( dirEntry->parent() == 0 ) )
  {
    lastItem = new QTreeWidgetItem( this );
    lastItem->setText(0, ".." );
    lastItem->setIcon( 0, FL_LOADICON( "up" ) );
    lastItem->setFlags( lastItem->flags() & (~Qt::ItemIsSelectable) );
  }
          
  int maxPercent = -1;
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File *item = *it;
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  { 
    File *item = *it;
    
    QString fileName = item->name();
    
    if( lastItem == 0 )
      lastItem = new DULinesItem( diskUsage, item, this, "", item->percent() + "  ", fileName );
    else
      lastItem = new DULinesItem( diskUsage, item, this, lastItem, "", item->percent() + "  ", fileName );
    
    if( item->isExcluded() )
      lastItem->setHidden( true );
                                    
    int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    lastItem->setIcon( 2, diskUsage->getIcon( item->mime() ) );
    lastItem->setData( 0, Qt::DecorationRole, createPixmap( item->intPercent(), maxPercent, header()->sectionSize( 0 ) - 2 * textMargin ) );

    if( showFileSize )
      lastItem->setData( 2, Qt::UserRole, "  [" + KIO::convertSize( item->size() ) + "]" );

    QSize size = lastItem->sizeHint( 0 );
    size.setWidth( 16 );
    lastItem->setSizeHint( 0, size );
  }
  
  if( topLevelItemCount() > 0 )
  {
    setCurrentItem( topLevelItem( 0 ) );
  }
}

QPixmap DULines::createPixmap( int percent, int maxPercent, int maxWidth )
{
  if( percent < 0 || percent > maxPercent || maxWidth < 2 || maxPercent == 0 )
    return QPixmap();
  maxWidth -= 2;

  int actualWidth = maxWidth*percent/maxPercent;
  if( actualWidth == 0 )
    return QPixmap();
    
  QPen pen;
  pen.setColor( Qt::black );  
  QPainter painter;
  
  int size = QFontMetrics(font()).height()-2;
  QRect rect( 0, 0, actualWidth, size );
  QRect frameRect( 0, 0, actualWidth-1, size-1 );
  QPixmap pixmap( rect.width(), rect.height() );

  painter.begin( &pixmap );
  painter.setPen( pen );
  
  for( int i = 1; i < actualWidth - 1; i++ )
  {
    int color = (511*i/(maxWidth - 1));
    if( color < 256 )
      pen.setColor( QColor( 255-color, 255, 0 ) );
    else
      pen.setColor( QColor( color-256, 511-color, 0 ) );
    
    painter.setPen( pen );
    painter.drawLine( i, 1, i, size-1 );
  }
  
  pen.setColor( Qt::black );  
  painter.setPen( pen );

  if( actualWidth != 1 )
    painter.drawRect( frameRect );
  else
    painter.drawLine( 0, 0, 0, size );

  painter.end();
  pixmap.detach();
  return pixmap;
}

void DULines::resizeEvent( QResizeEvent * re )
{
  KrTreeWidget::resizeEvent( re );

  if( started && ( re->oldSize() != re->size() ) )
    sectionResized( 0 );
}

void DULines::sectionResized( int column )
{
  if( topLevelItemCount() == 0 || column != 0 )
    return;
    
  Directory * currentDir = diskUsage->getCurrentDir();  
  if( currentDir == 0 )
    return;

  int maxPercent = -1;  
  for( Iterator<File> it = currentDir->iterator(); it != currentDir->end(); ++it )
  {
    File *item = *it;  
    
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  QTreeWidgetItemIterator it2( this );
  while( *it2 )
  {
    QTreeWidgetItem *lvitem = *it2;
    if( lvitem->text( 0 ) != ".." )
    {
      DULinesItem *duItem = dynamic_cast< DULinesItem *> ( lvitem );
      if( duItem )
      {
        int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        duItem->setData( 0, Qt::DecorationRole, createPixmap( duItem->getFile()->intPercent(), maxPercent, header()->sectionSize( 0 ) - 2 * textMargin ) );
        QSize size = duItem->sizeHint( 0 );
        size.setWidth( 16 );
        duItem->setSizeHint( 0, size );
      }
    }
    it2++;
  }
}

bool DULines::doubleClicked( QTreeWidgetItem * item )
{
  if( item )
  {
    if( item->text( 0 ) != ".." )
    {
      File *fileItem = ((DULinesItem *)item)->getFile();
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

void DULines::mouseDoubleClickEvent ( QMouseEvent * e )
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


void DULines::keyPressEvent( QKeyEvent *e )
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
 
void DULines::slotRightClicked( QTreeWidgetItem *item )
{
  File * file = 0;
  
  if ( item && item->text( 0 ) != ".." )
    file = ((DULinesItem *)item)->getFile();

  KMenu linesPopup;    
  int lid = linesPopup.insertItem( i18n("Show file sizes"), this, SLOT( slotShowFileSizes() ) );
  linesPopup.setItemChecked( lid, showFileSize );
    
  diskUsage->rightClickMenu( file, &linesPopup, i18n( "Lines" ) );
}

void DULines::slotShowFileSizes()
{
  showFileSize = !showFileSize;
  slotDirChanged( diskUsage->getCurrentDir() );
}

File * DULines::getCurrentFile()
{
  QTreeWidgetItem *item = currentItem();
  
  if( item == 0 || item->text( 0 ) == ".." )
    return 0;
  
  return ((DULinesItem *)item)->getFile();
}

void DULines::slotChanged( File * item )
{
  QTreeWidgetItemIterator it( this );
  while( *it )
  {
    QTreeWidgetItem *lvitem = *it;
    it++;

    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        setSortingEnabled( false );
        duItem->setHidden( item->isExcluded() );
        duItem->setText( 1, item->percent() );
        if( !refreshNeeded )
        {
          refreshNeeded = true;
          QTimer::singleShot( 0, this, SLOT( slotRefresh() ) );
        }
        break;
      }
    }
  }
}

void DULines::slotDeleted( File * item )
{
  QTreeWidgetItemIterator it( this );
  while( *it )
  {
    QTreeWidgetItem *lvitem = *it;
    it++;

    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        delete duItem;
        break;
      }
    }
  }
}

void DULines::slotRefresh() 
{ 
  if( refreshNeeded )
  {
    refreshNeeded = false;
    setSortingEnabled( true );
    sortItems( 1, Qt::AscendingOrder );
  }
}

#include "dulines.moc"
