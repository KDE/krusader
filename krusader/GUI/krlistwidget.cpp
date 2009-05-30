/***************************************************************************
                               krlistwidget.cpp
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

#include "krlistwidget.h"
#include "krstyleproxy.h"
#include <QtGui/QContextMenuEvent>

KrListWidget::KrListWidget( QWidget * parent ) : QListWidget( parent ) {
  setStyle( new KrStyleProxy() );
}

bool KrListWidget::event ( QEvent * event )
{
  switch (event->type()) {
  // HACK: QT 4 Context menu key isn't handled properly
  case QEvent::ContextMenu:
    {
      QContextMenuEvent* ce = (QContextMenuEvent*) event;

      if( ce->reason() == QContextMenuEvent::Mouse ) {
        QPoint pos = viewport()->mapFromGlobal( ce->globalPos() );

        QListWidgetItem * item = itemAt( pos );

        emit itemRightClicked( item, ce->globalPos() );
        return true;
      } else {
        if( currentItem() ) {
          QRect r = visualItemRect( currentItem() );
          QPoint p = viewport()->mapToGlobal( QPoint( r.x() + 5, r.y() + 5 ) );

          emit itemRightClicked( currentItem(), p );
          return true;
        }
      }
    }
    break;
  default:
    break;
  }

  return QListWidget::event( event );
}
