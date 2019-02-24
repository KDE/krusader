/*****************************************************************************
 * Copyright (C) 2008 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2008-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KRTREEWIDGET_H
#define KRTREEWIDGET_H

// QtWidgets
#include <QAbstractScrollArea>
#include <QTreeWidget>

class KrTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit KrTreeWidget(QWidget * parent);
    void setStretchingColumn(int col)                {
        _stretchingColumn = col;
    }

    QModelIndex indexOf(QTreeWidgetItem * item, int col = 0) {
        return indexFromItem(item, col);
    }
    QTreeWidgetItem * item(const QModelIndex & ndx)          {
        return itemFromIndex(ndx);
    }

signals:
    void itemRightClicked(QTreeWidgetItem * it, const QPoint & pos, int column);

protected:
    bool event(QEvent * event) Q_DECL_OVERRIDE;

private:
    int  _stretchingColumn;
    bool _inResize;
};

#endif /* KRTREEWIDGET_H */
