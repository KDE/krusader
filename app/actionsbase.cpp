/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionsbase.h"

#include "icon.h"
#include "krmainwindow.h"

// QtWidgets
#include <QAction>

#include <KActionCollection>
#include <KToggleAction>
#include <utility>

void ActionsBase::ActionGroup::reconnect(QObject *recv)
{
    const auto actions = _slots.keys();
    for (QAction *action : actions) {
        disconnect(action, nullptr, nullptr, nullptr);
        connect(action, SIGNAL(triggered(bool)), recv, _slots[action]);
    }
}

void ActionsBase::ActionGroup::addAction(QAction *action, const char *slot)
{
    _slots.insert(action, slot);
}

QAction *ActionsBase::createAction(const QString &text, const QString &icon, bool isToggleAction)
{
    if (isToggleAction) {
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

QAction *
ActionsBase::action(QString text, QString icon, const QKeySequence &shortcut, QObject *recv, const char *slot, const QString &name, bool isToggleAction)
{
    QAction *a = createAction(std::move(text), std::move(icon), isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcut(a, shortcut);

    return a;
}

QAction *
ActionsBase::action(QString text, QString icon, const QList<QKeySequence> &shortcuts, QObject *recv, const char *slot, const QString &name, bool isToggleAction)
{
    QAction *a = createAction(std::move(text), std::move(icon), isToggleAction);

    connect(a, SIGNAL(triggered(bool)), recv, slot);
    _mainWindow->actions()->addAction(name, a);
    _mainWindow->actions()->setDefaultShortcuts(a, shortcuts);

    return a;
}

QAction *
ActionsBase::action(QString text, QString icon, const QKeySequence &shortcut, ActionGroup &group, const char *slot, const QString &name, bool isToggleAction)
{
    QAction *action = createAction(std::move(text), std::move(icon), isToggleAction);

    group.addAction(action, slot);
    _mainWindow->actions()->addAction(name, action);
    _mainWindow->actions()->setDefaultShortcut(action, shortcut);

    return action;
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, const QKeySequence &shortcut, QObject *recv, const char *slot, QString name)
{
    return qobject_cast<KToggleAction *>(action(std::move(text), std::move(icon), shortcut, recv, slot, std::move(name), true));
}

KToggleAction *ActionsBase::toggleAction(QString text, QString icon, const QKeySequence &shortcut, ActionGroup &group, const char *slot, QString name)
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
