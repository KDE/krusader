/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2011 Jan Lepper <jan_lepper@gmx.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabactions.h"

#include "Panel/listpanel.h"
#include "krmainwindow.h"
#include "panelmanager.h"

// QtWidgets
#include <QAction>

#include <KLocalizedString>

TabActions::TabActions(QObject *parent, KrMainWindow *mainWindow)
    : ActionsBase(parent, mainWindow)
{
    actNewTab = action(i18n("New Tab"), "tab-new", Qt::ALT | Qt::CTRL | Qt::SHIFT | Qt::Key_N, SLOT(newTab()), "new_tab");
    actDupTab =
        action(i18n("Duplicate Current Tab"), "tab-duplicate", QKeySequence::keyBindings(QKeySequence::AddTab), this, SLOT(duplicateTab()), "duplicate tab");
    actMoveTabToOtherSide =
        action(i18n("Move Current Tab to Other Side"), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_O, SLOT(moveTabToOtherSide()), "move_tab_to_other_side");
    actMoveTabToLeft = action(i18n("Move Current Tab to the Left"), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_PageUp, SLOT(moveTabToLeft()), "move_tab_to_left");
    actMoveTabToRight =
        action(i18n("Move Current Tab to the Right"), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_PageDown, SLOT(moveTabToRight()), "move_tab_to_right");
    actCloseTab = action(i18n("Close Current Tab"), "tab-close", KStandardShortcut::close(), this, SLOT(closeTab()), "close tab");
    actUndoCloseTab = action(i18n("Undo Close Tab"), "edit-undo", Qt::CTRL | Qt::SHIFT | Qt::Key_T, this, SLOT(undoCloseTab()), "undo_close_tab");
    actUndoCloseTab->setEnabled(false);
    actNextTab = action(i18n("Next Tab"), QString(), KStandardShortcut::tabNext(), this, SLOT(nextTab()), "next tab");
    actPreviousTab = action(i18n("Previous Tab"), QString(), KStandardShortcut::tabPrev(), this, SLOT(previousTab()), "previous tab");
    actCloseInactiveTabs = action(i18n("Close Inactive Tabs"), nullptr, 0, SLOT(closeInactiveTabs()), "close inactive tabs");
    actCloseDuplicatedTabs = action(i18n("Close Duplicated Tabs"), nullptr, 0, SLOT(closeDuplicatedTabs()), "close duplicated tabs");
    actLockTab = action(i18n("Lock Tab"), "lock", 0, SLOT(lockTab()), "lock tab");
    actPinTab = action(i18n("Pin Tab"), "pin", 0, SLOT(pinTab()), "pin tab");
}

inline PanelManager *TabActions::activeManager()
{
    return dynamic_cast<PanelManager *>(static_cast<KrMainWindow *>(_mainWindow)->activeManager());
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
    actMoveTabToLeft->setEnabled(tabCount > 1);
    actMoveTabToRight->setEnabled(tabCount > 1);
    actNextTab->setEnabled(tabCount > 1);
    actPreviousTab->setEnabled(tabCount > 1);
    bool locked = activeManager()->currentPanel()->gui->isLocked();
    actLockTab->setText(locked ? i18n("Unlock Tab") : i18n("Lock Tab"));
    bool pinned = activeManager()->currentPanel()->gui->isPinned();
    actPinTab->setText(pinned ? i18n("Unpin Tab") : i18n("Pin Tab"));
}

void TabActions::newTab()
{
    activeManager()->slotNewTab();
}

void TabActions::duplicateTab()
{
    KrPanel *activePanel = static_cast<KrMainWindow *>(_mainWindow)->activePanel();
    activeManager()->duplicateTab(activePanel->virtualPath(), activePanel);
}

void TabActions::moveTabToOtherSide()
{
    activeManager()->moveTabToOtherSide();
}

void TabActions::moveTabToLeft()
{
    activeManager()->moveTabToLeft();
}

void TabActions::moveTabToRight()
{
    activeManager()->moveTabToRight();
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

void TabActions::undoCloseTab()
{
    activeManager()->slotUndoCloseTab();
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

void TabActions::pinTab()
{
    activeManager()->slotPinTab();
}
