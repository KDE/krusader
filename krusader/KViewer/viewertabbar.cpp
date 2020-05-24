/*****************************************************************************
 * Copyright (C) 2020 Krusader Krew [https://krusader.org]                   *
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

#include "viewertabbar.h"

#include <QDebug>
// QtGui
#include <QMouseEvent>

ViewerTabWidget::ViewerTabWidget(QWidget *parent) : QTabWidget(parent)
{
    setTabBar(new ViewerTabBar(this));
}

ViewerTabBar *ViewerTabWidget::tabBar() const
{
    return dynamic_cast<ViewerTabBar *>(QTabWidget::tabBar());
}

void ViewerTabBar::mousePressEvent(QMouseEvent *e)
{
    int clickedTab = tabAt(e->pos());

    if (-1 == clickedTab) { // clicked on nothing ...
        QTabBar::mousePressEvent(e);
        return;
    }

    setCurrentIndex(clickedTab);

    if (e->button() == Qt::MidButton) {
        // close the current tab
        emit closeTabSignal(clickedTab);
    }

    QTabBar::mousePressEvent(e);
}
