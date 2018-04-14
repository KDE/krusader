/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2009-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KRTRASHHANDLER_H
#define KRTRASHHANDLER_H

// QtCore
#include <QString>
#include <QUrl>

#include <KCoreAddons/KDirWatch>

class KrTrashWatcher;

class KrTrashHandler
{
public:
    static bool    isTrashEmpty();
    static QString trashIconName();
    static void    emptyTrash();
    static void    restoreTrashedFiles(const QList<QUrl> &url);
    static void    startWatcher();
    static void    stopWatcher();

private:
    static KrTrashWatcher * _trashWatcher;
};


/** Watches the trashrc config file for changes and updates the trash icon. */
class KrTrashWatcher : public QObject
{
    Q_OBJECT

public:
    KrTrashWatcher();
    virtual ~KrTrashWatcher();

public slots:
    void slotTrashChanged();

private:
    KDirWatch * _watcher;
};

#endif /* __KR_TRASH_HANDLER__ */
