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

// QtGui
#include <QPen>
#include <QPainter>
// QtWidgets
#include <QStyleOptionViewItem>

#include <KConfigCore/KSharedConfig>

#include "../krglobal.h"

void KrStyleProxy::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                 QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_FrameFocusRect) {
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
            newPen.setWidth(0);
            newPen.setStyle(Qt::DotLine);
            painter->setPen(newPen);
            QRect focusRect = option->rect /*.adjusted(1, 1, -1, -1) */;
            painter->drawRect(focusRect.adjusted(0, 0, -1, -1)); //draw pen inclusive
            painter->setPen(oldPen);
        }
    } else
        QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int KrStyleProxy::styleHint(QStyle::StyleHint hint, const QStyleOption *option,
                            const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_ToolTip_WakeUpDelay) {
        KConfigGroup group(krConfig, "Look&Feel");
        const int delay = group.readEntry("Panel Tooltip Delay", 1000);
        return delay < 0 ? INT_MAX : delay;
    }

    // disable instant consecutive tooltips
    if (hint == QStyle::SH_ToolTip_FallAsleepDelay) {
        return 0;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}
