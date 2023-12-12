/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGGENERAL_H
#define KGGENERAL_H

#include "konfiguratorpage.h"

class KgGeneral : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgGeneral(bool first, QWidget *parent = nullptr);

public slots:
    void applyTempDir(QObject *, const QString &, const QString &);
    void slotFindTools();

    void slotAddExtension();
    void slotRemoveExtension();

private:
    void createGeneralTab();
    void createViewerTab();
    void createExtensionsTab();
    QWidget *createTab(const QString &name);

    QTabWidget *tabWidget;
    KonfiguratorListBox *listBox;
};

#endif /* __KGGENERAL_H__ */
