/***************************************************************************
                         packjob.cpp  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "packjob.h"
#include "krarchandler.h"

// QtCore
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTimer>

#include <KI18n/KLocalizedString>

extern KRarcHandler arcHandler;

PackJob::PackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps) : AbstractThreadedJob()
{
    startAbstractJobThread(new PackThread(srcUrl, destUrl, fileNames, type, packProps));
}

PackJob * PackJob::createPacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames, const QString &type, const QMap<QString, QString> &packProps)
{
    return new PackJob(srcUrl, destUrl, fileNames, type, packProps);
}

PackThread::PackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames,
                       const QString &type, const QMap<QString, QString> &packProps) :
        AbstractJobThread(), _sourceUrl(srcUrl), _destUrl(destUrl), _fileNames(fileNames),
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
    bool result = KRarcHandler::pack(_fileNames, _type, arcFile, totalFiles, _packProperties, observer());
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

TestArchiveJob::TestArchiveJob(const QUrl &srcUrl, const QStringList & fileNames) : AbstractThreadedJob()
{
    startAbstractJobThread(new TestArchiveThread(srcUrl, fileNames));
}

TestArchiveJob * TestArchiveJob::testArchives(const QUrl &srcUrl, const QStringList & fileNames)
{
    return new TestArchiveJob(srcUrl, fileNames);
}

TestArchiveThread::TestArchiveThread(const QUrl &srcUrl, const QStringList & fileNames) : AbstractJobThread(),
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
        if (!KRarcHandler::test(path, type, password, observer(), 0)) {
            sendError(KIO::ERR_NO_CONTENT, i18nc("%1=archive filename", "%1, test failed.", arcName));
            return;
        }
    }

    sendMessage(i18n("Archive tests passed."));
    sendSuccess();
}


UnpackJob::UnpackJob(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames) : AbstractThreadedJob()
{
    startAbstractJobThread(new UnpackThread(srcUrl, destUrl, fileNames));
}

UnpackJob * UnpackJob::createUnpacker(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames)
{
    return new UnpackJob(srcUrl, destUrl, fileNames);
}

UnpackThread::UnpackThread(const QUrl &srcUrl, const QUrl &destUrl, const QStringList & fileNames) :
        AbstractJobThread(), _sourceUrl(srcUrl), _destUrl(destUrl), _fileNames(fileNames)
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
        bool result = KRarcHandler::unpack(path, type, password, localDest, observer());

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
