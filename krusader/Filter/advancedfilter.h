/***************************************************************************
                        advancedfilter.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "filterbase.h"
#include "../VFS/krquery.h"

#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QToolButton>

#include <KComboBox>
#include <KLineEdit>

class QSpinBox;

class AdvancedFilter : public QWidget, public FilterBase
{
    Q_OBJECT

public:
    AdvancedFilter(FilterTabs *tabs, QWidget *parent = 0);

    virtual void          queryAccepted() {}
    virtual QString       name() {
        return "AdvancedFilter";
    }
    virtual FilterTabs *  filterTabs() {
        return fltTabs;
    }
    virtual bool getSettings(FilterSettings&);
    virtual void applySettings(const FilterSettings&);

public slots:
    void modifiedBetweenSetDate1();
    void modifiedBetweenSetDate2();
    void notModifiedAfterSetDate();

public:
    QCheckBox* minSizeEnabled;
    QSpinBox* minSizeAmount;
    KComboBox* minSizeType;

    QCheckBox* maxSizeEnabled;
    QSpinBox* maxSizeAmount;
    KComboBox* maxSizeType;

    QRadioButton* modifiedBetweenEnabled;
    QRadioButton* notModifiedAfterEnabled;
    QRadioButton* modifiedInTheLastEnabled;

    KLineEdit* modifiedBetweenData1;
    KLineEdit* modifiedBetweenData2;

    QToolButton* modifiedBetweenBtn1;
    QToolButton* modifiedBetweenBtn2;
    QToolButton* notModifiedAfterBtn;

    KLineEdit* notModifiedAfterData;
    QSpinBox* modifiedInTheLastData;
    QSpinBox* notModifiedInTheLastData;
    KComboBox* modifiedInTheLastType;
    KComboBox* notModifiedInTheLastType;

    QCheckBox* belongsToUserEnabled;
    KComboBox* belongsToUserData;
    QCheckBox* belongsToGroupEnabled;
    KComboBox* belongsToGroupData;

    QCheckBox* permissionsEnabled;

    KComboBox* ownerW;
    KComboBox* ownerR;
    KComboBox* ownerX;
    KComboBox* groupW;
    KComboBox* groupR;
    KComboBox* groupX;
    KComboBox* allW;
    KComboBox* allX;
    KComboBox* allR;

    FilterTabs *fltTabs;

private:
    void changeDate(KLineEdit *p);
    void fillList(KComboBox *list, QString filename);
    void invalidDateMessage(KLineEdit *p);
};

#endif /* ADVANCEDFILTER_H */
