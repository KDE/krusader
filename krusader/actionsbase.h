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

#ifndef __ACTIONSBASE_H__
#define __ACTIONSBASE_H__

#include <QObject>
#include <QKeySequence>
#include <QHash>
#include <kstandardaction.h>


class KrMainWindow;

class ActionsBase : public QObject
{
    Q_OBJECT
protected:
    ActionsBase(QObject *parent, KrMainWindow *mainWindow) : QObject(parent),
                _mainWindow(mainWindow) {}
    class ActionGroup
    {
        QHash<KAction*, const char*> _slots;
    public:
        void reconnect(QObject *recv);
        void addAction(KAction *action, const char *slot);
    };

    KAction *createAction(QString text, QString icon, bool isToggleAction);

    KAction *action(QString text, QString icon, QKeySequence shortcut,
                    QObject *recv, const char *slot, QString name, bool isToggleAction = false);
    KAction *action(QString text, QString icon, QKeySequence shortcut,
                    const char *slot, QString name) {
        return action(text, icon, shortcut, this, slot, name);
    }
    KAction *action(QString text, QString icon, QKeySequence shortcut,
                    ActionGroup &group, const char *slot, QString name, bool isToggleAction = false);

    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                QObject *recv, const char *slot, QString name);
    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                const char *slot, QString name) {
        return toggleAction(text, icon, shortcut, this, slot, name);
    }
    KToggleAction *toggleAction(QString text, QString icon, QKeySequence shortcut,
                                ActionGroup &group, const char *slot, QString name);

    KAction *stdAction(KStandardAction::StandardAction id, QObject *recv, const char *slot);
    KAction *stdAction(KStandardAction::StandardAction id, const char *slot) {
        return stdAction(id, this, slot);
    }
    KAction *stdAction(KStandardAction::StandardAction id,
                       ActionGroup &group, const char *slot);

    KrMainWindow *_mainWindow;
};

#endif // __ACTIONSBASE_H__
