/***************************************************************************
                                 krsearchdialog.h
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



#ifndef KRSEARCHDIALOG_H
#define KRSEARCHDIALOG_H

#include <qwidget.h>
#include <qstringlist.h>
#include <kshellcompletion.h>
#include "krsearchdialogbase.h"
#include "krquery.h"
#include "krsearchmod.h"
#include <sys/types.h>
#include <time.h>


class KrSearchDialog : public KrSearchBase  {
   Q_OBJECT
public: 
	KrSearchDialog(QWidget *parent=0, const char *name=0);
	~KrSearchDialog();
	
	void prepareGUI();
	
public slots:
  void saveSearch();
  void loadSearch();
  void loadSearch(QListViewItem *);
  void startSearch();
  void stopSearch();
  void found(QString what, QString where, long size, time_t mtime, QString perm);
  void closeDialog();
  void addToSearchIn();
  void addToSearchInManually();
  void addToDontSearchIn();
  void addToDontSearchInManually();
  void modifiedBetweenSetDate1();
  void modifiedBetweenSetDate2();
  void notModifiedAfterSetDate();
  void resultClicked(QListViewItem*);

protected slots:
  void reject();

private:
  void refreshSavedSearches();
  void invalidDateMessage(QLineEdit *p);
  void fillList(QComboBox *list, QString filename);
  bool gui2query();
  void query2gui();

private:
  KRQuery *query;
  KRSearchMod *searcher;
  KShellCompletion completion;
  QStringList savedSearches;
};

#endif
