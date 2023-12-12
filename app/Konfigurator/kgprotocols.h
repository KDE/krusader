/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPROTOCOLS_H
#define KGPROTOCOLS_H

// QtWidgets
#include <QPushButton>

#include "../GUI/krlistwidget.h"
#include "../GUI/krtreewidget.h"
#include "konfiguratorpage.h"

class KgProtocols : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgProtocols(bool first, QWidget *parent = nullptr);

    void loadInitialValues() override;
    void setDefaults() override;
    bool apply() override;
    bool isChanged() override;

    static void init();

public slots:
    void slotDisableButtons();
    void slotAddProtocol();
    void slotRemoveProtocol();
    void slotAddMime();
    void slotRemoveMime();

protected:
    void loadProtocols();
    void loadMimes();
    void addSpacer(QBoxLayout *parent);

    void addProtocol(const QString &name, bool changeCurrent = false);
    void removeProtocol(const QString &name);
    void addMime(QString name, const QString &protocol);
    void removeMime(const QString &name);

    KrTreeWidget *linkList;

    KrListWidget *protocolList;
    KrListWidget *mimeList;

    QPushButton *btnAddProtocol;
    QPushButton *btnRemoveProtocol;
    QPushButton *btnAddMime;
    QPushButton *btnRemoveMime;
};

#endif /* __KgProtocols_H__ */
