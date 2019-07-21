/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2005-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef FILTERTABS_H
#define FILTERTABS_H

// QtCore
#include <QList>

#include "filterbase.h"

class QTabWidget;

class FilterTabs : public QObject
{
    Q_OBJECT

public:

    enum {
        HasProfileHandler       =   0x1000,
        HasRecurseOptions       =   0x2000,
        HasSearchIn             =   0x4000,
        HasDontSearchIn         =   0x8000,

        Default                 =   0xe000
    };

    static FilterTabs * addTo(QTabWidget *tabWidget, int props = FilterTabs::Default,
                              QStringList extraOptions = QStringList());
    static KRQuery      getQuery(QWidget *parent = nullptr);

    FilterBase *get(const QString& name);
    bool isExtraOptionChecked(QString name);
    void checkExtraOption(QString name, bool check);
    FilterSettings getSettings();
    void applySettings(const FilterSettings &s);
    void reset();

public slots:
    void  loadFromProfile(const QString&);
    void  saveToProfile(const QString&);
    bool  fillQuery(KRQuery *query);
    void  close(bool accept = true) {
        emit closeRequest(accept);
    }

signals:
    void  closeRequest(bool accept = true);

private:
    FilterTabs(int properties, QTabWidget *tabWidget, QObject *parent, QStringList extraOptions);
    void  acceptQuery();

    QList<FilterBase *> filterList;
    QList<int>      pageNumbers;

    QTabWidget * tabWidget;
};

#endif /* FILTERTABS_H */
