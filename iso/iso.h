/*****************************************************************************
 * Copyright (C) 2000 David Faure <faure@kde.org>                            *
 * Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>                       *
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
 * This file is heavily based on ktar from kdelibs                           *
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

#ifndef ISO_H
#define ISO_H

#include <QByteArray>

#include <kio/slavebase.h>
#include <sys/types.h>

#include "kisofile.h"

class KIso;

class kio_isoProtocol : public KIO::SlaveBase
{
public:
    kio_isoProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~kio_isoProtocol();

    virtual void listDir(const KUrl & url);
    virtual void stat(const KUrl & url);
    virtual void get(const KUrl & url);

protected:
    void getFile(const KIsoFile *isoFileEntry, const QString &path);
    void createUDSEntry(const KArchiveEntry * isoEntry, KIO::UDSEntry & entry);
    bool checkNewFile(QString fullPath, QString & path, int startsec);
    QString getPath(const KUrl & url);

    KIso * m_isoFile;
    time_t m_mtime;
    int m_mode;
};

#endif
