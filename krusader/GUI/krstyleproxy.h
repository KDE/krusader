/***************************************************************************
                                krstyleproxy.h
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRSTYLEPROXY_H
#define KRSTYLEPROXY_H

// QtWidgets
#include <QStyle>

class QPainter;
class QWidget;
class QStyleOptionComplex;

class KrStyleProxy: public QStyle
{
public:
    KrStyleProxy() {}
    virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex * option, QPainter * painter, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual void drawItemPixmap(QPainter * painter, const QRect & rectangle, int alignment, const QPixmap & pixmap) const Q_DECL_OVERRIDE;
    virtual void drawItemText(QPainter * painter, const QRect & rectangle, int alignment, const QPalette & palette, bool enabled, const QString & text, QPalette::ColorRole textRole = QPalette::NoRole) const Q_DECL_OVERRIDE;
    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap & pixmap, const QStyleOption * option) const Q_DECL_OVERRIDE;
    virtual QStyle::SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex * option, const QPoint & position, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual QRect itemPixmapRect(const QRect & rectangle, int alignment, const QPixmap & pixmap) const Q_DECL_OVERRIDE;
    virtual QRect itemTextRect(const QFontMetrics & metrics, const QRect & rectangle, int alignment, bool enabled, const QString & text) const Q_DECL_OVERRIDE;
    virtual int layoutSpacing(QSizePolicy::ControlType ctrl1, QSizePolicy::ControlType ctrl2, Qt::Orientation orientation, const QStyleOption *option = 0, const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual void polish(QWidget * widget) Q_DECL_OVERRIDE;
    virtual void polish(QApplication * application) Q_DECL_OVERRIDE;
    virtual void polish(QPalette & palette) Q_DECL_OVERRIDE;
    virtual QSize sizeFromContents(ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual QIcon standardIcon(StandardPixmap stdIcon, const QStyleOption *option = 0, const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    virtual QPalette standardPalette() const Q_DECL_OVERRIDE;
    virtual int styleHint(StyleHint stylehint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const Q_DECL_OVERRIDE;
    virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex * option, SubControl subControl, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual QRect subElementRect(SubElement element, const QStyleOption * option, const QWidget * widget = 0) const Q_DECL_OVERRIDE;
    virtual QPixmap standardPixmap(QStyle::StandardPixmap pixmap, const QStyleOption* option, const QWidget*widget = 0) const Q_DECL_OVERRIDE;
    virtual void unpolish(QWidget * widget) Q_DECL_OVERRIDE;
    virtual void unpolish(QApplication * application) Q_DECL_OVERRIDE;
};


#endif /* KRSTYLEPROXY_H */


