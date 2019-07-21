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

#ifndef KISOFILE_H
#define KISOFILE_H

// QtCore
#include <QString>

#include <KArchive/KArchive>

class KIsoFile : public KArchiveFile
{
public:
    KIsoFile(KArchive* archive, const QString& name, int access, int date,
             int adate, int cdate, const QString& user, const QString& group,
             const QString& symlink, long long pos, long long size);
    ~KIsoFile();
    void setZF(char algo[2], char parms[2], long long realsize);
    int adate() const {
        return m_adate;
    }
    int cdate() const {
        return m_cdate;
    }
    long long realsize() const {
        return m_realsize;
    }

    virtual QByteArray dataAt(long long pos, int count) const;
private:
    /* hide this member function, it's broken by design, because the full
    data often requires too much memory */
    char m_algo[2], m_parms[2];
    long long m_realsize;
    int m_adate, m_cdate;
    long long m_curpos;
};

#endif
