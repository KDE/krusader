/***************************************************************************
                        filterdialog.h  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
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

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include "filtersettings.h"
#include "../VFS/krquery.h"

#include <kdialog.h>

class FilterTabs;
class GeneralFilter;

class FilterDialog : public KDialog
{
    Q_OBJECT

public:
    FilterDialog(QWidget *parent = 0, QString caption = QString(),
                 QStringList extraOptions = QStringList(), bool modal = true);
    KRQuery getQuery();
    const FilterSettings& getSettings() {
        return settings;
    }
    void applySettings(const FilterSettings &s);
    bool isExtraOptionChecked(QString name);
    void checkExtraOption(QString name, bool check);

public slots:
    void slotCloseRequest(bool doAccept);

protected slots:
    virtual void slotButtonClicked(int button);
    void slotOk();

private:
    FilterTabs * filterTabs;
    GeneralFilter * generalFilter;
    FilterSettings settings;
};

#endif /* FILTERDIALOG_H */
