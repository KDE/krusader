/***************************************************************************
                                krdirwatch.h
                             -------------------
    begin                : Thu May 4 2000
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KRDIRWATCH_H
#define KRDIRWATCH_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QDir>


typedef struct krDirEntry_s {
    QString path;
    unsigned long count; // number of files
    QDateTime lastModified;
} krDirEntry ;

class KRdirWatch : public QObject
{
    Q_OBJECT
public:
    KRdirWatch(int msec = 250 , bool dirOnly = false);
    ~KRdirWatch();
    void addDir(QString path, bool checkPermissions = true);
    void removeDir(QString path);

    inline void clearList() {
        watched.clear();
    }
    inline void startScan() {
        t.start(delay);stopped = false;
    }
    inline void stopScan() {
        t.stop(); stopped = true;
    }

public slots:
    void checkDirs();  // here we do the actual checking

signals:
    void dirty();

protected:
    int delay;      // time in msec between updates
    QList<krDirEntry> watched;
    QTimer t;
    QDir dir;
    QFileInfo qfi;
    krDirEntry* it;
    bool changed; // true if something happend to the watched dirs
    bool stopped; // true if the watcher is stopped
};

#endif
