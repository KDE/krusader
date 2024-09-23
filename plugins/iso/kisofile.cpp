/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kisofile.h"

KIsoFile::KIsoFile(KArchive *archive,
                   const QString &name,
                   int access,
                   time_t date,
                   time_t adate,
                   time_t cdate,
                   const QString &user,
                   const QString &group,
                   const QString &symlink,
                   long long pos,
                   long long size)
    : KArchiveFile(archive, name, access, QDateTime::fromSecsSinceEpoch(static_cast<uint>(date)), user, group, symlink, pos, size)
{
    m_adate = adate;
    m_cdate = cdate;
    m_algo[0] = 0;
    m_algo[1] = 0;
    m_parms[0] = 0;
    m_parms[1] = 0;
    m_realsize = 0;
}

KIsoFile::~KIsoFile() = default;

void KIsoFile::setZF(char algo[2], char parms[2], long long realsize)
{
    m_algo[0] = algo[0];
    m_algo[1] = algo[1];
    m_parms[0] = parms[0];
    m_parms[1] = parms[1];
    m_realsize = realsize;
}

QByteArray KIsoFile::dataAt(long long pos, long long count) const
{
    QByteArray r;
    qint64 rlen;

    if (archive()->device()->seek(position() + pos)) {
        r.resize(static_cast<int>(((pos + count) < size()) ? count : size() - pos));
        if (r.size()) {
            rlen = archive()->device()->read(r.data(), r.size());
            if (rlen == -1)
                r.resize(0);
            else if (rlen != (int)r.size())
                r.resize(static_cast<int>(rlen));
        }
    }

    return r;
}
