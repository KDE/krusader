/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERACTIONPOPUPMENU_H
#define USERACTIONPOPUPMENU_H

#include <KActionMenu>

class QUrl;

class UserActionPopupMenu : public KActionMenu
{
public:
    explicit UserActionPopupMenu(const QUrl &currentURL, QWidget *parent = nullptr);
};

#endif // USERACTIONPOPUPMENU_H
