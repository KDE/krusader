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

#ifndef KGPROTOCOLS_H
#define KGPROTOCOLS_H

// QtWidgets
#include <QPushButton>

#include "konfiguratorpage.h"
#include "../GUI/krtreewidget.h"
#include "../GUI/krlistwidget.h"

class KgProtocols : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgProtocols(bool first, QWidget* parent = nullptr);

    void loadInitialValues() override;
    void setDefaults() override;
    bool apply() override;
    bool isChanged() override;

    static  void init();

public slots:
    void         slotDisableButtons();
    void         slotAddProtocol();
    void         slotRemoveProtocol();
    void         slotAddMime();
    void         slotRemoveMime();

protected:
    void         loadProtocols();
    void         loadMimes();
    void         addSpacer(QBoxLayout *parent);

    void         addProtocol(const QString& name, bool changeCurrent = false);
    void         removeProtocol(const QString& name);
    void         addMime(QString name, const QString& protocol);
    void         removeMime(const QString& name);

    KrTreeWidget *linkList;

    KrListWidget *protocolList;
    KrListWidget *mimeList;

    QPushButton *btnAddProtocol;
    QPushButton *btnRemoveProtocol;
    QPushButton *btnAddMime;
    QPushButton *btnRemoveMime;
};

#endif /* __KgProtocols_H__ */
