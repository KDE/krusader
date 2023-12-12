/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRPERMHANDLER_H
#define KRPERMHANDLER_H

// QtCore
#include <QHash>
#include <QSet>
#include <QString>

#include <KIO/Global>

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NO_PERM 0
#define UNKNOWN_PERM 1
#define ALLOWED_PERM 2

class KrPermHandler
{
public:
    static void init();

    static QString gid2group(gid_t groupId);
    static QString uid2user(uid_t userId);

    static char writeable(const QString &perm, gid_t gid, uid_t uid);
    static char readable(const QString &perm, gid_t gid, uid_t uid);
    static char executable(const QString &perm, gid_t gid, uid_t uid);

    static char ftpWriteable(const QString &fileOwner, const QString &userName, const QString &perm);
    static char ftpReadable(const QString &fileOwner, const QString &userName, const QString &perm);
    static char ftpExecutable(const QString &fileOwner, const QString &userName, const QString &perm);

    static QString mode2QString(mode_t m);
    static QString parseSize(KIO::filesize_t val);

private:
    KrPermHandler()
    {
    }
    static char getLocalPermission(const QString &perm, gid_t gid, uid_t uid, int permOffset, bool ignoreRoot = false);
    static char getFtpPermission(const QString &fileOwner, const QString &userName, const QString &perm, int permOffset);

    static QSet<int> currentGroups;
    static QHash<int, QString> uidCache;
    static QHash<int, QString> gidCache;
};

#endif
