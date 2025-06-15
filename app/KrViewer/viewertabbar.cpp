/*
    SPDX-FileCopyrightText: 2020-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewertabbar.h"

// QtGui
#include <QMouseEvent>

#include <KSharedConfig>

#include "../defaults.h"
#include "../krglobal.h"

ViewerTabWidget::ViewerTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabBar(new ViewerTabBar(this));
}

void ViewerTabWidget::adjustViewerTabBarVisibility()
{
    if (count() > 1) {
        tabBar()->show();
    } else if (count() == 1) {
        KConfigGroup group(krConfig, "General");
        bool hideSingleTab = group.readEntry("Viewer Hide Single Tab", _ViewerHideSingleTab);
        if (hideSingleTab)
            tabBar()->hide();
    }
    return;
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

    if (e->button() == Qt::MiddleButton) {
        // close the current tab
        emit closeTabSignal(clickedTab);
    }

    QTabBar::mousePressEvent(e);
}
