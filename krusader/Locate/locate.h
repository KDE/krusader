/***************************************************************************
                          locate.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#ifndef __LOCATE_H__
#define __LOCATE_H__

#include <kdialogbase.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kprocess.h>
#include <qcheckbox.h>

class LocateDlg : public KDialogBase
{
  Q_OBJECT
  
public:
  LocateDlg();

  virtual void      slotUser1();
  virtual void      slotUser2();
  virtual void      slotUser3();

public slots:
  void              processStdout(KProcess *, char *, int);
  void              slotRightClick(QListViewItem *);
  void              slotDoubleClick(QListViewItem *);
  void              updateFinished();
  
protected:
  virtual void      keyPressEvent( QKeyEvent * );
  
private:
  void              operate( QListViewItem *item, int task );

  bool              find();
  void              nextLine();

  bool              stopping;
  bool              onlyExist;

  KHistoryCombo    *locateSearchFor;
  KListView        *resultList;
  QString           remaining;
  KListViewItem    *lastItem;

  long              findOptions;
  QString           findPattern;
  KListViewItem    *findCurrentItem;

  QCheckBox        *existingFiles;
  QCheckBox        *caseSensitive;

  static KProcess  *updateProcess;
};

#endif /* __LOCATE_H__ */
