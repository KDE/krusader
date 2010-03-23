/***************************************************************************
                            krerrordisplay.cpp
                         -------------------
copyright            : (C) 2010 by Jan Lepper
e-mail               : krusader@users.sourceforge.net
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

#include <stdio.h>

#include "krerrordisplay.h"
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
    connect(&_dimTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

void KrErrorDisplay::setTargetColor(QColor &c)
{
    _targetColor = c;
    dim();
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
