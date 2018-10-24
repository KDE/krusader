/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
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

#include "krerrordisplay.h"
#include <stdio.h>

#include "krcolorcache.h"


KrErrorDisplay::KrErrorDisplay(QWidget *parent) :
    QLabel(parent),
    _currentDim(100)
{
    setAutoFillBackground(true);
    _startColor = QColor(240,150,150);
    QPalette p(palette());
    _targetColor = p.color(QPalette::Window);
    p.setColor(QPalette::Window, _startColor);
    setPalette(p);

    _dimTimer.setSingleShot(true);
    connect(&_dimTimer, &QTimer::timeout, this, &KrErrorDisplay::slotTimeout);
}

void KrErrorDisplay::setText(QString text)
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
    if( _currentDim > 0)
        _dimTimer.start(50);
}

void KrErrorDisplay::dim()
{
    QPalette p(palette());
    p.setColor(QPalette::Window, KrColorCache::dimColor(_startColor, _currentDim, _targetColor));
    setPalette(p);
}
