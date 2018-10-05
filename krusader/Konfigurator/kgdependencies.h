/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KGDEPENDENCIES_H
#define KGDEPENDENCIES_H

// QtWidgets
#include <QGridLayout>

#include "konfiguratorpage.h"

class QTabWidget;


class KgDependencies : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgDependencies(bool first, QWidget* parent = nullptr);

    int activeSubPage() Q_DECL_OVERRIDE;

private:
    void addApplication(QString name, QGridLayout *grid, int row, QWidget *parent, int page, QString additionalList = QString());

public slots:
    void slotApply(QObject *obj, QString configGroup, QString name);

private:
    QTabWidget *tabWidget;
};

#endif /* __KGDEPENDENCIES_H__ */
