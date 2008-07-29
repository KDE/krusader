//
// C++ Interface: UserActionPopupMenu
//
// Description: 
//
//
// Author: Jonas Bähr, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTIONPOPUPMENU_H
#define USERACTIONPOPUPMENU_H

#include <kactionmenu.h>

class KUrl;

class UserActionPopupMenu : public KActionMenu {
public:
   UserActionPopupMenu( const KUrl &currentURL, QWidget *parent = 0 );
};

#endif // ifndef USERACTIONPOPUPMENU_H
