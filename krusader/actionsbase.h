/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef ACTIONSBASE_H
#define ACTIONSBASE_H

// QtCore
#include <QObject>
#include <QHash>
// QtGui
#include <QKeySequence>
// QtWidgets
#include <QAction>

#include <KConfigWidgets/KStandardAction>

class KrMainWindow;

class ActionsBase : public QObject
{
    Q_OBJECT
protected:
    ActionsBase(QObject *parent, KrMainWindow *mainWindow) : QObject(parent),
                _mainWindow(mainWindow) {}
    class ActionGroup
    {
        QHash<QAction*, const char*> _slots;
    public:
        void reconnect(QObject *recv);
        void addAction(QAction *action, const char *slot);
    };

    QAction *createAction(QString text, QString icon, bool isToggleAction);

    QAction *action(QString text, QString icon, QKeySequence shortcut,
                    QObject *recv, const char *slot, QString name, bool isToggleAction = false);
    QAction *action(QString text, QString icon, QKeySequence shortcut,
                    const char *slot, QString name) {
        return action(text, icon, shortcut, this, slot, name);
    }
    QAction *action(QString text, QString icon, const QList<QKeySequence> &shortcuts,
                    QObject *recv, const char *slot, QString name, bool isToggleAction = false);
    QAction *action(QString text, QString icon, QKeySequence shortcut,
                    ActionGroup &group, const char *slot, QString name, bool isToggleAction = false);

    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                QObject *recv, const char *slot, QString name);
    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                const char *slot, QString name) {
        return toggleAction(text, icon, shortcut, this, slot, name);
    }
    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                ActionGroup &group, const char *slot, QString name);

    QAction *stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot);
    QAction *stdAction(KStandardAction::StandardAction id, const char *slot) {
        return stdAction(id, this, slot);
    }
    QAction *stdAction(KStandardAction::StandardAction id,
                       ActionGroup &group, const char *slot);

    KrMainWindow *_mainWindow;
};

#endif // ACTIONSBASE_H
