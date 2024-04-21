/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    Based on KrRemoteEncodingMenu (© Csaba Karai and Krusader Krew)
    from Krusader and DolphinRecentTabsMenu (© Emmanuel Pescosta)
    from Dolphin.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECENTLYCLOSEDTABSMENU_H
#define RECENTLYCLOSEDTABSMENU_H

#include <KActionMenu>

class KActionCollection;
class TabActions;

/**
 * A menu to perform operations with a list of recently closed tabs
 */
class RecentlyClosedTabsMenu : public KActionMenu
{
    Q_OBJECT
    friend class PanelManager;

public:
    explicit RecentlyClosedTabsMenu(const QString &text, const QString &iconName, KActionCollection *parent = nullptr);

protected slots:
    void slotTriggered(QAction *action);

private:
    QAction *updateAfterClosingATab(const QUrl &urlClosedTab, const QByteArray &backedUpData, TabActions *argTabActions);
    QAction *actClearTheList;
    TabActions *tabActions = nullptr;

    //! Krusader saves information about closed tabs until its quantity reaches that limit
    const int maxClosedTabs = 6;

    //! The quantity of fixed menu entries (the "Empty Recently Closed Tabs" one and a separator)
    const int quantFixedMenuEntries = 2;
};

#endif // RECENTLYCLOSEDTABSMENU_H
