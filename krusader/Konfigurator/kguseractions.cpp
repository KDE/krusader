/***************************************************************************
                        kguseractions.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas B�r
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kguseractions.h"
#include "../defaults.h"
#include "../ActionMan/actionman.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <QGridLayout>
#include <QLabel>


KgUserActions::KgUserActions(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    QGridLayout *kgUserActionLayout = new QGridLayout(innerWidget);

    // ============= Info Group =============
    QGroupBox *InfoGroup = createFrame(i18n("Information"), innerWidget);
    QGridLayout *InfoGrid = createGridLayout(InfoGroup);

    // terminal for the UserActions
    QLabel *labelInfo = new QLabel(i18n(
                                       "Here you can configure settings about useractions.\n"
                                       "To set up, configure and manage your useractions please use ActionMan."
                                   ), InfoGroup);
    InfoGrid->addWidget(labelInfo, 0, 0);
    KPushButton *actionmanButton = new KPushButton(i18n("Start ActionMan"), InfoGroup);
    connect(actionmanButton, SIGNAL(clicked()), SLOT(startActionMan()));
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
            "Terminal", _UserActions_Terminal, terminalGroup, false);
    terminalGrid->addWidget(urlReqUserActions, 0, 1);

    kgUserActionLayout->addWidget(terminalGroup, 1 , 0);

    // ============= Outputcollection Group =============
    QGroupBox *outputGroup = createFrame(i18n("Output collection"), innerWidget);
    QGridLayout *outputGrid = createGridLayout(outputGroup);

    QWidget *hboxWidget = new QWidget(outputGroup);
    QHBoxLayout *hbox = new QHBoxLayout(hboxWidget);
    QLabel *lbel = new QLabel(i18n("Normal font:"), hboxWidget);
    hbox->addWidget(lbel);

    KonfiguratorFontChooser *chser = createFontChooser("UserActions", "Normal Font", _UserActions_NormalFont, hboxWidget);
    hbox->addWidget(chser);

    QWidget *spcer = createSpacer(hboxWidget);
    hbox->addWidget(spcer);
    outputGrid->addWidget(hboxWidget, 2, 0);

    hboxWidget = new QWidget(outputGroup);
    hbox = new QHBoxLayout(hboxWidget);

    lbel = new QLabel(i18n("Font with fixed width:"), hboxWidget);
    hbox->addWidget(lbel);

    chser = createFontChooser("UserActions", "Fixed Font", _UserActions_FixedFont, hboxWidget);
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


#include "kguseractions.moc"
