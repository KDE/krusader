/***************************************************************************
                                kmountmangui.h
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


#ifndef KMOUNTMANGUI_H
#define KMOUNTMANGUI_H

#include <kdialogbase.h>
#include <kjanuswidget.h>
#include <qframe.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <kjanuswidget.h>
#include <qtimer.h>
#include <kurl.h>

#define  WATCHER_DELAY    500

class KRFSDisplay;
class KRdirWatch;

class KMountManGUI : public KDialogBase {
  Q_OBJECT

  enum Pages {
    Filesystems = 0
  };

public:
  KMountManGUI();
  ~KMountManGUI();
  void createLayout();   // calls the various tab layout-creators
  void createMainPage(); // creator of the main page - filesystems
  
public slots:
  void updateList();     // fill-up the filesystems list
  void doubleClicked(QListViewItem *);
  void clicked(QListViewItem *);
  void changeActive(QListViewItem *);
  void checkMountChange(); // check whether the mount table was changed

signals:
  void refreshPanel(const KURL &);

private:
  KRFSDisplay *info;
  QFrame *mainPage;
  KJanusWidget *widget;
  QListView *mountList;
  QTimer *watcher;
};

#endif
