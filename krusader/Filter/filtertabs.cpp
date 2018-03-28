/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2005-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "filtertabs.h"
#include "filterdialog.h"
#include "generalfilter.h"
#include "advancedfilter.h"
#include "../krglobal.h"

// QtWidgets
#include <QTabWidget>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>


FilterTabs::FilterTabs(int properties, QTabWidget *tabWidget,
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

FilterTabs * FilterTabs::addTo(QTabWidget *tabWidget, int props, QStringList extraOptions)
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
        KMessageBox::error(tabWidget, i18n("Could not load profile."));
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

