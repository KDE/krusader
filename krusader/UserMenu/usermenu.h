/***************************************************************************
                          usermenu.h  -  description
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

#ifndef USERMENU_H
#define USERMENU_H

#include <qwidget.h>
#include <kpopupmenu.h>
#include <qstringlist.h>

// an expander is a function that receives a QString input, expands
// it and returns a new QString output containing the expanded expression
typedef QString (EXPANDER)(const QString&);

// a UMCmd is an entry containing the expression and its expanding function
typedef struct UserMenuCmd {
  QString lc_expression;
  EXPANDER expFunc;
} UMCmd;

class UserMenu : public QWidget  {
#define NUM_EXPS  1

   Q_OBJECT
public:
  QString exec();
  UserMenu(QWidget *parent=0, const char *name=0);
	~UserMenu();

protected:
  QString expand(QString str);
  QString expPath(const QString& str);

private:
  KPopupMenu _popup;
  QStringList _entries;
  static UMCmd _expressions[NUM_EXPS];
};

#endif
