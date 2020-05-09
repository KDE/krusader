/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2005-2020 Krusader Krew [https://krusader.org]              *
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
    virtual ~FilterBase()   {}

    virtual void            queryAccepted() = 0;
    virtual QString         name() = 0;
    virtual FilterTabs *    filterTabs() = 0;
    virtual bool            getSettings(FilterSettings&) = 0;
    virtual void            applySettings(const FilterSettings&) = 0;

protected:
    static void setComboBoxValue(QComboBox *cb, QString value) {
        int idx = cb->findText(value);
        cb->setCurrentIndex(idx < 0 ? 0 : idx);
    }
};

#endif /* FILTERBASE_H */
