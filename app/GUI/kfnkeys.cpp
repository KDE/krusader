/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kfnkeys.h"

// QtWidgets
#include <QGridLayout>

#include <KLocalizedString>

#include "../Panel/listpanelactions.h"
#include "../defaults.h"
#include "../kractions.h"
#include "../krmainwindow.h"

KFnKeys::KFnKeys(QWidget *parent, KrMainWindow *mainWindow)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    buttonList << setup(mainWindow->listPanelActions()->actRenameF2, i18n("Rename")) << setup(mainWindow->listPanelActions()->actViewFileF3, i18n("View"))
               << setup(mainWindow->listPanelActions()->actEditFileF4, i18n("Edit")) << setup(mainWindow->listPanelActions()->actCopyF5, i18n("Copy"))
               << setup(mainWindow->listPanelActions()->actMoveF6, i18n("Move")) << setup(mainWindow->listPanelActions()->actNewFolderF7, i18n("Mkdir"))
               << setup(mainWindow->listPanelActions()->actDeleteF8, i18n("Delete")) << setup(mainWindow->listPanelActions()->actTerminalF9, i18n("Term"))
               << setup(mainWindow->krActions()->actF10Quit, i18n("Quit"));

    updateShortcuts();

    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    int pos = 0;
    for (QPair<QPushButton *, QPair<QAction *, const QString &>> entry : std::as_const(buttonList)) {
        layout->addWidget(entry.first, 0, pos++);
    }
    layout->activate();
}

void KFnKeys::updateShortcuts()
{
    for (ButtonEntry entry : buttonList) {
        entry.first->setText(entry.second.first->shortcut().toString() + ' ' + entry.second.second);
    }
}

KFnKeys::ButtonEntry KFnKeys::setup(QAction *action, const QString &text)
{
    auto *button = new QPushButton(this);
    button->setMinimumWidth(45);
    button->setToolTip(action->toolTip());
    connect(button, &QPushButton::clicked, action, &QAction::trigger);
    return QPair<QPushButton *, QPair<QAction *, QString>>(button, QPair<QAction *, QString>(action, text));
}
