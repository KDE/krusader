/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtertabs.h"
#include "../krglobal.h"
#include "advancedfilter.h"
#include "filterdialog.h"
#include "generalfilter.h"

// QtWidgets
#include <QTabWidget>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <utility>

FilterTabs::FilterTabs(int properties, QTabWidget *tabWidget, QObject *parent, QStringList extraOptions)
    : QObject(parent)
{
    this->tabWidget = tabWidget;

    GeneralFilter *generalFilter = new GeneralFilter(this, properties, tabWidget, std::move(extraOptions));
    tabWidget->addTab(generalFilter, i18n("&General"));
    filterList.append(generalFilter);
    pageNumbers.append(tabWidget->indexOf(generalFilter));

    auto *advancedFilter = new AdvancedFilter(this, tabWidget);
    tabWidget->addTab(advancedFilter, i18n("&Advanced"));
    filterList.append(advancedFilter);
    pageNumbers.append(tabWidget->indexOf(advancedFilter));

    reset(); // apply defaults
}

bool FilterTabs::isExtraOptionChecked(QString name)
{
    return dynamic_cast<GeneralFilter *>(get("GeneralFilter"))->isExtraOptionChecked(std::move(name));
}

void FilterTabs::checkExtraOption(QString name, bool check)
{
    dynamic_cast<GeneralFilter *>(get("GeneralFilter"))->checkExtraOption(std::move(name), check);
}

FilterTabs *FilterTabs::addTo(QTabWidget *tabWidget, int props, QStringList extraOptions)
{
    return new FilterTabs(props, tabWidget, tabWidget, std::move(extraOptions));
}

FilterSettings FilterTabs::getSettings()
{
    FilterSettings s;

    for (int i = 0; i != filterList.count(); i++) {
        if (!filterList[i]->getSettings(s)) {
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
    if (s.isValid()) {
        QListIterator<FilterBase *> it(filterList);
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

void FilterTabs::saveToProfile(const QString &name)
{
    FilterSettings s(getSettings());
    if (s.isValid())
        s.save(KConfigGroup(krConfig, name));
    krConfig->sync();
}

void FilterTabs::loadFromProfile(const QString &name)
{
    FilterSettings s;
    s.load(KConfigGroup(krConfig, name));
    if (!s.isValid())
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

bool FilterTabs::fillQuery(KrQuery *query)
{
    *query = getSettings().toQuery();

    return !query->isNull();
}

FilterBase *FilterTabs::get(const QString &name)
{
    QListIterator<FilterBase *> it(filterList);
    while (it.hasNext()) {
        FilterBase *filter = it.next();

        if (filter->name() == name)
            return filter;
    }

    return nullptr;
}

KrQuery FilterTabs::getQuery(QWidget *parent)
{
    FilterDialog dialog(parent);
    return dialog.getQuery();
}
