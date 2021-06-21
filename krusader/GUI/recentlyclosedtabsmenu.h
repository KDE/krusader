/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2005-2020 Krusader Krew <https://krusader.org>

    Based on KrRemoteEncodingMenu (© Csaba Karai and Krusader Krew)
    from Krusader and DolphinRecentTabsMenu (© Emmanuel Pescosta)
    from Dolphin.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECENTLYCLOSEDTABSMENU_H
#define RECENTLYCLOSEDTABSMENU_H

#include <KWidgetsAddons/KActionMenu>

class KActionCollection;
class TabActions;

/**
 * A menu to perform operations with a list of recently closed tabs
 */
class RecentlyClosedTabsMenu: public KActionMenu
{
    Q_OBJECT
    friend class PanelManager;

public:
    explicit RecentlyClosedTabsMenu(const QString &text, const QString &iconName,
                                    KActionCollection *parent = nullptr);

protected slots:
    void slotTriggered(QAction *action);

private:
    QAction *updateAfterClosingATab(const QUrl &urlClosedTab, const QByteArray &backedUpData,
                                    TabActions *argTabActions);
    QAction *actClearTheList;
    TabActions *tabActions = nullptr;
};

#endif // RECENTLYCLOSEDTABSMENU_H
