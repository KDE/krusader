/***************************************************************************
                               krstyleproxy.cpp
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

#include "krstyleproxy.h"
#include <qpen.h>
#include <qpainter.h>
#include <QStyleOptionViewItem>
#include <qapplication.h>

void KrStyleProxy::drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option, QPainter * painter, const QWidget * widget ) const {
  QApplication::style()->drawComplexControl( control, option, painter, widget );
}

void KrStyleProxy::drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget ) const {
  QApplication::style()->drawControl( element, option, painter, widget );
}

void KrStyleProxy::drawItemPixmap ( QPainter * painter, const QRect & rectangle, int alignment, const QPixmap & pixmap ) const {
  QApplication::style()->drawItemPixmap( painter, rectangle, alignment, pixmap );
}

void KrStyleProxy::drawItemText ( QPainter * painter, const QRect & rectangle, int alignment, const QPalette & palette, bool enabled, const QString & text, QPalette::ColorRole textRole ) const {
  QApplication::style()->drawItemText( painter, rectangle, alignment, palette, enabled, text, textRole );
}

void KrStyleProxy::drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget ) const {
  if( element == QStyle::PE_FrameFocusRect ) {
      if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
          QColor bg = fropt->backgroundColor;
          QPen oldPen = painter->pen();
          QPen newPen;
          if (bg.isValid()) {
              int h, s, v;
              bg.getHsv(&h, &s, &v);
              if (v >= 128)
                  newPen.setColor(Qt::black);
              else
                  newPen.setColor(Qt::white);
          } else {
              newPen.setColor(option->palette.foreground().color());
          }
          newPen.setWidth( 0 );
          newPen.setStyle( Qt::DotLine );
          painter->setPen( newPen );
          QRect focusRect = option->rect /*.adjusted(1, 1, -1, -1) */;
          painter->drawRect(focusRect.adjusted(0, 0, -1, -1)); //draw pen inclusive
          painter->setPen(oldPen);
      }
  }
  else
    QApplication::style()->drawPrimitive( element, option, painter, widget );
}

QPixmap KrStyleProxy::generatedIconPixmap ( QIcon::Mode iconMode, const QPixmap & pixmap, const QStyleOption * option ) const {
  return QApplication::style()->generatedIconPixmap( iconMode, pixmap, option );
}

QStyle::SubControl KrStyleProxy::hitTestComplexControl ( ComplexControl control, const QStyleOptionComplex * option, const QPoint & position, const QWidget * widget ) const {
  return QApplication::style()->hitTestComplexControl ( control, option, position, widget );
}

QRect KrStyleProxy::itemPixmapRect ( const QRect & rectangle, int alignment, const QPixmap & pixmap ) const {
  return QApplication::style()->itemPixmapRect ( rectangle, alignment, pixmap );
}

QRect KrStyleProxy::itemTextRect ( const QFontMetrics & metrics, const QRect & rectangle, int alignment, bool enabled, const QString & text ) const {
  return QApplication::style()->itemTextRect ( metrics, rectangle, alignment, enabled, text );
}

int KrStyleProxy::pixelMetric ( PixelMetric metric, const QStyleOption * option, const QWidget * widget ) const {
  return QApplication::style()->pixelMetric ( metric, option, widget );
}

void KrStyleProxy::polish ( QWidget * widget ) {
  QApplication::style()->polish( widget );
}

void KrStyleProxy::polish ( QApplication * application ) {
  QApplication::style()->polish( application );
}

void KrStyleProxy::polish ( QPalette & palette ) {
  QApplication::style()->polish( palette );
}

QSize KrStyleProxy::sizeFromContents ( ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget ) const {
  return QApplication::style()->sizeFromContents ( type, option, contentsSize, widget );
}

QPalette KrStyleProxy::standardPalette () const {
  return QApplication::style()->standardPalette ();
}

int KrStyleProxy::styleHint ( StyleHint hint, const QStyleOption * option, const QWidget * widget, QStyleHintReturn * returnData ) const {
  return QApplication::style()->styleHint( hint, option, widget, returnData );
}

QRect KrStyleProxy::subControlRect ( ComplexControl control, const QStyleOptionComplex * option, SubControl subControl, const QWidget * widget ) const {
  return QApplication::style()->subControlRect ( control, option, subControl, widget );
}

QRect KrStyleProxy::subElementRect ( SubElement element, const QStyleOption * option, const QWidget * widget ) const {
  return QApplication::style()->subElementRect ( element, option, widget );
}

QPixmap KrStyleProxy::standardPixmap(QStyle::StandardPixmap pixmap, const QStyleOption* option, const QWidget*widget) const {
  return QApplication::style()->standardPixmap ( pixmap, option, widget );
}

void KrStyleProxy::unpolish ( QWidget * widget ) {
  QApplication::style()->unpolish( widget );
}

void KrStyleProxy::unpolish ( QApplication * application ) {
  QApplication::style()->unpolish( application );
}
