/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2011 Jan Lepper <jan_lepper@gmx.de>                         *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef TABACTIONS_H
#define TABACTIONS_H

#include "actionsbase.h"

class KrMainWindow;
class PanelManager;

class TabActions : public ActionsBase
{
    friend class PanelTabBar;

    Q_OBJECT

public:
    TabActions(QObject *parent, KrMainWindow *mainWindow);

public slots:
    void refreshActions();

protected slots:
    void newTab();
    void duplicateTab();
    void lockTab();
    void pinTab();
    void closeTab();
    void nextTab();
    void previousTab();
    void closeInactiveTabs();
    void closeDuplicatedTabs();
    void moveTabToOtherSide();

protected:
    inline PanelManager *activeManager();

    QAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actMoveTabToOtherSide;
    QAction *actCloseInactiveTabs, *actCloseDuplicatedTabs, *actLockTab, *actPinTab;
};


#endif // TABACTIONS_H
