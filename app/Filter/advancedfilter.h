/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADVANCEDFILTER_H
#define ADVANCEDFILTER_H

#include "filterbase.h"

// QtWidgets
#include <QCheckBox>
#include <QRadioButton>
#include <QToolButton>
#include <QWidget>

#include <KComboBox>
#include <KLineEdit>

class QSpinBox;

class AdvancedFilter : public QWidget, public FilterBase
{
    Q_OBJECT

public:
    explicit AdvancedFilter(FilterTabs *tabs, QWidget *parent = nullptr);

    void queryAccepted() override
    {
    }
    QString name() override
    {
        return "AdvancedFilter";
    }
    FilterTabs *filterTabs() override
    {
        return fltTabs;
    }
    bool getSettings(FilterSettings &) override;
    void applySettings(const FilterSettings &) override;

public slots:
    void modifiedBetweenSetDate1();
    void modifiedBetweenSetDate2();
    void notModifiedAfterSetDate();

public:
    QCheckBox *minSizeEnabled;
    QSpinBox *minSizeAmount;
    KComboBox *minSizeType;

    QCheckBox *maxSizeEnabled;
    QSpinBox *maxSizeAmount;
    KComboBox *maxSizeType;

    QRadioButton *anyDateEnabled;
    QRadioButton *modifiedBetweenEnabled;
    QRadioButton *notModifiedAfterEnabled;
    QRadioButton *modifiedInTheLastEnabled;

    KLineEdit *modifiedBetweenData1;
    KLineEdit *modifiedBetweenData2;

    QToolButton *modifiedBetweenBtn1;
    QToolButton *modifiedBetweenBtn2;
    QToolButton *notModifiedAfterBtn;

    KLineEdit *notModifiedAfterData;
    QSpinBox *modifiedInTheLastData;
    QSpinBox *notModifiedInTheLastData;
    KComboBox *modifiedInTheLastType;
    KComboBox *notModifiedInTheLastType;

    QCheckBox *belongsToUserEnabled;
    KComboBox *belongsToUserData;
    QCheckBox *belongsToGroupEnabled;
    KComboBox *belongsToGroupData;

    QCheckBox *permissionsEnabled;

    KComboBox *ownerW;
    KComboBox *ownerR;
    KComboBox *ownerX;
    KComboBox *groupW;
    KComboBox *groupR;
    KComboBox *groupX;
    KComboBox *allW;
    KComboBox *allX;
    KComboBox *allR;

    FilterTabs *fltTabs;

private:
    void changeDate(KLineEdit *p);
    void fillList(KComboBox *list, const QString &filename);
    void invalidDateMessage(KLineEdit *p);
    static QDate stringToDate(const QString &text)
    {
        // 30.12.16 is interpreted as 1916-12-30
        return QLocale().toDate(text, QLocale::ShortFormat).addYears(100);
    }
    static QString dateToString(const QDate &date)
    {
        return QLocale().toString(date, QLocale::ShortFormat);
    }
};

#endif /* ADVANCEDFILTER_H */
