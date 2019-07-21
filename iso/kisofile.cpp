/*****************************************************************************
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
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

#include "kisofile.h"

KIsoFile::KIsoFile(KArchive* archive, const QString& name, int access,
		   int date, int adate, int cdate, const QString& user, const QString& group,
		   const QString& symlink, long long pos, long long size) :
	KArchiveFile(archive, name, access, QDateTime::fromTime_t(date), user, group, symlink, pos, size)
{


    m_adate = adate;
    m_cdate = cdate;
    m_algo[0] = 0;m_algo[1] = 0;m_parms[0] = 0;m_parms[1] = 0;m_realsize = 0;
}

KIsoFile::~KIsoFile()
= default;

void KIsoFile::setZF(char algo[2], char parms[2], long long realsize)
{
    m_algo[0] = algo[0];m_algo[1] = algo[1];
    m_parms[0] = parms[0];m_parms[1] = parms[1];
    m_realsize = realsize;
}

QByteArray KIsoFile::dataAt(long long pos, int count) const
{
    QByteArray r;
    int rlen;

    if (archive()->device()->seek(position() + pos)) {
        r.resize(((pos + count) < size()) ? count : size() - pos);
        if (r.size()) {
            rlen = archive()->device()->read(r.data(), r.size());
            if (rlen == - 1) r.resize(0);
            else if (rlen != (int)r.size()) r.resize(rlen);
        }
    }

    return r;
}
