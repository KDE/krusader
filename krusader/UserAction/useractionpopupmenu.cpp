/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "useractionpopupmenu.h"

// QtCore
#include <QUrl>

#include <KI18n/KLocalizedString>

#include "../krglobal.h"
#include "useraction.h"
#include "kraction.h"

UserActionPopupMenu::UserActionPopupMenu(const QUrl &currentURL, QWidget *parent)
        : KActionMenu(i18n("User Actions"), parent)
{
    krUserAction->populateMenu(this, &currentURL);
}
