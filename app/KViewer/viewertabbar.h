/*
    SPDX-FileCopyrightText: 2020-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWERTABBAR_H
#define VIEWERTABBAR_H

// QtWidgets
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

class ViewerTabBar;

/**
 * This class extends QTabWidget such that we can use a custom QTabBar on it
 */
class ViewerTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit ViewerTabWidget(QWidget *parent);
    void adjustViewerTabBarVisibility();

    ViewerTabBar *tabBar() const;
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
    explicit ViewerTabBar(QWidget *parent)
        : QTabBar(parent){};

protected:
    void mousePressEvent(QMouseEvent *) override;

signals:
    /**
     * emitted when the user mid-clicks on the tab
     */
    void closeTabSignal(int index);
};

#endif
