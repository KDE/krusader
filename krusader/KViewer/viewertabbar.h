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

#ifndef VIEWERTABBAR_H
#define VIEWERTABBAR_H

// QtWidgets
#include <QWidget>
#include <QTabBar>
#include <QTabWidget>

/**
 * This class extends QTabWidget such that we can use a custom QTabBar on it
 */
class ViewerTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit ViewerTabWidget(QWidget *parent);
};

/**
 * This class extends QTabBar such that right-clicking on a tab pops-up a menu
 * containing relevant actions for the tab. It also emits signals to close the
 * current tab.
 */
class ViewerTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit ViewerTabBar(QWidget *parent) : QTabBar(parent) {};

protected:
    void mousePressEvent(QMouseEvent *) override;

signals:
    /**
     * emitted when the user mid-clicks on the tab
     */
    void closeTabSignal(int index);

};

#endif
