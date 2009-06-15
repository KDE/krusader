/***************************************************************************
                                 krpermhandler.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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


#ifndef KRPERMHANDLER_H
#define KRPERMHANDLER_H

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <sys/types.h>
#include <QtCore/QHash>
#include <kio/global.h>

#define NO_PERM      0
#define UNKNOWN_PERM 1
#define ALLOWED_PERM 2

class KRpermHandler
{
public:
    KRpermHandler() {}
    ~KRpermHandler() {}

    static void init();

    static gid_t group2gid(QString group);
    static uid_t user2uid(QString user);

    static QString gid2group(gid_t groupId);
    static QString uid2user(uid_t userId);

    static char writeable(QString perm, gid_t gid, uid_t uid, int rwx = -1);
    static char readable(QString perm, gid_t gid, uid_t uid, int rwx = -1);
    static char executable(QString perm, gid_t gid, uid_t uid, int rwx = -1);

    static bool fileWriteable(QString localFile);
    static bool fileReadable(QString localFile);
    static bool fileExecutable(QString localFile);

    static char ftpWriteable(QString fileOwner, QString userName, QString perm);
    static char ftpReadable(QString fileOwner, QString userName, QString perm);
    static char ftpExecutable(QString fileOwner, QString userName, QString perm);

    static bool dirExist(QString path);
    static bool fileExist(QString fullPath);
    static bool fileExist(QString Path, QString name);

    static QString mode2QString(mode_t m);
    static QString parseSize(KIO::filesize_t val);
    static QString date2qstring(QString date);
    static time_t  QString2time(QString date);

private:
    // cache for passwd and group entries
    static QHash<QString, uid_t> *passwdCache;
    static QHash<QString, gid_t> *groupCache;
    static QHash<int, char>      *currentGroups;
    static QHash<int, QString>   *uidCache;
    static QHash<int, QString>   *gidCache;
};

#endif
