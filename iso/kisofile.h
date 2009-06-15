/***************************************************************************
                          kisofile.h  -  description
                             -------------------
    begin                : Wed Oct 30 2002
    copyright            : (C) 2002 by Szombathelyi Gy�gy
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KISOFILE_H
#define KISOFILE_H

#include <QtCore/QString>
#include <karchive.h>

/**
  *@author Szombathelyi Gy�gy
  */

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

    virtual QByteArray data(long long pos, int count) const;
private:
    /* hide this member function, it's broken by design, because the full
    data often requires too much memory */
    char m_algo[2], m_parms[2];
    long long m_realsize;
    int m_adate, m_cdate;
    long long m_curpos;
};

#endif
