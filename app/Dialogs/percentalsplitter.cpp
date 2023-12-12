/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "percentalsplitter.h"

// QtCore
#include <QList>
// QtGui
#include <QCursor>
#include <QPainter>
// QtWidgets
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QToolTip>

PercentalSplitter::PercentalSplitter(QWidget *parent)
    : QSplitter(parent)
    , label(nullptr)
    , opaqueOldPos(-1)
{
    connect(this, &PercentalSplitter::splitterMoved, this, &PercentalSplitter::slotSplitterMoved);
}

PercentalSplitter::~PercentalSplitter() = default;

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

        auto percent = (int)(((double)p / (double)(sum)) * 10000. + 0.5);
        return QString("%1.%2%3").arg(percent / 100).arg((percent / 10) % 10).arg(percent % 10) + '%';
    }
    return QString();
}

void PercentalSplitter::slotSplitterMoved(int p, int index)
{
    handle(index)->setToolTip(toolTipString(p));

    QToolTip::showText(QCursor::pos(), toolTipString(p), this);
}

void PercentalSplitter::showEvent(QShowEvent *event)
{
    QList<int> values = sizes();

    for (int i = 0; i != count(); i++) {
        int p = 0;
        for (int j = 0; j < i; j++)
            p += values[j];

        handle(i)->setToolTip(toolTipString(p));
    }

    QSplitter::showEvent(event);
}
