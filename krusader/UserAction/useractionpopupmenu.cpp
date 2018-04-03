/*****************************************************************************
 * Copyright (C) 2004 Jonas Bähr <jonas.baehr@web.de>                        *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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
