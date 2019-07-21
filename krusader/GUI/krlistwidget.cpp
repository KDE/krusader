/*****************************************************************************
 * Copyright (C) 2008 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2008-2019 Krusader Krew [https://krusader.org]              *
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

#include "krlistwidget.h"
#include "krstyleproxy.h"

// QtGui
#include <QContextMenuEvent>

KrListWidget::KrListWidget(QWidget *parent) : QListWidget(parent)
{
    auto *style = new KrStyleProxy();
    style->setParent(this);
    setStyle(style);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, [=](const QPoint &pos) {
        QListWidgetItem *item = itemAt(pos);
        emit itemRightClicked(item, viewport()->mapToGlobal(pos));
    });
}
