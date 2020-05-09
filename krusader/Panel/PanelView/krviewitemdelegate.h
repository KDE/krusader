/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <cskarai@freemail.hu>                      *
 * Copyright (C) 2009-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef KRVIEWITEMDELEGATE_H
#define KRVIEWITEMDELEGATE_H

// QtWidgets
#include <QItemDelegate>

class KrViewItemDelegate : public QItemDelegate
{
public:
    explicit KrViewItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                     const QString &text) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &sovi,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    mutable int _currentlyEdited;
    mutable bool _dontDraw;
};

#endif
