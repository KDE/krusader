/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
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

#include "actionsbase.h"

#include "krmainwindow.h"
#include "icon.h"

// QtWidgets
#include <QAction>

#include <KXmlGui/KActionCollection>
#include <KWidgetsAddons/KToggleAction>

void ActionsBase::ActionGroup::reconnect(QObject *recv)
{
    foreach(QAction *action, _slots.keys()) {
        disconnect(action, 0, 0, 0);
        connect(action, SIGNAL(triggered(bool)), recv, _slots[action]);
    }
}

void ActionsBase::ActionGroup::addAction(QAction *action, const char *slot)
{
    _slots.insert(action, slot);
}


QAction *ActionsBase::createAction(QString text, QString icon, bool isToggleAction)
{
    if(isToggleAction) {
        if (icon == 0)
            return (QAction *)(new KToggleAction(text, this));
        else
            return (QAction *)(new KToggleAction(Icon(icon), text, this));
    } else {
        if (icon == 0)
            return new QAction(text, this);
        else
            return new QAction(Icon(icon), text, this);
    }
}

QAction *ActionsBase::action(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, bool isToggleAction)
{
    QAction *a = createAction(text, icon, isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcut(a, shortcut);

    return a;
}

QAction *ActionsBase::action(QString text, QString icon, const QList<QKeySequence> &shortcuts,
                             QObject *recv, const char *slot, QString name, bool isToggleAction)
{
    QAction *a = createAction(text, icon, isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcuts(a, shortcuts);

    return a;
}

QAction *ActionsBase::action(QString text, QString icon, QKeySequence shortcut,
                             ActionGroup &group, const char *slot, QString name, bool isToggleAction)
{
    QAction *action = createAction(text, icon, isToggleAction);

    group.addAction(action, slot);
    _mainWindow->actions()->addAction(name, action);
    _mainWindow->actions()->setDefaultShortcut(action, shortcut);

    return action;
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, QKeySequence shortcut,
                                QObject *recv, const char *slot, QString name)
{
    return (KToggleAction *)(action(text, icon, shortcut, recv, slot, name, true));
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, QKeySequence shortcut,
                                         ActionGroup &group, const char *slot, QString name)
{
    return (KToggleAction *)(action(text, icon, shortcut, group, slot, name, true));
}

QAction *ActionsBase::stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot)
{
    return KStandardAction::create(id, recv, slot, _mainWindow->actions());
}

QAction *ActionsBase::stdAction(KStandardAction::StandardAction id, ActionGroup &group, const char *slot)
{
    QAction *action = KStandardAction::create(id, 0, 0, _mainWindow->actions());
    group.addAction(action, slot);
    return action;
}
