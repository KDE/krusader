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

#include <kmenu.h>

class KUrl;

class UserActionPopupMenu : public KMenu {
public:
   UserActionPopupMenu( const KUrl &currentURL, QWidget *parent = 0 );
};

#endif // ifndef USERACTIONPOPUPMENU_H
