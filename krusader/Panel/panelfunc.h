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
#include "kvfspanel.h"
#include <qlistview.h>
#include <qobject.h>

class PanelFunc : public QObject{
  Q_OBJECT
public:
	           PanelFunc(){}
	virtual  	~PanelFunc(){}

public slots:
	virtual void redirectLink   (){}
	virtual void krlink         (bool sym){}
	virtual void goBack         (){}
  virtual void properties     (){}
	virtual void dirUp          (){}
 	virtual void terminal       (){}
	virtual void editFile      	(){}
	virtual void view         	(){}
	virtual void mkdir        	(){}
	virtual void moveFiles    	(){}
	virtual void rename       	(){}
  virtual void pack         	(){}
  virtual void unpack       	(){}
  virtual	void copyFiles    	(){}
	virtual	void deleteFiles  	(){}
	virtual void calcSpace    	(){}
	virtual void testArchive  	(){}
  virtual void FTPDisconnect	(){}
  virtual void execute(QListViewItem *i){}
	virtual void changeVFS(QString type, QString origin){}
  //	inline  void refresh(const QString path){ panel->refresh(path) }
  virtual void getSelectedNames(QStringList*){}
  inline  bool canGoBack(){return !backStack.isEmpty();}
	virtual void newFTPconnection(QString host=QString::null){}

public:
  QStringList backStack; // Path stack for the "back" button
};

class ListPanelFunc : public PanelFunc{
	Q_OBJECT
public:
	ListPanelFunc(class ListPanel *parent);
 ~ListPanelFunc(){}
	
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
	void execute(QListViewItem *i);
  void calcSpace();
  void FTPDisconnect();
  void newFTPconnection(QString host=QString::null);
  void changeVFS(QString type, QString origin);

protected:
	ListPanel	  *panel;
};

class TreePanelFunc : public PanelFunc{
	Q_OBJECT
public:	
	TreePanelFunc(class TreePanel *parent);
 ~TreePanelFunc(){}
	
	void execute(QListViewItem*){}
  void properties();
	void terminal();
	void mkdir();
	void moveFiles();
	void rename();
  void pack();
	void copyFiles();
	void deleteFiles();
	void calcSpace();
	
protected:
	TreePanel	  *panel;
};

class QuickViewPanelFunc : public PanelFunc{
	Q_OBJECT
public:	
	QuickViewPanelFunc(){}
 ~QuickViewPanelFunc(){}
};

#endif
