/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "krbookmarkbutton.h"
#include "krbookmarkhandler.h"
#include "../krglobal.h"
#include "../icon.h"

// QtGui
#include <QPixmap>
// QtWidgets
#include <QAction>
#include <QMenu>

#include <KI18n/KLocalizedString>


KrBookmarkButton::KrBookmarkButton(QWidget *parent): QToolButton(parent)
{
    setAutoRaise(true);
    setIcon(Icon("bookmarks"));
    setText(i18n("BookMan II"));
    setToolTip(i18n("BookMan II"));
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);

    acmBookmarks = new KActionMenu(Icon("bookmarks"), i18n("Bookmarks"), this);
    acmBookmarks->setDelayed(false);

    setMenu(acmBookmarks->menu());
    connect(acmBookmarks->menu(), &QMenu::aboutToShow, this, &KrBookmarkButton::populate);
    connect(acmBookmarks->menu(), &QMenu::aboutToShow, this, &KrBookmarkButton::aboutToShow);
}

void KrBookmarkButton::populate()
{
    krBookMan->populate(static_cast<QMenu*>(menu()));
}

void KrBookmarkButton::showMenu()
{
    populate();
    menu()->exec(mapToGlobal(QPoint(0, height())));
}

