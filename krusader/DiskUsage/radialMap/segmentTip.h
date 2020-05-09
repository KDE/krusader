/*****************************************************************************
 * Copyright (C) 2003-2004 Max Howell <max.howell@methylblue.com>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef SEGMENTTIP_H
#define SEGMENTTIP_H

// QtCore
#include <QEvent>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QWidget>

class File;
class Directory;

namespace RadialMap
{
class SegmentTip : public QWidget
{
public:
    explicit SegmentTip(uint);

    void updateTip(const File*, const Directory*);
    void moveto(QPoint, QWidget&, bool);

private:
    virtual bool eventFilter(QObject*, QEvent*) override;
    virtual bool event(QEvent*) override;

    uint    m_cursorHeight;
    QPixmap m_pixmap;
    QString m_text;
};
}

#endif
