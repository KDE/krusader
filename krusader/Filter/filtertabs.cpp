/***************************************************************************
                        filtertab.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filtertabs.h"
#include "filterdialog.h"
#include "generalfilter.h"
#include "advancedfilter.h"
#include "../krglobal.h"

#include <klocale.h>
#include <kmessagebox.h>


FilterTabs::FilterTabs(int properties, KTabWidget *tabWidget,
                       QObject *parent, QStringList extraOptions) :
        QObject(parent)
{
    this->tabWidget = tabWidget;

    GeneralFilter *generalFilter = new GeneralFilter(this, properties, tabWidget, extraOptions);
    tabWidget->addTab(generalFilter, i18n("&General"));
    filterList.append(generalFilter);
    pageNumbers.append(tabWidget->indexOf(generalFilter));

    AdvancedFilter *advancedFilter = new AdvancedFilter(this, tabWidget);
    tabWidget->addTab(advancedFilter, i18n("&Advanced"));
    filterList.append(advancedFilter);
    pageNumbers.append(tabWidget->indexOf(advancedFilter));

    reset(); // apply defaults
}

bool FilterTabs::isExtraOptionChecked(QString name)
{
    return static_cast<GeneralFilter*>(get("GeneralFilter"))->isExtraOptionChecked(name);
}

void FilterTabs::checkExtraOption(QString name, bool check)
{
    static_cast<GeneralFilter*>(get("GeneralFilter"))->checkExtraOption(name, check);
}

FilterTabs * FilterTabs::addTo(KTabWidget *tabWidget, int props, QStringList extraOptions)
{
    return new FilterTabs(props, tabWidget, tabWidget, extraOptions);
}

FilterSettings FilterTabs::getSettings()
{
    FilterSettings s;

    for (int i = 0; i != filterList.count(); i++) {
        if(!filterList[i]->getSettings(s)) {
            tabWidget->setCurrentIndex(pageNumbers[i]);
            return FilterSettings();
        }
    }

    s.valid = true;
    acceptQuery();

    return s;
}

void FilterTabs::applySettings(const FilterSettings &s)
{
    if(s.isValid()) {
        QListIterator<FilterBase*> it(filterList);
        while (it.hasNext())
            it.next()->applySettings(s);
    }
}

void FilterTabs::reset()
{
    FilterSettings s; // default settings
    s.valid = true;
    applySettings(s);
}

void FilterTabs::saveToProfile(QString name)
{
    FilterSettings s(getSettings());
    if(s.isValid())
        s.save(KConfigGroup(krConfig, name));
    krConfig->sync();
}

void FilterTabs::loadFromProfile(QString name)
{
    FilterSettings s;
    s.load(KConfigGroup(krConfig, name));
    if(!s.isValid())
        KMessageBox::error(tabWidget, i18n("Couldn't load profile."));
    else
        applySettings(s);
}

void FilterTabs::acceptQuery()
{
    QListIterator<FilterBase *> it(filterList);
    while (it.hasNext()) {
        FilterBase *filter = it.next();

        filter->queryAccepted();
    }
}

bool FilterTabs::fillQuery(KRQuery *query)
{
    *query = getSettings().toQuery();

    return !query->isNull();
}

FilterBase * FilterTabs::get(QString name)
{
    QListIterator<FilterBase *> it(filterList);
    while (it.hasNext()) {
        FilterBase *filter = it.next();

        if (filter->name() == name)
            return filter;
    }

    return 0;
}

KRQuery FilterTabs::getQuery(QWidget *parent)
{
    FilterDialog dialog(parent);
    return dialog.getQuery();
}


#include "filtertabs.moc"
