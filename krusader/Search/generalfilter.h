/***************************************************************************
                       generalfilter.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

                                                     S o u r c e    F i l e

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

#include "krquery.h"

#include <qwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <qlistbox.h>
#include <qtoolbutton.h>
#include <kshellcompletion.h>

class GeneralFilter : public QWidget
{
  Q_OBJECT
  
public:
  GeneralFilter( QWidget *parent = 0, const char *name = 0 );
  ~GeneralFilter();
  
  bool fillQuery( KRQuery *query );
  void queryAccepted();
  
public slots:  
  void addToDontSearchIn();
  void addToDontSearchInManually();
  void addToSearchIn();
  void addToSearchInManually();
  void deleteSelectedItems( QListBox * );
  
  virtual void keyPressEvent(QKeyEvent *e);

public:  
  QCheckBox* searchForCase;  
  QCheckBox* containsTextCase;
  QCheckBox* containsWholeWord;
  QCheckBox* searchInDirs;
  QCheckBox* searchInArchives;
  QCheckBox* followLinks;
    
  KHistoryCombo* searchFor;
  KHistoryCombo* containsText;
    
  KComboBox* ofType;
  
  KLineEdit* searchInEdit;
  KLineEdit* dontSearchInEdit;
    
  QListBox* searchIn;
  QListBox* dontSearchIn;
    
  QToolButton* searchInBtn;
  QToolButton* searchInBtnAdd;
  QToolButton* dontSearchInBtn;
  QToolButton* dontSearchInBtnAdd;

  KShellCompletion completion;
};

#endif /* GENERALFILTER_H */
