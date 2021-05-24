/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew [https://krusader.org]

    This file is part of Krusader [https://krusader.org].

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
