/***************************************************************************
                                krlistitem.h
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


#ifndef KRLISTITEM_H
#define KRLISTITEM_H

#include <qwidget.h>
#include <qlistview.h>

class KRListItem : public QListViewItem  {
public:
  enum cmpColor {none = 0, exclusive = 1, newer = 2, older = 4, identical = 8 };

  KRListItem( QListView * parent ):QListViewItem( parent ){}

	// constructor for list view
	KRListItem(QListView *parent, QListViewItem * after ,QString name, QString size,
						 QString dateTime, QString r, QString w, QString x, int cmID = 0):
						 QListViewItem(parent,after) {
								setText(0,name);
								setText(1,size);
								setText(2,dateTime);
								setText(3,r);
								setText(4,w);
								setText(5,x);
								compareModeID = cmID;
	}

	// constructor for tree view
	KRListItem(QListViewItem *parent,QString name, QString size, QString dateTime,
             QString r, QString w, QString x):
             QListViewItem(parent,name,size,dateTime,r,w,x){}

	KRListItem(QListView *parent,QString name) : QListViewItem(parent,name){}
  KRListItem(QListViewItem *parent,QString name) : QListViewItem(parent,name){}
  KRListItem(QListView *parent,QString name, QString size):
             QListViewItem(parent,name,size){}
	~KRListItem();
	
  QString key ( int column, bool ascending ) const;
  inline int getCompareModeID() const { return compareModeID; }
  void paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align);

private:
  int compareModeID;
};

#endif
