/*****************************************************************************
 * Copyright (C) 2008 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2008-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

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
        if (const auto *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(option)) {
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
                newPen.setColor(option->palette.windowText().color());
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
