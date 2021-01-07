/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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

#include "kfnkeys.h"

// QtWidgets
#include <QGridLayout>

#include <KI18n/KLocalizedString>

#include "../defaults.h"
#include "../krmainwindow.h"
#include "../kractions.h"
#include "../Panel/listpanelactions.h"

KFnKeys::KFnKeys(QWidget *parent, KrMainWindow *mainWindow) :
        QWidget(parent), mainWindow(mainWindow)
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

    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    int pos = 0;
    for(QPair<QPushButton *, QPair<QAction *, const QString&>> entry : qAsConst(buttonList)) {
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
    auto *button = new QPushButton(this);
    button->setMinimumWidth(45);
    button->setToolTip(action->toolTip());
    connect(button, &QPushButton::clicked, action, &QAction::trigger);
    return QPair<QPushButton *, QPair<QAction *, QString>>(button,
                                                           QPair<QAction *, QString>(action, text));
}
