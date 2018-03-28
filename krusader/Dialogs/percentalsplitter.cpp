/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
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

#include "percentalsplitter.h"

// QtCore
#include <QList>
// QtGui
#include <QPainter>
#include <QCursor>
// QtWidgets
#include <QApplication>
#include <QLabel>
#include <QFrame>
#include <QToolTip>

PercentalSplitter::PercentalSplitter(QWidget * parent) : QSplitter(parent), label(0), opaqueOldPos(-1)
{
    connect(this, SIGNAL(splitterMoved(int,int)), SLOT(slotSplitterMoved(int,int)));
}

PercentalSplitter::~PercentalSplitter()
{
}

QString PercentalSplitter::toolTipString(int p)
{
    QList<int> values = sizes();
    if (values.count() != 0) {
        int sum = 0;

        QListIterator<int> it(values);
        while (it.hasNext())
            sum += it.next();

        if (sum == 0)
            sum++;

        int percent = (int)(((double)p / (double)(sum)) * 10000. + 0.5);
        return QString("%1.%2%3").arg(percent / 100).arg((percent / 10) % 10).arg(percent % 10) + '%';
    }
    return QString();
}

void PercentalSplitter::slotSplitterMoved(int p, int index)
{
    handle(index) -> setToolTip(toolTipString(p));

    QToolTip::showText(QCursor::pos(), toolTipString(p) , this);
}

void PercentalSplitter::showEvent(QShowEvent * event)
{
    QList<int> values = sizes();

    for (int i = 0; i != count(); i++) {
        int p = 0;
        for (int j = 0; j < i; j++)
            p += values[ j ];

        handle(i) -> setToolTip(toolTipString(p));
    }

    QSplitter::showEvent(event);
}

