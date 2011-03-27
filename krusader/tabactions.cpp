/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2011 Jan Lepper <jan_lepper@gmx.de>                         *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "tabactions.h"

#include "filemanagerwindow.h"
#include "panelmanager.h"
#include "Panel/listpanel.h"

#include <klocale.h>
#include <kaction.h>

TabActions::TabActions(QObject *parent, FileManagerWindow *mainWindow) : ActionsBase(parent, mainWindow)
{
    actNewTab = action(i18n("New Tab"), "tab-new", Qt::ALT + Qt::CTRL + Qt::Key_N, SLOT(newTab()), "new tab");
    actDupTab = action(i18n("Duplicate Current Tab"), "tab_duplicate", Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_N, SLOT(duplicateTab()), "duplicate tab");
    actMoveTabToOtherSide = action(i18n("Move Current Tab to Other Side"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_O, SLOT(moveTabToOtherSide()), "move_tab_to_other_side");
    actCloseTab = action(i18n("Close Current Tab"), "tab-close", Qt::CTRL + Qt::Key_W, SLOT(closeTab()), "close tab");
    actNextTab = action(i18n("Next Tab"), 0, Qt::SHIFT + Qt::Key_Right, SLOT(nextTab()), "next tab");
    actPreviousTab = action(i18n("Previous Tab"), 0, Qt::SHIFT + Qt::Key_Left, SLOT(previousTab()), "previous tab");
    actCloseInactiveTabs = action(i18n("Close Inactive Tabs"), 0, 0, SLOT(closeInactiveTabs()), "close inactive tabs");
    actCloseDuplicatedTabs = action(i18n("Close Duplicated Tabs"), 0, 0, SLOT(closeDuplicatedTabs()), "close duplicated tabs");
    actLockTab = action(i18n("Lock Tab"), 0, 0, SLOT(lockTab()), "lock tab");
}

inline PanelManager *TabActions::activeManager()
{
    return static_cast<PanelManager*>(
        static_cast<FileManagerWindow*>(_mainWindow)->activeManager());
}

void TabActions::refreshActions()
{
    if (!activeManager())
        return;
    int tabCount = activeManager()->tabCount();
    actCloseTab->setEnabled(tabCount > 1);
    actCloseInactiveTabs->setEnabled(tabCount > 1);
    actCloseDuplicatedTabs->setEnabled(tabCount > 1);
    actMoveTabToOtherSide->setEnabled(tabCount > 1);
    actNextTab->setEnabled(tabCount > 1);
    actPreviousTab->setEnabled(tabCount > 1);
    bool locked = activeManager()->currentPanel()->gui->isLocked();
    actLockTab->setText(locked ? i18n("Unlock Tab") : i18n("Lock Tab"));
}

void TabActions::newTab()
{
    activeManager()->slotNewTab();
}

void TabActions::duplicateTab()
{
    KrPanel *activePanel = static_cast<FileManagerWindow*>(_mainWindow)->activePanel();
    activeManager()->newTab(activePanel->virtualPath(), activePanel);
}

void TabActions::moveTabToOtherSide()
{
    activeManager()->moveTabToOtherSide();
}

void TabActions::nextTab()
{
    activeManager()->slotNextTab();
}

void TabActions::previousTab()
{
    activeManager()->slotPreviousTab();
}

void TabActions::closeTab()
{
    activeManager()->slotCloseTab();
}

void TabActions::closeInactiveTabs()
{
    activeManager()->slotCloseInactiveTabs();
}

void TabActions::closeDuplicatedTabs()
{
    activeManager()->slotCloseDuplicatedTabs();
}

void TabActions::lockTab()
{
    activeManager()->slotLockTab();
}
