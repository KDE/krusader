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
#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"
#include "../UserAction/useractionxml.h"
#include "usermenu.h"
#include "usermenuadd.h"


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

UserMenuGui::UserMenuGui( UserMenu *menu, QWidget * parent ) : KPopupMenu( parent ) {
   createMenu();

   // create the 'add entry' gui
   _addgui = new UserMenuAdd(this);
   connect( _addgui, SIGNAL( newEntry( UserActionProperties* ) ), this, SLOT( addEntry( UserActionProperties* ) ) );
}

void UserMenuGui::createMenu() {
//    kdDebug() << "UserMenuGui::createMenu called" << endl;
   clear();
   insertTitle( i18n("User Menu") );

   // read entries from config file.
   readEntries();

   // add the "add new entry" command
   insertSeparator();
   insertItem( i18n("Add new entry"), 0 );
}

void UserMenuGui::readEntries() {
   // Note: entries are marked 1..n, so that entry 0 is always
   // available. It is used by the "add new entry" command.
   int idx = 1;

   //FIXME: don't plug ALL useractions into the usermenu. TODO: read the usermenu-strukture from an other file (krusaderrc ?)
   for ( UserAction::KrActionList::iterator it = krUserAction->actionList()->begin(); it != krUserAction->actionList()->end(); ++it )
      ( *it )->plug( this, idx++ );

}

void UserMenuGui::run() {
   //disable unwanted actions:
   krApp->userAction->setAvailability();

   int idx = exec();
   if ( idx == -1 ) // nothing was selected
     return;
   if ( idx == 0 ) {
      _addgui->exec();
      return;
   }
}

void UserMenuGui::addEntry( UserActionProperties *properties ) {
  //kdDebug() << "UserMenuGui::addEntry called" << endl;

  // add the new action to the xml
  krUserAction->xml()->addActionToDom( properties );
  krUserAction->xml()->writeActionDom();
  krUserAction->addKrAction( properties );
  
   // update the menu, by re-reading the entries
   createMenu();
   //kdDebug() << "UserMenuGui::addEntry finished" << endl;
}

