/***************************************************************************
                                 kviewer.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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

#ifndef KVIEWER_H
#define KVIEWER_H

#include <qstring.h>
#include <kurl.h>
#include <keditcl.h>
#include <qwidget.h>

class KhtmlViewer;
// A wrapper-class for Krusader's viewing abilities. Kviewer is called statically
// and runs the nessecery viewer
class KViewer : public QObject {
  Q_OBJECT
public:
	// This function is the only interface you'll ever need with KViewer, just call
	// the function with the name and mimetype (obtainable from it's vfile) and go!
	static void view(QString fileName, QString mimetype);
	static void viewGeneric(QString fileName, QString mimetype, QString caption = QString::null);
  static void viewSpecial(QString fileName);
  static void viewText(QString fileName);
	static void viewImage(QString fileName);
  static void viewHex(QString fileName);
	KViewer() {} // a dummy constructor
};

class KREdit : public KEdit {
  Q_OBJECT
public:
  KREdit(QWidget* parent): KEdit(parent) {}
public slots:
  void krsearch(){ search(); }
};

#endif
