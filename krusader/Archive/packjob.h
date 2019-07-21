/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2009-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef PACKJOB_H
#define PACKJOB_H

// QtCore
#include <QMap>

#include "abstractthreadedjob.h"

class PackThread;
class TestArchiveThread;
class UnpackThread;

class PackJob : public AbstractThreadedJob
{
    Q_OBJECT

private:
    PackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps);

public:
    static PackJob * createPacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps);
};


class PackThread : public AbstractJobThread
{
    Q_OBJECT

public:
    PackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps);
    ~PackThread() override = default;

protected slots:
    void slotStart() Q_DECL_OVERRIDE;

private:
    QUrl                   _sourceUrl;
    QUrl                   _destUrl;
    QStringList            _fileNames;
    QString                _type;
    QMap<QString, QString> _packProperties;
};


class TestArchiveJob : public AbstractThreadedJob
{
    Q_OBJECT

private:
    TestArchiveJob(const QUrl &srcUrl, const QStringList & fileNames);

public:
    static TestArchiveJob * testArchives(const QUrl &srcUrl, const QStringList & fileNames);
};


class TestArchiveThread : public AbstractJobThread
{
    Q_OBJECT

public:
    TestArchiveThread(const QUrl &srcUrl, const QStringList & fileNames);
    ~TestArchiveThread() override = default;

protected slots:
    void slotStart() Q_DECL_OVERRIDE;

private:
    QUrl                   _sourceUrl;
    QStringList            _fileNames;
};



class UnpackJob : public AbstractThreadedJob
{
    Q_OBJECT

private:
    UnpackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames);

public:
    static UnpackJob * createUnpacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames);
};


class UnpackThread : public AbstractJobThread
{
    Q_OBJECT

public:
    UnpackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames);
    ~UnpackThread() override = default;

protected slots:
    void slotStart() Q_DECL_OVERRIDE;

private:
    QUrl                   _sourceUrl;
    QUrl                   _destUrl;
    QStringList            _fileNames;
};

#endif // __PACK_JOB_H__

