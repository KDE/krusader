/***************************************************************************
                                 krslots.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
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



#ifndef KRSLOTS_H
#define KRSLOTS_H

#include <qobject.h>
// the 2 following #includes should go away with the ugly stubs on the bottom
#include "krusader.h"
#include "krusaderview.h"

class ListPanel;

class KRslots : public QObject  {
	Q_OBJECT

public:
  enum compareMode { full } ;

	KRslots(){}
	~KRslots(){}

public slots:
	void selectCompareMask();
	void compareDirectories();
  void sendFileByEmail(QString filename);
  void compareContent();
  void rightclickMenu();
  void toggleHidden();
	void toggleSortByExt();
	void configToolbar();
  void configKeys();
  void toggleToolbar();
  void toggleStatusbar();
	void toggleTerminal();
  void home();
	void root();
  void dirUp();
  void markAll();
	void unmarkAll();
  void markGroup();	
  void unmarkGroup();	
	void invert();
	void refresh();
  void refresh(QString p);
  void properties();
  void back();
  void slotPack();
  void slotUnpack();
  void testArchive();
  void calcSpace();
  void FTPDisconnect();
	void allFilter();
	void execFilter();
	void customFilter();
  void newFTPconnection();
  void newFTPconnection(QString host);
  void runKonfigurator(bool firstTime=false);
  void startKonfigurator() { runKonfigurator(false); }
  void search();						 				// call the search module	
  void homeTerminal();
  void sysInfo();
  void addBookmark();
  void runMountMan();
	void runRemoteMan();
	void toggleFnkeys();
	void toggleCmdline();
	void changeTrashIcon();
  void showAboutApplication();
	void multiRename();
	// F2
	void terminal();
	// F3
	void view();
	// F4
	void edit();
	// F5
	void copyFiles();
	// F6
	void moveFiles();	
	// F7
	void mkdir();
	// F8
	void deleteFiles();     	
	// F9
	void rename();

  // ugly stubs, remove later ?
  void slotCurrentChanged(QString p) { krApp->mainView->slotCurrentChanged(p); }
  void slotSetActivePanel(ListPanel *p) { krApp->mainView->slotSetActivePanel(p); }
};

#endif
