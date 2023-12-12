/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krerrordisplay.h"
#include <stdio.h>

#include "krcolorcache.h"

KrErrorDisplay::KrErrorDisplay(QWidget *parent)
    : QLabel(parent)
    , _currentDim(100)
{
    setAutoFillBackground(true);
    _startColor = QColor(240, 150, 150);
    QPalette p(palette());
    _targetColor = p.color(QPalette::Window);
    p.setColor(QPalette::Window, _startColor);
    setPalette(p);

    _dimTimer.setSingleShot(true);
    connect(&_dimTimer, &QTimer::timeout, this, &KrErrorDisplay::slotTimeout);
}

void KrErrorDisplay::setText(const QString &text)
{
    QLabel::setText(text);
    _currentDim = 100;

    QPalette p(palette());
    p.setColor(QPalette::Window, _startColor);
    setPalette(p);

    _dimTimer.start(5000);
}

void KrErrorDisplay::slotTimeout()
{
    _currentDim -= 2;
    dim();
    if (_currentDim > 0)
        _dimTimer.start(50);
}

void KrErrorDisplay::dim()
{
    QPalette p(palette());
    p.setColor(QPalette::Window, KrColorCache::dimColor(_startColor, _currentDim, _targetColor));
    setPalette(p);
}
