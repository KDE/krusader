/*****************************************************************************
 * Copyright (C) 2003-2004 Max Howell <max.howell@methylblue.com>            *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef SEGMENTTIP_H
#define SEGMENTTIP_H

#include <QPixmap>
#include <qwidget.h>
#include <QEvent>

class File;
class Directory;

namespace RadialMap
{
    class SegmentTip : public QWidget
    {
    public:
        SegmentTip( uint );

        void updateTip( const File*, const Directory* );
        void moveto( QPoint, QWidget&, bool );

    private:
        virtual bool eventFilter( QObject*, QEvent* );
        virtual bool event( QEvent* );

        uint    m_cursorHeight;
        QPixmap m_pixmap;
        QString m_text;
    };
}

#endif
