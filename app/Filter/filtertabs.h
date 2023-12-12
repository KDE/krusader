/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
        HasProfileHandler = 0x1000,
        HasRecurseOptions = 0x2000,
        HasSearchIn = 0x4000,
        HasDontSearchIn = 0x8000,

        Default = 0xe000
    };

    static FilterTabs *addTo(QTabWidget *tabWidget, int props = FilterTabs::Default, QStringList extraOptions = QStringList());
    static KrQuery getQuery(QWidget *parent = nullptr);

    FilterBase *get(const QString &name);
    bool isExtraOptionChecked(QString name);
    void checkExtraOption(QString name, bool check);
    FilterSettings getSettings();
    void applySettings(const FilterSettings &s);
    void reset();

public slots:
    void loadFromProfile(const QString &);
    void saveToProfile(const QString &);
    bool fillQuery(KrQuery *query);
    void close(bool accept = true)
    {
        emit closeRequest(accept);
    }

signals:
    void closeRequest(bool accept = true);

private:
    FilterTabs(int properties, QTabWidget *tabWidget, QObject *parent, QStringList extraOptions);
    void acceptQuery();

    QList<FilterBase *> filterList;
    QList<int> pageNumbers;

    QTabWidget *tabWidget;
};

#endif /* FILTERTABS_H */
