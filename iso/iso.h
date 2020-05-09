/*****************************************************************************
 * Copyright (C) 2000 David Faure <faure@kde.org>                            *
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
 * Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>                       *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is heavily based on ktar from kdelibs                           *
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

#ifndef ISO_H
#define ISO_H

// QtCore
#include <QByteArray>
#include <QUrl>

#include <KIO/SlaveBase>

#include "kisofile.h"

class KIso;

class kio_isoProtocol : public KIO::SlaveBase
{
public:
    kio_isoProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~kio_isoProtocol();

    virtual void listDir(const QUrl &url) override;
    virtual void stat(const QUrl &url) override;
    virtual void get(const QUrl &url) override;

protected:
    void getFile(const KIsoFile *isoFileEntry, const QString &path);
    void createUDSEntry(const KArchiveEntry * isoEntry, KIO::UDSEntry & entry);
    bool checkNewFile(QString fullPath, QString & path, int startsec);
    QString getPath(const QUrl &url);

    KIso * m_isoFile;
    time_t m_mtime;
    int m_mode;
};

#endif
