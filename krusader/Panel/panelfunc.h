/***************************************************************************
                                panelfunc.h
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


#ifndef PANELFUNC_H
#define PANELFUNC_H
#include "listpanel.h"
#include <qobject.h>
#include <qptrstack.h>

class ListPanelFunc : public QObject{
friend class ListPanel;
	Q_OBJECT
public slots:
	void execute(QListViewItem *i);

public:
	ListPanelFunc(class ListPanel *parent);
 ~ListPanelFunc();

  inline void refresh(){ refresh(panel->virtualPath); } // re-read the files
	inline vfs* files()  { return vfsStack.top();      } // return the vfs
  void refresh(const QString path);
  void openUrl( QString path,QString type=QString::null );
  void refreshActions();

	void redirectLink();
	void krlink(bool sym);
	void goBack();
  void dirUp();
  void properties();
	void terminal();
	void editFile();
	void view();
	void mkdir();
	void moveFiles();
	void rename();
  void pack();
  void unpack();
  void testArchive();
  void copyFiles();
	void deleteFiles();
  void calcSpace();
  void FTPDisconnect();
  void newFTPconnection(QString host=QString::null);
  void changeVFS(QString type, QString origin);
	inline ListPanelFunc* otherFunc(){ return panel->otherPanel->func; }

protected:
	ListPanel	     *panel;    // our ListPanel
	QStringList    backStack; // Path stack for the "back" button
	bool           inRefresh; // true when we are in refresh()
	QPtrStack<vfs> vfsStack; // the vfs stack.
};

#endif
