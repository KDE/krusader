/***************************************************************************
                          usermenuaddimpl.cpp  -  description
                             -------------------
    begin                : Fri Dec 26 2003
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

#include "usermenuaddimpl.h"
#include <klocale.h>
#include <qtoolbutton.h>

#define EXECUTABLE_ID   10

UserMenuAddImpl::UserMenuAddImpl(QWidget *parent, const char *name ) : UserMenuAdd(parent,name) {
   // create the 'add' popup menu
   _popup = new KPopupMenu(this);
   _activeSub = new KPopupMenu(_popup);
   _otherSub = new KPopupMenu(_popup);

   _popup->insertItem(i18n("Active panel"), _activeSub);
   _popup->insertItem(i18n("Other panel"), _otherSub);
   _popup->insertItem(i18n("Executable"), EXECUTABLE_ID);
}
UserMenuAddImpl::~UserMenuAddImpl(){
}

void UserMenuAddImpl::accept() {
   emit newEntry("testname", "test cmdline", UserMenuProc::Terminal, false,
                  false, false, false);
   UserMenuAdd::accept();
}

void UserMenuAddImpl::popupAddMenu() {
   _popup->exec(mapToGlobal(QPoint(addButton->pos().x()+addButton->width(), addButton->pos().y())));
}
