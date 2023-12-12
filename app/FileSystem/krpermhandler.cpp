/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krpermhandler.h"

// QtCore
#include <QLocale>

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

QSet<int> KrPermHandler::currentGroups;
QHash<int, QString> KrPermHandler::uidCache;
QHash<int, QString> KrPermHandler::gidCache;

QString KrPermHandler::mode2QString(mode_t m)
{
    char perm[11];
    for (int i = 0; i != 10; i++)
        perm[i] = '-';
    perm[10] = 0;

    if (S_ISLNK(m))
        perm[0] = 'l'; // check for symLink
    if (S_ISDIR(m))
        perm[0] = 'd'; // check for directory

    // ReadUser = 0400, WriteUser = 0200, ExeUser = 0100, Suid = 04000
    if (m & 0400)
        perm[1] = 'r';
    if (m & 0200)
        perm[2] = 'w';
    if (m & 0100)
        perm[3] = 'x';
    if (m & 04000)
        perm[3] = 's';
    // ReadGroup = 0040, WriteGroup = 0020, ExeGroup = 0010, Gid = 02000
    if (m & 0040)
        perm[4] = 'r';
    if (m & 0020)
        perm[5] = 'w';
    if (m & 0010)
        perm[6] = 'x';
    if (m & 02000)
        perm[6] = 's';
    // ReadOther = 0004, WriteOther = 0002, ExeOther = 0001, Sticky = 01000
    if (m & 0004)
        perm[7] = 'r';
    if (m & 0002)
        perm[8] = 'w';
    if (m & 0001)
        perm[9] = 'x';
    if (m & 01000)
        perm[9] = 't';

    return QString(perm);
}

void KrPermHandler::init()
{
    // set the umask to 022
    // umask( 022 );

    // 200 groups should be enough
    gid_t groupList[200];
    int groupNo = getgroups(200, groupList);

// In kdewin32 implementation as of 4.1.2, getpwent always returns the same struct
#ifndef Q_OS_WIN
    // fill the UID cache
    struct passwd *pass;
    while ((pass = getpwent()) != nullptr) {
        uidCache.insert(pass->pw_uid, pass->pw_name);
    }
    delete pass;
    endpwent();

    // fill the GID cache
    struct group *gr;
    while ((gr = getgrent()) != nullptr) {
        gidCache.insert(gr->gr_gid, QString(gr->gr_name));
    }
    delete gr;
    endgrent();
#endif

    // fill the groups for the current user
    for (int i = 0; i < groupNo; ++i) {
        currentGroups.insert(groupList[i]);
    }
    // just to be sure add the effective gid...
    currentGroups.insert(getegid());
}

char KrPermHandler::readable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 0);
}

char KrPermHandler::writeable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 1);
}

char KrPermHandler::executable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 2, true);
}

char KrPermHandler::getLocalPermission(const QString &perm, gid_t gid, uid_t uid, int permOffset, bool ignoreRoot)
{
    // root override
    if (!ignoreRoot && getuid() == 0)
        return ALLOWED_PERM;
    // first check other permissions.
    if (perm[7 + permOffset] != '-')
        return ALLOWED_PERM;
    // now check group permission
    if ((perm[4 + permOffset] != '-') && currentGroups.contains(gid))
        return ALLOWED_PERM;
    // the last chance - user permissions
    if ((perm[1 + permOffset] != '-') && (uid == getuid()))
        return ALLOWED_PERM;
    // sorry !
    return NO_PERM;
}

char KrPermHandler::ftpReadable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 0);
}

char KrPermHandler::ftpWriteable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 1);
}

char KrPermHandler::ftpExecutable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 2);
}

char KrPermHandler::getFtpPermission(const QString &fileOwner, const QString &userName, const QString &perm, int permOffset)
{
    // first check other permissions.
    if (perm[7 + permOffset] != '-')
        return ALLOWED_PERM;
    // can't check group permission !
    // so check the user permissions
    if ((perm[1 + permOffset] != '-') && (fileOwner == userName))
        return ALLOWED_PERM;
    if ((perm[1 + permOffset] != '-') && (userName.isEmpty()))
        return UNKNOWN_PERM;
    if (perm[4 + permOffset] != '-')
        return UNKNOWN_PERM;
    return NO_PERM;
}

QString KrPermHandler::parseSize(KIO::filesize_t val)
{
    return QLocale().toString(val);
}

QString KrPermHandler::gid2group(gid_t groupId)
{
    return gidCache.value(groupId, QStringLiteral("???"));
}

QString KrPermHandler::uid2user(uid_t userId)
{
    return uidCache.value(userId, QStringLiteral("???"));
}
