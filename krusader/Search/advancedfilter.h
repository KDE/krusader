/***************************************************************************
                      advancedfilter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai & Csaba Karai
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

#ifndef ADVANCEDFILTER_H
#define ADVANCEDFILTER_H

#include "krquery.h"

#include <qwidget.h>
#include <qcheckbox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>

class AdvancedFilter : public QWidget
{
  Q_OBJECT
  
public:
  AdvancedFilter( QWidget *parent = 0, const char *name = 0 );
  
  bool fillQuery( KRQuery *query );
    
public slots:
  void modifiedBetweenSetDate1();
  void modifiedBetweenSetDate2();
  void notModifiedAfterSetDate();
  
public:
  QCheckBox* smallerThanEnabled;
  QLineEdit* smallerThanAmount;
  KComboBox* smallerThanType;
  
  QCheckBox* biggerThanEnabled;
  KComboBox* biggerThanType;
  QLineEdit* biggerThanAmount;
  
  QRadioButton* modifiedBetweenEnabled;
  QRadioButton* notModifiedAfterEnabled;
  QRadioButton* modifiedInTheLastEnabled;
  
  QLineEdit* modifiedBetweenData1;
  QLineEdit* modifiedBetweenData2;
  
  QToolButton* modifiedBetweenBtn1;
  QToolButton* modifiedBetweenBtn2;
  QToolButton* notModifiedAfterBtn;
  
  QLineEdit* notModifiedAfterData;
  QLineEdit* modifiedInTheLastData;
  QLineEdit* notModifiedInTheLastData;
  QComboBox* modifiedInTheLastType;
  QComboBox* notModifiedInTheLastType;
    
  QCheckBox* belongsToUserEnabled;
  QComboBox* belongsToUserData;
  QCheckBox* belongsToGroupEnabled;
  QComboBox* belongsToGroupData;

  QCheckBox* permissionsEnabled;
  
  QComboBox* ownerW;
  QComboBox* ownerR;
  QComboBox* ownerX;
  QComboBox* groupW;
  QComboBox* groupR;
  QComboBox* groupX;
  QComboBox* allW;
  QComboBox* allX;
  QComboBox* allR;

private:
  void changeDate(QLineEdit *p);
  void fillList(QComboBox *list, QString filename);  
  void qdate2time_t(time_t *dest, QDate d, bool start);
  void invalidDateMessage(QLineEdit *p);
};

#endif /* ADVANCEDFILTER_H */
