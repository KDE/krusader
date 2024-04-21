/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krstyleproxy.h"

// QtGui
#include <QPainter>
#include <QPen>
// QtWidgets
#include <QStyleOptionViewItem>

#include <KSharedConfig>

#include "../krglobal.h"

void KrStyleProxy::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
            painter->drawRect(focusRect.adjusted(0, 0, -1, -1)); // draw pen inclusive
            painter->setPen(oldPen);
        }
    } else
        QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int KrStyleProxy::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
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
