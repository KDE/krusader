/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERALFILTER_H
#define GENERALFILTER_H

// QtWidgets
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QWidget>

#include <KComboBox>
#include <KShellCompletion>

#include "../Dialogs/kurllistrequester.h"
#include "../GUI/krhistorycombobox.h"
#include "../GUI/krlistwidget.h"
#include "../GUI/profilemanager.h"
#include "filterbase.h"

class GeneralFilter : public QWidget, public FilterBase
{
    Q_OBJECT

public:
    GeneralFilter(FilterTabs *tabs, int properties, QWidget *parent = nullptr, QStringList extraOptions = QStringList());
    ~GeneralFilter() override;

    void queryAccepted() override;
    QString name() override
    {
        return "GeneralFilter";
    }
    FilterTabs *filterTabs() override
    {
        return fltTabs;
    }
    bool getSettings(FilterSettings &) override;
    void applySettings(const FilterSettings &) override;

    bool isExtraOptionChecked(const QString &name);
    void checkExtraOption(const QString &name, bool check);

public slots:
    void slotAddBtnClicked();
    void slotLoadBtnClicked();
    void slotOverwriteBtnClicked();
    void slotRemoveBtnClicked();
    void slotDisable();
    void slotRegExpTriggered(QAction *act);
    void slotProfileDoubleClicked(QListWidgetItem *);
    void refreshProfileListBox();

public:
    KComboBox *contentEncoding;
    QCheckBox *searchForCase;
    QCheckBox *containsTextCase;
    QCheckBox *containsWholeWord;
    QCheckBox *useExcludeFolderNames;
    QCheckBox *searchInDirs;
    QCheckBox *searchInArchives;
    QCheckBox *followLinks;
    QHash<QString, QCheckBox *> extraOptions;

    KURLListRequester *searchIn;
    KURLListRequester *dontSearchIn;
    QLayout *middleLayout;

    KrHistoryComboBox *searchFor;
    KrHistoryComboBox *containsText;
    KrHistoryComboBox *excludeFolderNames;
    QToolButton *containsRegExp;

    KComboBox *ofType;

    QLabel *encLabel;
    QLabel *containsLabel;

    KShellCompletion completion;

    KrListWidget *profileListBox;

    QPushButton *profileAddBtn;
    QPushButton *profileLoadBtn;
    QPushButton *profileOverwriteBtn;
    QPushButton *profileRemoveBtn;

    ProfileManager *profileManager;

    int properties;

    FilterTabs *fltTabs;

private:
    QCheckBox *createExcludeCheckBox(const KConfigGroup &group);
    KrHistoryComboBox *createExcludeComboBox(const KConfigGroup &group);
};

#endif /* GENERALFILTER_H */
