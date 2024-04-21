/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    Based on KrRemoteEncodingMenu (© Csaba Karai and Krusader Krew)
    from Krusader, DolphinRecentTabsMenu (© Emmanuel Pescosta)
    from Dolphin and PanelManager::slotDuplicateTab() (© Shie Erlich,
    Rafi Yanai and Krusader Krew) from Krusader.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "recentlyclosedtabsmenu.h"

#include "../Panel/PanelView/krview.h"
#include "../Panel/listpanel.h"
#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../tabactions.h"

#include <KActionCollection>
#include <KConfig>
#include <KIO/Global>
#include <KLocalizedString>

// QtWidgets
#include <QMenu>

RecentlyClosedTabsMenu::RecentlyClosedTabsMenu(const QString &text, const QString &iconName, KActionCollection *parent)
    : KActionMenu(Icon(iconName), text, parent)
{
    parent->addAction("recently_closed_tabs", this);

    // Construct the Empty Recently Closed Tabs action and add it to the menu
    actClearTheList = new QAction(i18n("Empty Recently Closed Tabs"), this);
    actClearTheList->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-list")));
    addAction(actClearTheList);
    // Add a separator after that menu entry
    addSeparator();

    connect(menu(), &QMenu::triggered, this, &RecentlyClosedTabsMenu::slotTriggered);
}

QAction *RecentlyClosedTabsMenu::updateAfterClosingATab(const QUrl &urlClosedTab, const QByteArray &backedUpData, TabActions *argTabActions)
{
    // Create the related action
    QAction *actReopenTab = new QAction(menu());
    actReopenTab->setText(urlClosedTab.path());
    actReopenTab->setData(backedUpData);
    const QString iconName = KIO::iconNameForUrl(urlClosedTab);
    actReopenTab->setIcon(QIcon::fromTheme(iconName));

    // Add a menu entry (related to the closed tab) after the
    // fixed menu entries
    if (menu()->actions().size() == quantFixedMenuEntries) {
        addAction(actReopenTab);
    } else {
        insertAction(menu()->actions().at(quantFixedMenuEntries), actReopenTab);
    }

    // Remove the last entry of the menu if the limit is exceeded
    if (menu()->actions().size() > quantFixedMenuEntries + maxClosedTabs) {
        ACTIVE_MNG->delClosedTab(menu()->actions().last());
    }

    // Enable objects
    setEnabled(true);
    tabActions = argTabActions;
    tabActions->actUndoCloseTab->setEnabled(true);

    return actReopenTab;
}

void RecentlyClosedTabsMenu::slotTriggered(QAction *action)
{
    if (!action)
        return;

    if (action == actClearTheList) {
        ACTIVE_MNG->delAllClosedTabs();
    } else {
        ACTIVE_MNG->undoCloseTab(action);

        removeAction(action);
        delete action;
        action = nullptr;
    }

    if (menu()->actions().size() <= quantFixedMenuEntries) {
        // Disable objects
        tabActions->actUndoCloseTab->setEnabled(false);
        setEnabled(false);
    }
}
