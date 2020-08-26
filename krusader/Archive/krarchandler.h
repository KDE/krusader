/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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
#ifndef KRARCHANDLER_H
#define KRARCHANDLER_H

// QtCore
#include <QStringList>
#include <QObject>
#include <QSet>
#include <QUrl>

#include <KCoreAddons/KProcess>

#include "../../krArc/krarcbasemanager.h"

namespace KWallet {
class Wallet;
}

class KrArcObserver : public QObject
{
    Q_OBJECT
public:
    ~KrArcObserver() override = default;

    virtual void processEvents() = 0;
    virtual void subJobStarted(const QString & jobTitle, qulonglong count) = 0;
    virtual void subJobStopped() = 0;
    virtual bool wasCancelled() = 0;
    virtual void error(const QString & error) = 0;
    virtual void detailedError(const QString & error, const QString & details) = 0;

public slots:
    virtual void incrementProgress(int) = 0;
};

class KrArcHandler: public QObject, public KrArcBaseManager
{
    Q_OBJECT
public:
    explicit KrArcHandler(QObject *parent = nullptr);

    // return the number of files in the archive
    qulonglong arcFileCount(const QString& archive, const QString& type, const QString& password, KrArcObserver *observer);
    // unpack an archive to destination directory
    bool unpack(QString archive, const QString& type, const QString& password, const QString& dest, KrArcObserver *observer );
    // pack an archive to destination directory
    bool pack(QStringList fileNames, QString type, const QString& dest, qulonglong count, QMap<QString, QString> extraProps, KrArcObserver *observer);
    // test an archive
    bool test(const QString& archive, const QString& type, const QString& password, KrArcObserver *observer, qulonglong count = 0L);
    // returns `true` if the right unpacker exist in the system
    static bool arcSupported(QString type);
    // return the list of supported packers
    static QStringList supportedPackers();
    // returns `true` if the url is an archive (ie: tar:/home/test/file.tar.bz2)
    static bool isArchive(const QUrl &url);
    // used to determine the type of the archive
    QString getType(bool &encrypted, QString fileName, const QString& mime, bool check7zEncrypted = true, bool fast = false);
    // queries the password from the user
    static QString getPassword(const QString& path);
    // detects the archive type
    void checkIf7zIsEncrypted(bool &, QString) override;
    // returns the registered protocol associated with the mimetype
    QString registeredProtocol(const QString& mimetype);
    // clear the cache of handled protocols
    static void clearProtocolCache();

private:
    //! checks if a returned status ("exit code") of an archiving-related process is OK
    static bool checkStatus(const QString& type, int exitCode);

    static bool openWallet();

    //! the list of archive mimetypes that are openable by the krarc protocol
    QSet<QString> krarcArchiveMimetypes;

    //! the cache of handled protocols
    static QMap<QString, QString>* slaveMap;

    static KWallet::Wallet * wallet;
};

#endif
