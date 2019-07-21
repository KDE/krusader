/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef KGGENERAL_H
#define KGGENERAL_H

#include "konfiguratorpage.h"

class KgGeneral : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgGeneral(bool first, QWidget* parent = 0);

public slots:
    void  applyTempDir(QObject *, QString, QString);
    void  slotFindTools();

    void  slotAddExtension();
    void  slotRemoveExtension();

private:
    void createGeneralTab();
    void createViewerTab();
    void createExtensionsTab();
    QWidget* createTab(QString name);

    QTabWidget *tabWidget;
    KonfiguratorListBox *listBox;
};

#endif /* __KGGENERAL_H__ */
