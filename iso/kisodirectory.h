/*****************************************************************************
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KISODIRECTORY_H
#define KISODIRECTORY_H

// QtCore
#include <QString>

#include <KArchive/KArchive>

class KIsoDirectory : public KArchiveDirectory
{
public:
    KIsoDirectory(KArchive* archive, const QString& name, int access, int date,
                  int adate, int cdate, const QString& user, const QString& group,
                  const QString& symlink);
    ~KIsoDirectory();
    int date() const {
        return m_date;
    }
    int adate() const {
        return m_adate;
    }
    int cdate() const {
        return m_cdate;
    }
private:
    int m_date, m_adate, m_cdate;
};

#endif
