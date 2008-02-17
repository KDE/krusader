/***************************************************************************
                       generalfilter.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
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

#ifndef GENERALFILTER_H
#define GENERALFILTER_H

#include "filterbase.h"
#include "../VFS/krquery.h"
#include "../Dialogs/kurllistrequester.h"
#include "../GUI/profilemanager.h"

#include <qwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../GUI/krlistwidget.h"
#include <QHBoxLayout>
#include <kcombobox.h>
#include <kshellcompletion.h>
#include <khistorycombobox.h>

class GeneralFilter : public QWidget, public FilterBase
{
  Q_OBJECT

public:
  GeneralFilter( FilterTabs *tabs, int properties, QWidget *parent = 0 );
  ~GeneralFilter();

  virtual bool          fillQuery( KRQuery *query );
  virtual void          queryAccepted();
  virtual void          loadFromProfile( QString name );
  virtual void          saveToProfile( QString name );
  virtual QString       name() { return "GeneralFilter";}
  virtual FilterTabs *  filterTabs() { return fltTabs; }

public slots:
  void    slotAddBtnClicked();
  void    slotLoadBtnClicked();
  void    slotOverwriteBtnClicked();
  void    slotRemoveBtnClicked();
  void    slotProfileDoubleClicked( QListWidgetItem * );
  void    refreshProfileListBox();
  
public:
  QCheckBox* searchForCase;
  QCheckBox* remoteContentSearch;
  QCheckBox* containsTextCase;
  QCheckBox* containsWholeWord;
  QCheckBox* searchInDirs;
  QCheckBox* searchInArchives;
  QCheckBox* followLinks;

  KURLListRequester *searchIn;
  KURLListRequester *dontSearchIn;
  QHBoxLayout *middleLayout;

  KHistoryComboBox* searchFor;
  KHistoryComboBox* containsText;

  KComboBox* ofType;

  KShellCompletion completion;

  KrListWidget *profileListBox;
  
  QPushButton *profileAddBtn;
  QPushButton *profileLoadBtn;
  QPushButton *profileOverwriteBtn;
  QPushButton *profileRemoveBtn;
  
  ProfileManager *profileManager;

  int properties;

  FilterTabs *fltTabs;

};

#endif /* GENERALFILTER_H */
