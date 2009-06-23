/*****************************************************************************
 * Copyright (C) 2000 David Faure <faure@kde.org>                            *
 * This file is part of the KDE libraries                                    *
 *                                                                           *
 * This package is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Lesser General Public                *
 * License version 2 as published by the Free Software Foundation.           *
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

#ifndef TAR_H
#define TAR_H

#include <sys/types.h>

#include <QByteArray>

#include <kio/slavebase.h>

class ArchiveProtocol : public KIO::SlaveBase
{
public:
    ArchiveProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~ArchiveProtocol();

    virtual void listDir(const KUrl & url);
    virtual void stat(const KUrl & url);
    virtual void get(const KUrl & url);
    virtual void put(const KUrl& url, int permissions, bool overwrite, bool resume);
    virtual void mkdir(const KUrl& url, int permissions);

protected:
    void createUDSEntry(const KArchiveEntry * tarEntry, KIO::UDSEntry & entry);
    bool checkNewFile(const KUrl & url, QString & path);

    KArchive * m_archiveFile;
    QString m_archiveName;
    QString user;
    QString group;
    time_t m_mtime;
};

#endif
