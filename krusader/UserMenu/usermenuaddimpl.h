/***************************************************************************
                          usermenuaddimpl.h  -  description
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

#ifndef USERMENUADDIMPL_H
#define USERMENUADDIMPL_H

#include <qwidget.h>
#include <usermenuadd.h>
#include <qstringlist.h>
#include "usermenu.h"

class KPopupMenu;

class UserMenuAddImpl : public UserMenuAdd  {
   Q_OBJECT
public:
	UserMenuAddImpl(QWidget *parent=0, const char *name=0);
	~UserMenuAddImpl();


public slots:
   void accept();
   void popupAddMenu();

signals:
   void newEntry(QString name, QString cmdline, UserMenuProc::ExecType execType, bool separateStderr,
                  bool acceptURLs, bool acceptRemote, bool showEverywhere, QStringList showIn = 0);

private:
   KPopupMenu *_popup, *_activeSub, *_otherSub;
};

#endif
