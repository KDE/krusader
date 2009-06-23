/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <cskarai@freemail.hu>                      *
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

#ifndef KRINTERVIEWITEMDELEGATE_H
#define KRINTERVIEWITEMDELEGATE_H

#include <QtGui/QItemDelegate>

class KrInterViewItemDelegate : public QItemDelegate
{
public:
    KrInterViewItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDisplay(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text) const;
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    bool eventFilter(QObject *object, QEvent *event);

private:
    mutable int _currentlyEdited;
    mutable bool _dontDraw;
};

#endif
