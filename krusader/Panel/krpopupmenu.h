/***************************************************************************
                                krpopupmenu.h 
                             -------------------
    begin                : Tue Aug 26 2003
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

#ifndef KRPOPUPMENU_H
#define KRPOPUPMENU_H

#include <qwidget.h>
#include <qpopupmenu.h>

class KrPopupMenu : public QPopupMenu  {
   Q_OBJECT
public: 
	KrPopupMenu(QWidget *parent=0, const char *name=0);
	~KrPopupMenu();

//  void populate(KURL::List urls);

protected:
  enum ID{
    OPEN_ID       = 90,
    OPEN_WITH_ID  = 91,
    OPEN_KONQ_ID  = 92,
    OPEN_TERM_ID  = 93,
    CHOOSE_ID     = 94,
    DELETE_ID     = 95,
    COPY_ID       = 96,
    MOVE_ID       = 97,
    RENAME_ID     = 98,
    PROPERTIES_ID = 99,
    MOUNT_ID      = 100,
    UNMOUNT_ID    = 101,
    SHRED_ID      = 102,
    NEW_LINK      = 103,
    NEW_SYMLINK   = 104,
    REDIRECT_LINK = 105,
    SEND_BY_EMAIL = 106,
    LINK_HANDLING = 107,
    EJECT_ID      = 108,
    PREVIEW_ID    = 109,
  // those will sometimes appear
    SERVICE_LIST_ID  = 200
  };
};

#endif
