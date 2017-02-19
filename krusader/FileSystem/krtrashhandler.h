/***************************************************************************
                      krtrashhandler.h  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    static QString trashIcon();
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
