/***************************************************************************
                                kfilelist.h
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


#ifndef KFILELIST_H
#define KFILELIST_H

#include <qwidget.h>
#include <klistview.h>
#include <qstring.h>
#include <qdatetime.h>
#include "../krusader.h"

// this class overloads the QListView class in order to allow sorting
// with directories shown first
/////////////////////////////////////////////////////////////////////
class KFileList : public KListView {
	 Q_OBJECT

enum  dState {
  pending  = 0,
  dragging = 1,
  nothing  = 3
};
   	
public:
	KFileList ( QWidget *parent = 0, const char *name = 0 );
  QListViewItem* root;
  QListViewItem* firstUnmarkedAboveCurrent();
  QString getFilename(QListViewItem* it) {
    if (it) return it->text(0); else return QString::null;
  } // <rewrite>
  void getSelectedNames(QStringList* fileNames);
    	
public slots:
  void select(QString filter);
  void unselect(QString filter);
  void invertSelection();
  void markCurrent(bool movedown=true);
  void finishedDragging() { dragState=nothing; }
  //void checkForRightClickMenu();
  void startScrolling( int y );
  void scrollItem( int y ); // scroll y items down, if y<0 scroll y items up
  void keepScrolling();

protected:
  void keyPressEvent(QKeyEvent *e);
  void contentsMouseMoveEvent(QMouseEvent *e);
  //void viewportMouseMoveEvent(QMouseEvent *e);
  //void contentsMousePressEvent(QMouseEvent *e);
  //void contentsMouseReleaseEvent(QMouseEvent *e);

signals:
  void letsDrag(int);
  void rightClickMenu(QListViewItem *,QPoint);

private:
  int dragSX,dragSY; // location of mouse in the beginning of a drag
  int dragState;     // pending | dragging | nothing
  int pressX,pressY; // location of mouse in the beginning of right-click hold
  int yDelta;        // the delta between global cursor position and viewport position
  bool stillPressed; // true if right button is still pressed
  bool toolTip;
  int scrollDir;
  bool stillScrolling;
  int mouseSelType;  // selection type: 0 - normal, 1 - left click, 2 - right click
  int numOfSelected; // how many files are seleceted
  QListViewItem *toolTipItem;
};

#endif
