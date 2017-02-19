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

// QtCore
#include <QHash>
#include <QSet>
#include <QString>

#include <KIO/Global>

#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define NO_PERM      0
#define UNKNOWN_PERM 1
#define ALLOWED_PERM 2

class KRpermHandler
{
public:
    static void init();

    static QString gid2group(gid_t groupId);
    static QString uid2user(uid_t userId);

    static char writeable(const QString &perm, gid_t gid, uid_t uid);
    static char readable(const QString &perm, gid_t gid, uid_t uid);
    static char executable(const QString &perm, gid_t gid, uid_t uid);

    static char ftpWriteable(const QString &fileOwner, const QString & userName, const QString &perm);
    static char ftpReadable(const QString &fileOwner, const QString &userName, const QString &perm);
    static char ftpExecutable(const QString &fileOwner, const QString &userName, const QString &perm);

    static QString mode2QString(mode_t m);
    static QString parseSize(KIO::filesize_t val);

private:
    KRpermHandler() {}
    static char getLocalPermission(const QString &perm, gid_t gid, uid_t uid, int permOffset,
                                   bool ignoreRoot = false);
    static char getFtpPermission(const QString &fileOwner, const QString &userName,
                                 const QString &perm, int permOffset);

    static QSet<int> currentGroups;
    static QHash<int, QString> uidCache;
    static QHash<int, QString> gidCache;
};

#endif
