/***************************************************************************
                                krdirwatch.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

                                                     S o u rc e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "krdirwatch.h"
#include <unistd.h>
#include <sys/types.h>
#include <kdebug.h>
#include "krpermhandler.h"
#include "qstringlist.h"
#include <QtCore/QDir>

KRdirWatch::KRdirWatch(int msec, bool dirOnly):
        delay(msec), t(this), changed(false)
{
    if (dirOnly) dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
    watched.setAutoDelete(true);
    connect(&t, SIGNAL(timeout()), this, SLOT(checkDirs()));
    startScan();
}

KRdirWatch::~KRdirWatch()
{
    clearList();
    stopScan();
}

void KRdirWatch::removeDir(QString path)
{
    t.stop();
    for (it = watched.first(); it != 0;)
        if (it->path == path) watched.remove();
        else it = watched.next();
    if (!stopped) startScan();
}

void KRdirWatch::addDir(QString path, bool checkPermissions)
{
    t.stop();

    krDirEntry* temp = new krDirEntry;
    if (!dir.cd(path)) { // if it's not a dir or don't exist - don't add it
        //kDebug() << "KRDirWatch: can't watch " + path +", (don't exist)" << endl;
        return;
    }
    if (checkPermissions) {
        // if we can't read it - don't bother
        if (getgid() != 0 && !KRpermHandler::fileReadable(path)) {
            //kDebug() << "KRDirWatch: can't watch " + path +", (not readable)" << endl;
            return;
        }
        if (!KRpermHandler::fileWriteable(path)) { // read-only directories can't be changed
            //kDebug() << "KRDirWatch: not watching " + path +", (read-only directory)" << endl;
            return;
        }
    }
    qfi.setFile(path);

    temp->path = dir.path();
    temp->count = dir.entryList(QDir::TypeMask | QDir::AccessMask).count();
    temp->lastModified = qfi.lastModified();

    watched.append(temp);
    if (!stopped) startScan();
}

// here we do the actual checking
void KRdirWatch::checkDirs()
{
    t.stop();

    QString path;
    unsigned long count;
    QDateTime dt;

    for (it = watched.first(); it != 0; it = watched.next()) {
        path = it->path;
        qfi.setFile(path);
        if (!dir.cd(path)) {
            clearList();
            emit dirty();
            return;
        }
        dt = qfi.lastModified();
        count = dir.entryList(QDir::TypeMask | QDir::AccessMask).count();
        // check for changes
        if (it->lastModified != dt || it->count != count) {
            changed = true;
            it->lastModified = dt;
            it->count = count;
            startScan();
            return;
        }
        if (changed) {
            changed = false;
            clearList();
            emit dirty();
            return;
        }
    }
    if (!stopped) startScan();
}

#include "krdirwatch.moc"
