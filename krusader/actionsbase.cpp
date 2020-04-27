/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2019 Krusader Krew [https://krusader.org]              *
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
#include <utility>

void ActionsBase::ActionGroup::reconnect(QObject *recv)
{
    foreach(QAction *action, _slots.keys()) {
        disconnect(action, nullptr, nullptr, nullptr);
        connect(action, SIGNAL(triggered(bool)), recv, _slots[action]);
    }
}

void ActionsBase::ActionGroup::addAction(QAction *action, const char *slot)
{
    _slots.insert(action, slot);
}


QAction *ActionsBase::createAction(const QString& text, const QString& icon, bool isToggleAction)
{
    if(isToggleAction) {
        if (icon == nullptr)
            return (QAction *)(new KToggleAction(text, this));
        else
            return (QAction *)(new KToggleAction(Icon(icon), text, this));
    } else {
        if (icon == nullptr)
            return new QAction(text, this);
        else
            return new QAction(Icon(icon), text, this);
    }
}

QAction *ActionsBase::action(QString text, QString icon, const QKeySequence& shortcut,
                                 QObject *recv, const char *slot, const QString& name, bool isToggleAction)
{
    QAction *a = createAction(std::move(text), std::move(icon), isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcut(a, shortcut);

    return a;
}

QAction *ActionsBase::action(QString text, QString icon, const QList<QKeySequence> &shortcuts,
                             QObject *recv, const char *slot, const QString& name, bool isToggleAction)
{
    QAction *a = createAction(std::move(text), std::move(icon), isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcuts(a, shortcuts);

    return a;
}

QAction *ActionsBase::action(QString text, QString icon, const QKeySequence& shortcut,
                             ActionGroup &group, const char *slot, const QString& name, bool isToggleAction)
{
    QAction *action = createAction(std::move(text), std::move(icon), isToggleAction);

    group.addAction(action, slot);
    _mainWindow->actions()->addAction(name, action);
    _mainWindow->actions()->setDefaultShortcut(action, shortcut);

    return action;
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, const QKeySequence& shortcut,
                                QObject *recv, const char *slot, QString name)
{
    return qobject_cast<KToggleAction *>(action(std::move(text), std::move(icon), shortcut, recv, slot, std::move(name), true));
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, const QKeySequence& shortcut,
                                         ActionGroup &group, const char *slot, QString name)
{
    return qobject_cast<KToggleAction *>(action(std::move(text), std::move(icon), shortcut, group, slot, std::move(name), true));
}

QAction *ActionsBase::stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot)
{
    return KStandardAction::create(id, recv, slot, _mainWindow->actions());
}

QAction *ActionsBase::stdAction(KStandardAction::StandardAction id, ActionGroup &group, const char *slot)
{
    QAction *action = KStandardAction::create(id, nullptr, nullptr, _mainWindow->actions());
    group.addAction(action, slot);
    return action;
}
