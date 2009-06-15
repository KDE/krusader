/*****************************************************************************
 * Copyright (C) 2003 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2003 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include "usermenu.h"

#include <kdebug.h>
#include <klocale.h>

#include "../krusader.h"
#include "../Konfigurator/konfigurator.h"
#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"

void UserMenu::exec()
{
    _popup->run();
}

UserMenu::UserMenu(QWidget * parent) : QWidget(parent)
{
    _popup = new UserMenuGui(this);
}

void UserMenu::update()
{
    _popup->createMenu();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserMenuGui::UserMenuGui(UserMenu *, QWidget * parent) : KMenu(parent)
{
    createMenu();
}

void UserMenuGui::createMenu()
{
//    kDebug() << "UserMenuGui::createMenu called" << endl;
    clear();
    setTitle(i18n("User Menu"));

    // read entries from config file.
    readEntries();

    // add the "add new entry" command
    addSeparator();
    _manageAction = addAction(i18n("Manage user actions"));
}

void UserMenuGui::readEntries()
{
    // Note: entries are marked 1..n, so that entry 0 is always
    // available. It is used by the "add new entry" command.

    //FIXME: don't plug ALL useractions into the usermenu. TODO: read the usermenu-strukture from an other file (krusaderrc ?)
    UserAction::KrActionList list = krUserAction->actionList();

    QListIterator<KrAction *> it(list);
    while (it.hasNext())
        addAction(it.next());

}

void UserMenuGui::run()
{
    //disable unwanted actions:
    // disabled due to conflicts with the toolbar
    // (a check on each file-cursor-movement would be necessary;
    // hit the performance)
    // krApp->userAction->setAvailability();

    QAction * act = exec();
    if (act == 0)   // nothing was selected
        return;
    if (act == _manageAction) {
        Konfigurator konfigurator(false, 7);   // page 7 are the UserActions
        return;
    }
}
