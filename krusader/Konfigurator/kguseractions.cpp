/*****************************************************************************
 * Copyright (C) 2004 Jonas Bähr <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#include "kguseractions.h"
#include "../defaults.h"
#include "../ActionMan/actionman.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include <KI18n/KLocalizedString>

KgUserActions::KgUserActions(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgUserActionLayout = new QGridLayout(innerWidget);

    // ============= Info Group =============
    QGroupBox *InfoGroup = createFrame(i18n("Information"), innerWidget);
    QGridLayout *InfoGrid = createGridLayout(InfoGroup);

    // terminal for the UserActions
    QLabel *labelInfo = new QLabel(i18n(
                                       "Here you can configure settings about useractions.\n"
                                       "To set up, configure and manage your useractions please use ActionMan."
                                   ), InfoGroup);
    InfoGrid->addWidget(labelInfo, 0, 0);
    QPushButton *actionmanButton = new QPushButton(i18n("Start ActionMan"), InfoGroup);
    connect(actionmanButton, &QPushButton::clicked, this, &KgUserActions::startActionMan);
    InfoGrid->addWidget(actionmanButton, 1, 0);

    kgUserActionLayout->addWidget(InfoGroup, 0 , 0);

    // ============= Terminal Group =============
    QGroupBox *terminalGroup = createFrame(i18n("Terminal execution"), innerWidget);
    QGridLayout *terminalGrid = createGridLayout(terminalGroup);

    // terminal for the UserActions
    QLabel *labelTerminal = new QLabel(i18n("Terminal for UserActions:"),
                                       terminalGroup);
    terminalGrid->addWidget(labelTerminal, 0, 0);
    KonfiguratorURLRequester *urlReqUserActions = createURLRequester("UserActions",
            "Terminal", _UserActions_Terminal, labelTerminal, terminalGroup, false, FIRST_PAGE, false);
    terminalGrid->addWidget(urlReqUserActions, 0, 1);
    labelTerminal = new QLabel(i18n("%t will be replaced by the title of the action,\n%d with the workdir."),
                                       terminalGroup);
    terminalGrid->addWidget(labelTerminal, 1, 1);

    kgUserActionLayout->addWidget(terminalGroup, 1 , 0);

    // ============= Outputcollection Group =============
    QGroupBox *outputGroup = createFrame(i18n("Output collection"), innerWidget);
    QGridLayout *outputGrid = createGridLayout(outputGroup);

    QWidget *hboxWidget = new QWidget(outputGroup);
    auto *hbox = new QHBoxLayout(hboxWidget);
    QLabel *lbel = new QLabel(i18n("Normal font:"), hboxWidget);
    hbox->addWidget(lbel);

    KonfiguratorFontChooser *chser = createFontChooser("UserActions", "Normal Font",
                                                       _UserActions_NormalFont, lbel, hboxWidget);
    hbox->addWidget(chser);

    QWidget *spcer = createSpacer(hboxWidget);
    hbox->addWidget(spcer);
    outputGrid->addWidget(hboxWidget, 2, 0);

    hboxWidget = new QWidget(outputGroup);
    hbox = new QHBoxLayout(hboxWidget);

    lbel = new QLabel(i18n("Font with fixed width:"), hboxWidget);
    hbox->addWidget(lbel);

    chser = createFontChooser("UserActions", "Fixed Font",
                              _UserActions_FixedFont, lbel, hboxWidget);
    hbox->addWidget(chser);

    spcer = createSpacer(hboxWidget);
    hbox->addWidget(spcer);

    outputGrid->addWidget(hboxWidget, 3, 0);

    KonfiguratorCheckBox *useFixed = createCheckBox("UserActions", "Use Fixed Font", _UserActions_UseFixedFont,
                                     i18n("Use fixed width font as default"), outputGroup);
    outputGrid->addWidget(useFixed, 4, 0);

    kgUserActionLayout->addWidget(outputGroup, 2 , 0);
}

void KgUserActions::startActionMan()
{
    ActionMan actionMan(static_cast<QWidget*>(this));
}

