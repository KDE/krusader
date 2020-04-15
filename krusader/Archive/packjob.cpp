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

#include "packjob.h"
#include "krarchandler.h"
#include "../krglobal.h"

// QtCore
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTimer>

#include <KI18n/KLocalizedString>

PackJob::PackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps)
{
    startAbstractJobThread(new PackThread(srcUrl, destUrl, fileNames, type, packProps));
}

PackJob * PackJob::createPacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps)
{
    return new PackJob(srcUrl, destUrl, fileNames, type, packProps);
}

PackThread::PackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames,
                       const QString &type, const QMap<QString, QString> &packProps) :
        _sourceUrl(srcUrl), _destUrl(destUrl), _fileNames(fileNames),
        _type(type), _packProperties(packProps)
{
}


void PackThread::slotStart()
{
    QUrl newSource = downloadIfRemote(_sourceUrl, _fileNames);
    if (newSource.isEmpty())
        return;

    unsigned long totalFiles = 0;

    countLocalFiles(newSource, _fileNames, totalFiles);

    QString arcFile = tempFileIfRemote(_destUrl, _type);
    QString arcDir = newSource.adjusted(QUrl::StripTrailingSlash).path();

    setProgressTitle(i18n("Processed files"));

    QString save = QDir::currentPath();
    QDir::setCurrent(arcDir);
    bool result = krArcMan.pack(_fileNames, _type, arcFile, totalFiles, _packProperties, observer());
    QDir::setCurrent(save);

    if (isExited())
        return;
    if (!result) {
        sendError(KIO::ERR_INTERNAL, i18n("Error while packing"));
        return;
    }

    if (!uploadTempFiles())
        return;

    sendSuccess();
}

TestArchiveJob::TestArchiveJob(const QUrl &srcUrl, const QStringList & fileNames)
{
    startAbstractJobThread(new TestArchiveThread(srcUrl, fileNames));
}

TestArchiveJob * TestArchiveJob::testArchives(const QUrl &srcUrl, const QStringList & fileNames)
{
    return new TestArchiveJob(srcUrl, fileNames);
}

TestArchiveThread::TestArchiveThread(const QUrl &srcUrl, const QStringList & fileNames) :
        _sourceUrl(srcUrl), _fileNames(fileNames)
{
}

void TestArchiveThread::slotStart()
{
    // Gets a QUrl of the source folder, which may be remote
    QUrl newSource = downloadIfRemote(_sourceUrl, _fileNames);
    if (newSource.isEmpty())
        return;

    for (int i = 0; i < _fileNames.count(); ++i) {
        QString path, type, password, arcName = _fileNames[i];
        if (!getArchiveInformation(path, type, password, arcName, newSource))
            return;

        // test the archive
        if (!krArcMan.test(path, type, password, observer(), 0)) {
            sendError(KIO::ERR_NO_CONTENT, i18nc("%1=archive filename", "%1, test failed.", arcName));
            return;
        }
    }

    sendMessage(i18n("Archive tests passed."));
    sendSuccess();
}


UnpackJob::UnpackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames)
{
    startAbstractJobThread(new UnpackThread(srcUrl, destUrl, fileNames));
}

UnpackJob * UnpackJob::createUnpacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames)
{
    return new UnpackJob(srcUrl, destUrl, fileNames);
}

UnpackThread::UnpackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames) :
        _sourceUrl(srcUrl), _destUrl(destUrl), _fileNames(fileNames)
{
}

void UnpackThread::slotStart()
{
    // Gets a QUrl of the source folder, which may be remote
    QUrl newSource = downloadIfRemote(_sourceUrl, _fileNames);
    if (newSource.isEmpty())
        return;

    QString localDest = tempDirIfRemote(_destUrl);

    for (int i = 0; i < _fileNames.count(); ++i) {
        QString path, type, password, arcName = _fileNames[i];
        if (!getArchiveInformation(path, type, password, arcName, newSource))
            return;

        setProgressTitle(i18n("Processed files"));
        // unpack the files
        bool result = krArcMan.unpack(path, type, password, localDest, observer());

        if (isExited())
            return;
        if (!result) {
            sendError(KIO::ERR_INTERNAL, i18n("Error while unpacking"));
            return;
        }
    }

    if (!uploadTempFiles())
        return;

    sendSuccess();
}
