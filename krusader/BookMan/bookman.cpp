/***************************************************************************
                                bookman.cpp
                             -------------------
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <klocale.h>
#include <qpopupmenu.h>

#include "../krusader.h"
#include "../resources.h"
#include "../defaults.h"

#include "bookman.h"

BookMan::BookMan() {
  // remove items from list if they are deleted ... avoid memory leaks
  bookmarks.setAutoDelete(true);
  loadBookmarks();
}

BookMan::~BookMan() {
  while (bookmarks.removeFirst());  // kill the list, free memory    
}

// given a popup menu, this function clears it and fills it with bookmarks
void BookMan::fillPopupMenu(QPopupMenu *menu) {
  krConfig->setGroup("Bookmarks");
  bool sorted=krConfig->readBoolEntry("Sorted",_Sorted);
  // create a QSortedList, we may need it
  QSortedList<KRBookmark> sbookmarks;
  if (sorted) { // if we need a sorted list, we create a copy and sort it!
    KRBookmark *i;
    for ( i=bookmarks.first(); i != 0; i=bookmarks.next() )
      sbookmarks.append(i);
    sbookmarks.sort();
  }
  menu->clear();   // just in case
  KRBookmark *it;
  int i=0;
  for ( it=(sorted ? sbookmarks : bookmarks).first(); it != 0;
        it=(sorted ? sbookmarks : bookmarks).next() ) {
    menu->insertItem((*it).getName(), i);
    (*it).setId(i);
    i++;
  }
}

QString BookMan::getUrlById(const int id) {
  KRBookmark *it;
  for ( it=bookmarks.first() ; it != 0; it=bookmarks.next() )
    if ((*it).getId() == id) return (*it).getUrl();
  return 0;
}

KRBookmark* BookMan::find(const QString &_name) {
 KRBookmark *i;
  for ( i=bookmarks.first(); i != 0; i=bookmarks.next() )
    if ((*i).getName()==_name) return i;
  // if we got here ... none was found
  return 0;
}

// This function does the real job of adding a new bookmark, whereis
// addBookmark just pops the GUI
void BookMan::newBookmark(const QString &_name, const QString &_url) {
  // append bookmark to the list
  KRBookmark *temp=new KRBookmark(_name,_url);
  bookmarks.append(temp);
  // save the bookmarks
  saveBookmarks();
  // last, force a bookmark refresh on the "list" panels
  emit refreshBookmarks();
}

void BookMan::loadBookmarks() {
  krConfig->setGroup("Bookmarks");
  ////////////////////////////////
  QStringList list=krConfig->readListEntry("Links");
  QString name,url;

  if (list.count()==0) list << i18n("Root (/)") << "/";
  // create the KRBookmark list
  bookmarks.clear();
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
    name = *(it++);
    url = *it;
    bookmarks.append(new KRBookmark(name,url));
  }
}

void BookMan::saveBookmarks() {
  QStringList list;

  krConfig->setGroup("Bookmarks");
  ////////////////////////////////
  // traverse the list of bookmarks and create a string list which
  // can be written to disk - we read from the unsorted list to keep it
  // in its own order ... allow custom sorting of bookmarks
  KRBookmark *i;
  for ( i=bookmarks.first(); i != 0; i=bookmarks.next() )
    list << i->getName() << i->getUrl();
  // and write it down to disk
  krConfig->writeEntry("Links",list);
}

KRBookmark::KRBookmark(QString _name, QString _url) {
  url=_url;
  name=(_name=="" ? url : _name);
  menuId=-1;
}

bool operator==(QString &b, KRBookmark &a) {
  return b==a.getName();
}

#include "bookman.moc"

