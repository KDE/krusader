/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    This file is heavily based on ktar from kdelibs

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISO_H
#define ISO_H

// QtCore
#include <QByteArray>
#include <QUrl>

#include <KIO/WorkerBase>

#include "kisofile.h"

class KIso;

class kio_isoProtocol : public KIO::WorkerBase
{
public:
    kio_isoProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~kio_isoProtocol();

    virtual KIO::WorkerResult listDir(const QUrl &url) override;
    virtual KIO::WorkerResult stat(const QUrl &url) override;
    virtual KIO::WorkerResult get(const QUrl &url) override;

protected:
    KIO::WorkerResult getFile(const KIsoFile *isoFileEntry, const QString &path);
    void createUDSEntry(const KArchiveEntry *isoEntry, KIO::UDSEntry &entry);
    bool checkNewFile(QString fullPath, QString &path, int startsec);
    QString getPath(const QUrl &url);

    KIso *m_isoFile;
    time_t m_mtime;
    int m_mode;
};

#endif
