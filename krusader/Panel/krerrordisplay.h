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

#ifndef KRERRORDISPLAY_H
#define KRERRORDISPLAY_H

// QtCore
#include <QTimer>
// QtGui
#include <QColor>
// QtWidgets
#include <QWidget>
#include <QLabel>

class KrErrorDisplay: public QLabel
{
    Q_OBJECT
public:
    explicit KrErrorDisplay(QWidget *parent);

    void setText(QString text);

private slots:
    void slotTimeout();

private:
    void dim();

    QTimer _dimTimer;
    QColor _startColor;
    QColor _targetColor;
    int _currentDim;
};

#endif
