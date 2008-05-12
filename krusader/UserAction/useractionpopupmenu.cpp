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

UserActionPopupMenu::UserActionPopupMenu( const KUrl &currentURL, QWidget *parent ) : KMenu( parent ) {
   krUserAction->populateMenu( this, &currentURL );
}
