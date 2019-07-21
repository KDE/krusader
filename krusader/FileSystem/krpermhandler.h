/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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
