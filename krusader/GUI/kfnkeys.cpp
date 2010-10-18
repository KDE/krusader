/***************************************************************************
                                kfnkeys.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#include "kfnkeys.h"

#include <QGridLayout>
#include <QtGui/QFontMetrics>

#include <klocale.h>
#include <kglobalsettings.h>

#include "../krglobal.h"
#include "../kractions.h"
#include "../defaults.h"
#include "../krslots.h"

KFnKeys::KFnKeys(QWidget *parent): QWidget(parent)
{
    ////////////////////////////////
#define SETUP(TARGET) {\
        TARGET = new QPushButton(this); \
        TARGET->setMinimumWidth(45);\
        TARGET->setToolTip(actions->act##TARGET->toolTip()); \
        connect(TARGET, SIGNAL(clicked()), actions->act##TARGET, SLOT(trigger())); \
    }

    ListPanelActions *actions = ListPanelActions::self;

    setFont(KGlobalSettings::generalFont());
    layout = new QGridLayout(this); // 9 keys
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    SETUP(F2);
    SETUP(F3);
    SETUP(F4);
    SETUP(F5);
    SETUP(F6);
    SETUP(F7);
    SETUP(F8);
    SETUP(F9);

    F10 = new QPushButton(this);
    F10->setToolTip(i18n("Quit Krusader."));
    connect(F10, SIGNAL(clicked()), krMainWindow, SLOT(slotClose()));
    F10->setMinimumWidth(45);

    updateShortcuts();
    /*
        // set a tighter box around the keys
        int h = QFontMetrics(F2->font()).height()+2;
        F2->setMaximumHeight(h); F3->setMaximumHeight(h);
        F4->setMaximumHeight(h); F5->setMaximumHeight(h);
        F6->setMaximumHeight(h); F7->setMaximumHeight(h);
        F8->setMaximumHeight(h); F9->setMaximumHeight(h);
        F10->setMaximumHeight(h);
    */
    layout->addWidget(F2, 0, 0);
    layout->addWidget(F3, 0, 1);
    layout->addWidget(F4, 0, 2);
    layout->addWidget(F5, 0, 3);
    layout->addWidget(F6, 0, 4);
    layout->addWidget(F7, 0, 5);
    layout->addWidget(F8, 0, 6);
    layout->addWidget(F9, 0, 7);
    layout->addWidget(F10, 0, 8);

    layout->activate();
}

void KFnKeys::updateShortcuts()
{
    //FIXME: use QAction::text()
    F2->setText(krF2->shortcut().toString() + i18n(" Term"));
    F3->setText(krF3->shortcut().toString() + i18n(" View"));
    F4->setText(krF4->shortcut().toString() + i18n(" Edit"));
    F5->setText(krF5->shortcut().toString() + i18n(" Copy"));
    F6->setText(krF6->shortcut().toString() + i18n(" Move"));
    F7->setText(krF7->shortcut().toString() + i18n(" Mkdir"));
    F8->setText(krF8->shortcut().toString() + i18n(" Delete"));
    F9->setText(krF9->shortcut().toString() + i18n(" Rename"));
    F10->setText(krF10->shortcut().toString() + i18n(" Quit"));
}

#include "kfnkeys.moc"

