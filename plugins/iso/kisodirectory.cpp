/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Szombathelyi György <gyurco@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kisodirectory.h"

KIsoDirectory::KIsoDirectory(KArchive *archive,
                             const QString &name,
                             int access,
                             time_t date,
                             time_t adate,
                             time_t cdate,
                             const QString &user,
                             const QString &group,
                             const QString &symlink)
    : KArchiveDirectory(archive, name, access, QDateTime::fromSecsSinceEpoch(static_cast<uint>(date)), user, group, symlink)
{
    m_adate = adate;
    m_cdate = cdate;
}

KIsoDirectory::~KIsoDirectory() = default;
