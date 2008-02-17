/***************************************************************************
                                 krtreewidget.cpp
                             -------------------
    copyright            : (C) 2008+ by Csaba Karai
    email                : krusader@users.sourceforge.net
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

#include "krtreewidget.h"
#include "krstyleproxy.h"
#include <qevent.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qtooltip.h>

KrTreeWidget::KrTreeWidget( QWidget * parent ) : QTreeWidget( parent ), _inResize( false )
{
  setRootIsDecorated( false );
  setSortingEnabled( true );
  setAllColumnsShowFocus(true);

  _stretchingColumn = -1;

  setStyle( new KrStyleProxy( style() ) );
}

bool KrTreeWidget::event ( QEvent * event )
{
  switch (event->type()) {
  // HACK: QT 4 Context menu key isn't handled properly
  case QEvent::ContextMenu:
    {
      QContextMenuEvent* ce = (QContextMenuEvent*) event;

      if( ce->reason() == QContextMenuEvent::Mouse ) {
        QPoint pos = viewport()->mapFromGlobal( ce->globalPos() );

        QTreeWidgetItem * item = itemAt( pos );
        int column = columnAt( pos.x() );

        emit itemRightClicked( item, ce->globalPos(), column );
        return true;
      } else {
        if( currentItem() ) {
          QRect r = visualItemRect( currentItem() );
          QPoint p = viewport()->mapToGlobal( QPoint( r.x() + 5, r.y() + 5 ) );

          emit itemRightClicked( currentItem(), p, currentColumn() );
          return true;
        }
      }
    }
    break;
  case QEvent::KeyPress:
    {
      // HACK: QT 4 Ctrl+A bug fix: Ctrl+A doesn't work if QTreeWidget contains parent / child items
      QKeyEvent* ke = (QKeyEvent*) event;
      switch( ke->key() )
      {
      case Qt::Key_A:
        if( ke->modifiers() == Qt::ControlModifier )
        {
          QAbstractItemView::SelectionMode mode = selectionMode();

          if( mode == QAbstractItemView::ContiguousSelection || mode == QAbstractItemView::ExtendedSelection ||
              mode == QAbstractItemView::MultiSelection )
          {
            selectAll();
            ke->accept();
            return true;
          }
        }
        break;
      default:
        break;
      }
    }
    break;
  case QEvent::Resize:
    {
      QResizeEvent * re = (QResizeEvent *)event;
      if( !_inResize && re->oldSize() != re->size() )
      {
        if( _stretchingColumn != -1 && columnCount() )
        {
          QList< int > columnsSizes;
          int oldSize = 0;

          for( int i=0; i != header()->count(); i++ ) {
             columnsSizes.append( header()->sectionSize( i ) );
             oldSize += header()->sectionSize( i );
          }

          bool res = QTreeWidget::event( event );

          int newSize = viewport()->width();
          int delta = newSize - oldSize;

          if( delta )
          {
            _inResize = true;

            for( int i=0; i != header()->count(); i++ ) {
               if( i == _stretchingColumn )
               {
                 int newNs = columnsSizes[ i ] + delta;
                 if( newNs < 8 )
                   newNs = 8;
                 header()->resizeSection( i, newNs );
               } else if( header()->sectionSize( i ) != columnsSizes[ i ] )
               {
                 header()->resizeSection( i, columnsSizes[ i ] );
               }
            }
            _inResize = false;
          }
          return res;
        }
      }
      break;
    }
  case QEvent::ToolTip:
    {
      QHelpEvent *he = static_cast<QHelpEvent*>(event);

      if( viewport() )
      {
        QPoint pos = viewport()->mapFromGlobal( he->globalPos() );

        QTreeWidgetItem * item = itemAt( pos );

        int column = columnAt( pos.x() );

        if ( item ) {
            if( !item->toolTip( column ).isEmpty() )
              break;

            QString tip = item->text( column );

            int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            int requiredWidth = QFontMetrics( font() ).width( tip ) + 2 * textMargin;

            if( column == 0 && indentation() )
            {
              int level = 0;

              QTreeWidgetItem *parent = item;

              while( (parent = parent->parent()) )
                level++;

              if( rootIsDecorated() )
                level++;

              requiredWidth += level * indentation();
            }

            QIcon icon = item->icon( column );
            if( !icon.isNull() )
            {
              QStyleOptionViewItem opts = viewOptions();
              QSize iconSize = icon.actualSize(opts.decorationSize);
              requiredWidth += iconSize.width();

              int pixmapMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, this) + 1;
              requiredWidth += 2 * pixmapMargin;
            }

            if( !tip.isEmpty() && ( columnWidth( column ) < requiredWidth ) )
              QToolTip::showText(he->globalPos(), tip, this);
            return true;
        }
      }
    }
    break;
  default:
    break;
  }
  return QTreeWidget::event( event );
}
