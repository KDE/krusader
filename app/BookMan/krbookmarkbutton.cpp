/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krbookmarkbutton.h"
#include "../compat.h"
#include "../icon.h"
#include "../krglobal.h"
#include "krbookmarkhandler.h"

// QtGui
#include <QPixmap>
// QtWidgets
#include <QAction>
#include <QMenu>

#include <KLocalizedString>

KrBookmarkButton::KrBookmarkButton(QWidget *parent)
    : QToolButton(parent)
{
    setAutoRaise(true);
    setIcon(Icon("bookmarks"));
    setText(i18n("BookMan II"));
    setToolTip(i18n("BookMan II"));
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);

    acmBookmarks = new KActionMenu(Icon("bookmarks"), i18n("Bookmarks"), this);
    acmBookmarks->KACTIONMENU_SETDELAYED;

    setMenu(acmBookmarks->menu());
    connect(acmBookmarks->menu(), &QMenu::aboutToShow, this, &KrBookmarkButton::populate);
    connect(acmBookmarks->menu(), &QMenu::aboutToShow, this, &KrBookmarkButton::aboutToShow);
}

void KrBookmarkButton::populate()
{
    krBookMan->populate(static_cast<QMenu *>(menu()));
}

void KrBookmarkButton::showMenu()
{
    populate();
    menu()->exec(mapToGlobal(QPoint(0, height())));
}
