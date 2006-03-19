/***************************************************************************
                      _expressionsu.cpp  -  description
                         -------------------
begin                : Sat Dec 6 2003
copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include "../krusader.h"
#include "../Konfigurator/konfigurator.h"
#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"
#include "usermenu.h"


void UserMenu::exec() {
   _popup->run();
}

UserMenu::UserMenu( QWidget * parent, const char * name ) : QWidget( parent, name ) {
   _popup = new UserMenuGui(this);
}

void UserMenu::update() {
   _popup->createMenu();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserMenuGui::UserMenuGui( UserMenu *, QWidget * parent ) : KPopupMenu( parent ) {
   createMenu();
}

void UserMenuGui::createMenu() {
//    kdDebug() << "UserMenuGui::createMenu called" << endl;
   clear();
   insertTitle( i18n("User Menu") );

   // read entries from config file.
   readEntries();

   // add the "add new entry" command
   insertSeparator();
   insertItem( i18n("Manage user actions"), 0 );
}

void UserMenuGui::readEntries() {
   // Note: entries are marked 1..n, so that entry 0 is always
   // available. It is used by the "add new entry" command.
   int idx = 1;

   //FIXME: don't plug ALL useractions into the usermenu. TODO: read the usermenu-strukture from an other file (krusaderrc ?)
   UserAction::KrActionList list = krUserAction->actionList();
   for ( KrAction* action = list.first(); action; action = list.next() )
      action->plug( this, idx++ );

}

void UserMenuGui::run() {
   //disable unwanted actions:
   // disabled due to conflicts with the toolbar (a check on each file-cursor-movement would be nessesary; hit the performance)
//    krApp->userAction->setAvailability();

   int idx = exec();
   if ( idx == -1 ) // nothing was selected
     return;
   if ( idx == 0 ) {
      Konfigurator konfigurator( false, 7 ); // page 7 are the UserActions
      return;
   }
}
