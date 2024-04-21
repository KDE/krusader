/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KISODIRECTORY_H
#define KISODIRECTORY_H

// QtCore
#include <QString>

#include <KArchive>

class KIsoDirectory : public KArchiveDirectory
{
public:
    KIsoDirectory(KArchive *archive,
                  const QString &name,
                  int access,
                  time_t date,
                  time_t adate,
                  time_t cdate,
                  const QString &user,
                  const QString &group,
                  const QString &symlink);
    ~KIsoDirectory();
    time_t date() const
    {
        return m_date;
    }
    time_t adate() const
    {
        return m_adate;
    }
    time_t cdate() const
    {
        return m_cdate;
    }

private:
    time_t m_date, m_adate, m_cdate;
};

#endif
