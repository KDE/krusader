  /***************************************************************************
                                 kviewerwidget.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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



#include "kviewerwidget.h"
#include "../resources.h"
#include <qapplication.h>

KViewerWidget::KViewerWidget(QWidget *parent, const char *name ) :
  QWidget(parent,name) {
}
KViewerWidget::~KViewerWidget(){
}

void KViewerWidget::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
    case Key_F10:
    case Key_Escape:
      delete this;
      break;
    case Key_F:
      if (e->state()==ControlButton) emit search();
      break;
  }
}

// overloaded to show-up centered
void KViewerWidget::show() {
	move( QApplication::desktop()->width()/2-(width()/2),
	  QApplication::desktop()->height()/2-(height()/2) );
  QWidget::show();
}

#include "kviewerwidget.moc"
