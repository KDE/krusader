/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONSBASE_H
#define ACTIONSBASE_H

// QtCore
#include <QHash>
#include <QObject>
// QtGui
#include <QKeySequence>
// QtWidgets
#include <QAction>

#include <KStandardAction>

class KrMainWindow;

class ActionsBase : public QObject
{
    Q_OBJECT
protected:
    ActionsBase(QObject *parent, KrMainWindow *mainWindow)
        : QObject(parent)
        , _mainWindow(mainWindow)
    {
    }
    class ActionGroup
    {
        QHash<QAction *, const char *> _slots;

    public:
        void reconnect(QObject *recv);
        void addAction(QAction *action, const char *slot);
    };

    QAction *createAction(const QString &text, const QString &icon, bool isToggleAction);

    QAction *
    action(QString text, QString icon, const QKeySequence &shortcut, QObject *recv, const char *slot, const QString &name, bool isToggleAction = false);
    QAction *action(QString text, QString icon, QKeySequence shortcut, const char *slot, QString name)
    {
        return action(text, icon, shortcut, this, slot, name);
    }
    QAction *
    action(QString text, QString icon, const QList<QKeySequence> &shortcuts, QObject *recv, const char *slot, const QString &name, bool isToggleAction = false);
    QAction *
    action(QString text, QString icon, const QKeySequence &shortcut, ActionGroup &group, const char *slot, const QString &name, bool isToggleAction = false);

    KToggleAction *toggleAction(QString text, QString icon, const QKeySequence &shortcut, QObject *recv, const char *slot, QString name);
    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut, const char *slot, QString name)
    {
        return toggleAction(text, icon, shortcut, this, slot, name);
    }
    KToggleAction *toggleAction(QString text, QString icon, const QKeySequence &shortcut, ActionGroup &group, const char *slot, QString name);

    QAction *stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot);
    QAction *stdAction(KStandardAction::StandardAction id, const char *slot)
    {
        return stdAction(id, this, slot);
    }
    QAction *stdAction(KStandardAction::StandardAction id, ActionGroup &group, const char *slot);

    KrMainWindow *_mainWindow;
};

#endif // ACTIONSBASE_H
