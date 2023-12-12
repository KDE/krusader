/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include "../FileSystem/krquery.h"
#include "filtersettings.h"

// QtWidgets
#include <QDialog>

class FilterTabs;
class GeneralFilter;

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = nullptr, const QString &caption = QString(), QStringList extraOptions = QStringList(), bool modal = true);
    KrQuery getQuery();
    const FilterSettings &getSettings()
    {
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
    FilterTabs *filterTabs;
    GeneralFilter *generalFilter;
    FilterSettings settings;
};

#endif /* FILTERDIALOG_H */
