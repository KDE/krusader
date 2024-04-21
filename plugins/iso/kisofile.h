/*
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KISOFILE_H
#define KISOFILE_H

// QtCore
#include <QString>

#include <KArchive>

class KIsoFile : public KArchiveFile
{
public:
    KIsoFile(KArchive *archive,
             const QString &name,
             int access,
             time_t date,
             time_t adate,
             time_t cdate,
             const QString &user,
             const QString &group,
             const QString &symlink,
             long long pos,
             long long size);
    ~KIsoFile();
    void setZF(char algo[2], char parms[2], long long realsize);
    time_t adate() const
    {
        return m_adate;
    }
    time_t cdate() const
    {
        return m_cdate;
    }
    long long realsize() const
    {
        return m_realsize;
    }

    virtual QByteArray dataAt(long long pos, long long count) const;

private:
    /* hide this member function, it's broken by design, because the full
    data often requires too much memory */
    char m_algo[2], m_parms[2];
    long long m_realsize;
    time_t m_adate, m_cdate;
};

#endif
