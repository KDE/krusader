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

#ifndef KRLISTWIDGET_H
#define KRLISTWIDGET_H

// QtWidgets
#include <QListWidget>

class KrListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit KrListWidget(QWidget * parent = 0);

signals:
    void itemRightClicked(QListWidgetItem * it, const QPoint & pos);
};

#endif /* KRLISTVIEW_H */
