/***************************************************************************
                                bookman.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BOOKMAN_H
#define BOOKMAN_H 
  
#include <qwidget.h>
#include <qsortedlist.h>
#include <qstring.h>
#include "bookmangui.h"

class QPopupMenu;
class KRBookmark; // forward declaration

class BookMan : public QWidget {
  friend class BookManGUI;
  Q_OBJECT
public:
  BookMan();
 ~BookMan(); 
  void fillPopupMenu(QPopupMenu *menu);  // fill-up the menu with the bookmarks
  QString getUrlById(const int id);      // find a url by it's popupmenu id
  KRBookmark* find(const QString &_name);// find a bookmark named: _name
  void newBookmark(const QString &_name, const QString &_url);
  void loadBookmarks();
  void saveBookmarks();

signals:
  void refreshBookmarks();

public slots:  
  void addBookmark(QString path){ new BookManGUI(path); }
  void showGUI() { new BookManGUI(""); }

private:
  QList<KRBookmark>       bookmarks;
};
 
class KRBookmark {
public:
  KRBookmark(QString _name,QString _url);
  inline int  getId() { return menuId; }
  inline void setId(int id) { menuId=id; }
  inline QString getName() { return name; }
  inline QString getUrl() { return url; }
  inline void setName(QString _name) { name=_name; }
  inline void setUrl(QString _url) { url=_url; }
  inline bool operator==(KRBookmark &b) { return getName()==b.getName(); }
  inline bool operator<(KRBookmark &b) { return getName()<b.getName(); }
  inline bool operator>(KRBookmark &b) { return getName()>b.getName(); }
  inline bool operator==(QString &b) { return getName()==b; }
  friend bool operator==(QString &b, KRBookmark &a);
   
private:
  QString name,url;
  int menuId;
};

#endif
