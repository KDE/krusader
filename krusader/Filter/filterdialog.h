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

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include "filtersettings.h"
#include "../FileSystem/krquery.h"

// QtWidgets
#include <QDialog>

class FilterTabs;
class GeneralFilter;

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = 0, QString caption = QString(),
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
    void slotReset();
    void slotOk();

private:
    FilterTabs * filterTabs;
    GeneralFilter * generalFilter;
    FilterSettings settings;
};

#endif /* FILTERDIALOG_H */
