//
// C++ Implementation: UserActionPopupMenu
//
// Description: 
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "useractionpopupmenu.h"

#include <kurl.h>

#include "../krusader.h"
#include "useraction.h"
#include "kraction.h"

UserActionPopupMenu::UserActionPopupMenu( KURL currentURL, QWidget *parent ) : KPopupMenu( parent, "useraction popupmenu" ) {
   UserAction::KrActionList list = krUserAction->actionList();
   for ( KrAction* action = list.first(); action; action = list.next() )
      if ( action->isAvailable( currentURL ) )
         action->plug( this );
}
