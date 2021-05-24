/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2011 Jan Lepper <jan_lepper@gmx.de>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew [https://krusader.org]

    This file is part of Krusader [https://krusader.org].

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    void moveTabToLeft();
    void moveTabToRight();

protected:
    inline PanelManager *activeManager();

    QAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actMoveTabToOtherSide;
    QAction *actCloseInactiveTabs, *actCloseDuplicatedTabs, *actLockTab, *actPinTab;
    QAction *actMoveTabToLeft, *actMoveTabToRight;
};


#endif // TABACTIONS_H
