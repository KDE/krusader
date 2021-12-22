/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2005-2021 Krusader Krew <https://krusader.org>

    Based on KrRemoteEncodingMenu (© Csaba Karai and Krusader Krew)
    from Krusader, DolphinRecentTabsMenu (© Emmanuel Pescosta)
    from Dolphin and PanelManager::slotDuplicateTab() (© Shie Erlich,
    Rafi Yanai and Krusader Krew) from Krusader.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "recentlyclosedtabsmenu.h"

#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../tabactions.h"
#include "../Panel/listpanel.h"
#include "../Panel/PanelView/krview.h"

#include <KConfigCore/KConfig>
#include <KI18n/KLocalizedString>
#include <KIO/Global>
#include <KXmlGui/KActionCollection>

// QtWidgets
#include <QMenu>

RecentlyClosedTabsMenu::RecentlyClosedTabsMenu(const QString &text, const QString &iconName, KActionCollection *parent) :
        KActionMenu(Icon(iconName), text, parent)
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

QAction *RecentlyClosedTabsMenu::updateAfterClosingATab(const QUrl &urlClosedTab, const QByteArray &backedUpData,
                                                        TabActions *argTabActions)
{
    // Create the related action
    QAction *actReopenTab = new QAction(menu());
    actReopenTab->setText(urlClosedTab.path());
    actReopenTab->setData(backedUpData);
    const QString iconName = KIO::iconNameForUrl(urlClosedTab);
    actReopenTab->setIcon(QIcon::fromTheme(iconName));

    // Add a menu entry (related to the closed tab) after the
    // separator and the "Empty Recently Closed Tabs" entry
    if (menu()->actions().size() == 2) {
        addAction(actReopenTab);
    } else {
        insertAction(menu()->actions().at(2), actReopenTab);
    }

    // Remove the last entry of the menu if there are more than
    // six (8 - 2) closed tabs there
    if (menu()->actions().size() > 8) {
        removeAction(menu()->actions().last());
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
        // Remove the actions that follow those two items:
        // the "Empty Recently Closed Tabs" one and a separator
        const int quantActions = menu()->actions().size();
        for (int x = quantActions - 1; x >= 2; x--) {
            removeAction(menu()->actions().at(x));
        }
    } else {
        ACTIVE_MNG->undoCloseTab(action);

        removeAction(action);
        delete action;
        action = nullptr;
    }

    if (menu()->actions().size() <= 2) {
        // Disable objects
        tabActions->actUndoCloseTab->setEnabled(false);
        setEnabled(false);
    }
}
