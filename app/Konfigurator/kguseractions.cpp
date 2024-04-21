/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kguseractions.h"
#include "../ActionMan/actionman.h"
#include "../defaults.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include <KLocalizedString>

KgUserActions::KgUserActions(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgUserActionLayout = new QGridLayout(innerWidget);

    // ============= Info Group =============
    QGroupBox *InfoGroup = createFrame(i18n("Information"), innerWidget);
    QGridLayout *InfoGrid = createGridLayout(InfoGroup);

    // terminal for the UserActions
    QLabel *labelInfo = new QLabel(i18n("Here you can configure settings about useractions.\n"
                                        "To set up, configure and manage your useractions please use ActionMan."),
                                   InfoGroup);
    InfoGrid->addWidget(labelInfo, 0, 0);
    QPushButton *actionmanButton = new QPushButton(i18n("Start ActionMan"), InfoGroup);
    connect(actionmanButton, &QPushButton::clicked, this, &KgUserActions::startActionMan);
    InfoGrid->addWidget(actionmanButton, 1, 0);

    kgUserActionLayout->addWidget(InfoGroup, 0, 0);

    // ============= Terminal Group =============
    QGroupBox *terminalGroup = createFrame(i18n("Terminal execution"), innerWidget);
    QGridLayout *terminalGrid = createGridLayout(terminalGroup);

    // terminal for the UserActions
    QLabel *labelTerminal = new QLabel(i18n("Terminal for UserActions:"), terminalGroup);
    terminalGrid->addWidget(labelTerminal, 0, 0);
    KonfiguratorURLRequester *urlReqUserActions =
        createURLRequester("UserActions", "Terminal", _UserActions_Terminal, labelTerminal, terminalGroup, false, QString(), FIRST_PAGE, false);
    terminalGrid->addWidget(urlReqUserActions, 0, 1);
    labelTerminal = new QLabel(i18n("%t will be replaced by the title of the action,\n%d with the workdir."), terminalGroup);
    terminalGrid->addWidget(labelTerminal, 1, 1);

    kgUserActionLayout->addWidget(terminalGroup, 1, 0);

    // ============= Outputcollection Group =============
    QGroupBox *outputGroup = createFrame(i18n("Output collection"), innerWidget);
    QGridLayout *outputGrid = createGridLayout(outputGroup);

    QWidget *hboxWidget = new QWidget(outputGroup);
    auto *hbox = new QHBoxLayout(hboxWidget);
    QLabel *lbel = new QLabel(i18n("Normal font:"), hboxWidget);
    hbox->addWidget(lbel);

    KonfiguratorFontChooser *chser = createFontChooser("UserActions", "Normal Font", _UserActions_NormalFont, lbel, hboxWidget);
    hbox->addWidget(chser);

    QWidget *spcer = createSpacer(hboxWidget);
    hbox->addWidget(spcer);
    outputGrid->addWidget(hboxWidget, 2, 0);

    hboxWidget = new QWidget(outputGroup);
    hbox = new QHBoxLayout(hboxWidget);

    lbel = new QLabel(i18n("Font with fixed width:"), hboxWidget);
    hbox->addWidget(lbel);

    chser = createFontChooser("UserActions", "Fixed Font", _UserActions_FixedFont, lbel, hboxWidget);
    hbox->addWidget(chser);

    spcer = createSpacer(hboxWidget);
    hbox->addWidget(spcer);

    outputGrid->addWidget(hboxWidget, 3, 0);

    KonfiguratorCheckBox *useFixed =
        createCheckBox("UserActions", "Use Fixed Font", _UserActions_UseFixedFont, i18n("Use fixed width font as default"), outputGroup);
    outputGrid->addWidget(useFixed, 4, 0);

    kgUserActionLayout->addWidget(outputGroup, 2, 0);
}

void KgUserActions::startActionMan()
{
    ActionMan actionMan(static_cast<QWidget *>(this));
}
