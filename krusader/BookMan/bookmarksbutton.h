/***************************************************************************
                          bookmarksbutton.h  -  description
                             -------------------
    begin                : Ne jún 8 2003
    copyright            : (C) 2003 by Ján Hala?a
    email                : xhalasa@fi.muni.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BOOKMARKSBUTTON_H
#define BOOKMARKSBUTTON_H

#include "kbookmarkhandler.h"
#include <qwidget.h>
#include <ktoolbar.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <kpopupmenu.h>

/**
  *@author Ján Hala¹a
  */

class BookmarksButton : public QToolButton {
  Q_OBJECT
  
public:
  BookmarksButton(QWidget *parent);
  ~BookmarksButton();

  void openPopup();
private:
  KBookmarkHandler *bookmarkHandler;
signals:
  void openUrl(const QString& url);
};

#endif
