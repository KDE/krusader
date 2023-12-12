/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    explicit KgDependencies(bool first, QWidget *parent = nullptr);

    int activeSubPage() override;

private:
    void addApplication(const QString &name, QGridLayout *grid, int row, QWidget *parent, int page, const QString &additionalList = QString());

public slots:
    void slotApply(QObject *obj, const QString &configGroup, const QString &name);

private:
    QTabWidget *tabWidget;
};

#endif /* __KGDEPENDENCIES_H__ */
