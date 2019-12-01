/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
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

#ifndef GENERALFILTER_H
#define GENERALFILTER_H

// QtWidgets
#include <QWidget>
#include <QLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>

#include <KCompletion/KComboBox>
#include <KShellCompletion>
#include <KCompletion/KHistoryComboBox>

#include "filterbase.h"
#include "../Dialogs/kurllistrequester.h"
#include "../GUI/profilemanager.h"
#include "../GUI/krlistwidget.h"
#include "../GUI/krhistorycombobox.h"

class GeneralFilter : public QWidget, public FilterBase
{
    Q_OBJECT

public:
    GeneralFilter(FilterTabs *tabs, int properties, QWidget *parent = nullptr,
                  QStringList extraOptions = QStringList());
    ~GeneralFilter() override;

    void          queryAccepted() override;
    QString       name() override {
        return "GeneralFilter";
    }
    FilterTabs *  filterTabs() override {
        return fltTabs;
    }
    bool getSettings(FilterSettings&) override;
    void applySettings(const FilterSettings&) override;

    bool isExtraOptionChecked(const QString& name);
    void checkExtraOption(const QString& name, bool check);

public slots:
    void    slotAddBtnClicked();
    void    slotLoadBtnClicked();
    void    slotOverwriteBtnClicked();
    void    slotRemoveBtnClicked();
    void    slotDisable();
    void    slotRegExpTriggered(QAction * act);
    void    slotProfileDoubleClicked(QListWidgetItem *);
    void    refreshProfileListBox();

public:
    KComboBox* contentEncoding;
    QCheckBox* searchForCase;
    QCheckBox* containsTextCase;
    QCheckBox* containsWholeWord;
    QCheckBox* useExcludeFolderNames;
    QCheckBox* searchInDirs;
    QCheckBox* searchInArchives;
    QCheckBox* followLinks;
    QHash<QString, QCheckBox*> extraOptions;

    KURLListRequester *searchIn;
    KURLListRequester *dontSearchIn;
    QLayout *middleLayout;

    KrHistoryComboBox* searchFor;
    KHistoryComboBox* containsText;
    KHistoryComboBox* excludeFolderNames;
    QToolButton*      containsRegExp;

    KComboBox* ofType;

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
    KHistoryComboBox *createExcludeComboBox(const KConfigGroup &group);
};

#endif /* GENERALFILTER_H */
