/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERBASE_H
#define FILTERBASE_H

#include "filtersettings.h"

// QtCore
#include <QString>
// QtWidgets
#include <QComboBox>

class FilterTabs;

class FilterBase
{
public:
    virtual ~FilterBase()
    {
    }

    virtual void queryAccepted() = 0;
    virtual QString name() = 0;
    virtual FilterTabs *filterTabs() = 0;
    virtual bool getSettings(FilterSettings &) = 0;
    virtual void applySettings(const FilterSettings &) = 0;

protected:
    static void setComboBoxValue(QComboBox *cb, QString value)
    {
        int idx = cb->findText(value);
        cb->setCurrentIndex(idx < 0 ? 0 : idx);
    }
};

#endif /* FILTERBASE_H */
