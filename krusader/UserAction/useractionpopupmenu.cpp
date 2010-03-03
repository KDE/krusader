/*****************************************************************************
 * Copyright (C) 2004 Jonas Bähr <jonas.baehr@web.de>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "useractionpopupmenu.h"

#include <kurl.h>
#include <klocale.h>

#include "../krglobal.h"
#include "useraction.h"
#include "kraction.h"

UserActionPopupMenu::UserActionPopupMenu(const KUrl &currentURL, QWidget *parent)
        : KActionMenu(i18n("User Actions"), parent)
{
    krUserAction->populateMenu(this, &currentURL);
}
