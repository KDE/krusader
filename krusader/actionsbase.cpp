/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "actionsbase.h"

#include "krmainwindow.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <ktoggleaction.h>

void ActionsBase::ActionGroup::reconnect(QObject *recv)
{
    foreach(KAction *action, _slots.keys()) {
        disconnect(action, 0, 0, 0);
        connect(action, SIGNAL(triggered(bool)), recv, _slots[action]);
    }
}

void ActionsBase::ActionGroup::addAction(KAction *action, const char *slot)
{
    _slots.insert(action, slot);
}


KAction *ActionsBase::createAction(QString text, QString icon, bool isToggleAction)
{
    KAction *a;
    if(isToggleAction) {
        if (icon == 0)
            a = new KToggleAction(text, this);
        else
            a = new KToggleAction(KIcon(icon), text, this);
    } else {
        if (icon == 0)
            a = new KAction(text, this);
        else
            a = new KAction(KIcon(icon), text, this);
    }

    return a;
}

KAction *ActionsBase::action(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, bool isToggleAction)
{
    KAction *a = createAction(text, icon, isToggleAction);

    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);

    return a;
}

KAction *ActionsBase::action(QString text, QString icon, QKeySequence shortcut,
                             ActionGroup &group, const char *slot, QString name, bool isToggleAction)
{
    KAction *action = createAction(text, icon, isToggleAction);

    action->setShortcut(shortcut);
    group.addAction(action, slot);
    _mainWindow->actions()->addAction(name, action);

    return action;
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, QKeySequence shortcut,
                                QObject *recv, const char *slot, QString name)
{
    return static_cast<KToggleAction*>(action(text, icon, shortcut, recv, slot, name, true));
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, QKeySequence shortcut,
                                         ActionGroup &group, const char *slot, QString name)
{
    return static_cast<KToggleAction*>(action(text, icon, shortcut, group, slot, name, true));
}

KAction *ActionsBase::stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot)
{
    return KStandardAction::create(id, recv, slot, _mainWindow->actions());
}

KAction *ActionsBase::stdAction(KStandardAction::StandardAction id, ActionGroup &group, const char *slot)
{
    KAction *action = KStandardAction::create(id, 0, 0, _mainWindow->actions());
    group.addAction(action, slot);
    return action;
}
