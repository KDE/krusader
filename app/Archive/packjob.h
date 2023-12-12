/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    PackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames, const QString &type, const QMap<QString, QString> &packProps);

public:
    static PackJob *
    createPacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames, const QString &type, const QMap<QString, QString> &packProps);
};

class PackThread : public AbstractJobThread
{
    Q_OBJECT

public:
    PackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames, const QString &type, const QMap<QString, QString> &packProps);
    ~PackThread() override = default;

protected slots:
    void slotStart() override;

private:
    QUrl _sourceUrl;
    QUrl _destUrl;
    QStringList _fileNames;
    QString _type;
    QMap<QString, QString> _packProperties;
};

class TestArchiveJob : public AbstractThreadedJob
{
    Q_OBJECT

private:
    TestArchiveJob(const QUrl &srcUrl, const QStringList &fileNames);

public:
    static TestArchiveJob *testArchives(const QUrl &srcUrl, const QStringList &fileNames);
};

class TestArchiveThread : public AbstractJobThread
{
    Q_OBJECT

public:
    TestArchiveThread(const QUrl &srcUrl, const QStringList &fileNames);
    ~TestArchiveThread() override = default;

protected slots:
    void slotStart() override;

private:
    QUrl _sourceUrl;
    QStringList _fileNames;
};

class UnpackJob : public AbstractThreadedJob
{
    Q_OBJECT

private:
    UnpackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames);

public:
    static UnpackJob *createUnpacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames);
};

class UnpackThread : public AbstractJobThread
{
    Q_OBJECT

public:
    UnpackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList &fileNames);
    ~UnpackThread() override = default;

protected slots:
    void slotStart() override;

private:
    QUrl _sourceUrl;
    QUrl _destUrl;
    QStringList _fileNames;
};

#endif // __PACK_JOB_H__
