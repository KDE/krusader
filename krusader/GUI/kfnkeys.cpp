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

// QtWidgets
#include <QGridLayout>

#include <KI18n/KLocalizedString>

#include "../defaults.h"
#include "../krmainwindow.h"
#include "../kractions.h"
#include "../Panel/listpanelactions.h"

KFnKeys::KFnKeys(QWidget *parent, KrMainWindow *mainWindow) :
        QWidget(parent), mainWindow(mainWindow), buttonList()
{
    buttonList << setup(mainWindow->listPanelActions()->actRenameF2, i18n("Rename"))
               << setup(mainWindow->listPanelActions()->actViewFileF3, i18n("View"))
               << setup(mainWindow->listPanelActions()->actEditFileF4, i18n("Edit"))
               << setup(mainWindow->listPanelActions()->actCopyF5, i18n("Copy"))
               << setup(mainWindow->listPanelActions()->actMoveF6, i18n("Move"))
               << setup(mainWindow->listPanelActions()->actNewFolderF7, i18n("Mkdir"))
               << setup(mainWindow->listPanelActions()->actDeleteF8, i18n("Delete"))
               << setup(mainWindow->listPanelActions()->actTerminalF9, i18n("Term"))
               << setup(mainWindow->krActions()->actF10Quit, i18n("Quit"));

    updateShortcuts();

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    int pos = 0;
    for(QPair<QPushButton *, QPair<QAction *, const QString&>> entry : buttonList) {
        layout->addWidget(entry.first, 0, pos++);
    }
    layout->activate();
}

void KFnKeys::updateShortcuts()
{
    for(ButtonEntry entry : buttonList) {
        entry.first->setText(entry.second.first->shortcut().toString() + ' ' + entry.second.second);
    }
}

KFnKeys::ButtonEntry KFnKeys::setup(QAction *action, const QString &text)
{
    QPushButton *button = new QPushButton(this);
    button->setMinimumWidth(45);
    button->setToolTip(action->toolTip());
    connect(button, &QPushButton::clicked, action, &QAction::trigger);
    return QPair<QPushButton *, QPair<QAction *, QString>>(button,
                                                           QPair<QAction *, QString>(action, text));
}
