/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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
#include <QUrl>

#include <KCoreAddons/KProcess>

#include "../../krArc/krarcbasemanager.h"

namespace KWallet {
class Wallet;
}

class KRarcObserver : public QObject
{
    Q_OBJECT
public:
    ~KRarcObserver() override = default;

    virtual void processEvents() = 0;
    virtual void subJobStarted(const QString & jobTitle, int count) = 0;
    virtual void subJobStopped() = 0;
    virtual bool wasCancelled() = 0;
    virtual void error(const QString & error) = 0;
    virtual void detailedError(const QString & error, const QString & details) = 0;

public slots:
    virtual void incrementProgress(int) = 0;
};

class KRarcHandler: public QObject, public KrArcBaseManager
{
    Q_OBJECT
public:
    // return the number of files in the archive
    static long arcFileCount(const QString& archive, const QString& type, const QString& password, KRarcObserver *observer);
    // unpack an archive to destination directory
    static bool unpack(QString archive, const QString& type, const QString& password, const QString& dest, KRarcObserver *observer );
    // pack an archive to destination directory
    static bool pack(QStringList fileNames, QString type, const QString& dest, long count, QMap<QString, QString> extraProps, KRarcObserver *observer );
    // test an archive
    static bool test(const QString& archive, const QString& type, const QString& password, KRarcObserver *observer, long count = 0L );
    // returns `true` if the right unpacker exist in the system
    static bool arcSupported(QString type);
    // return the list of supported packers
    static QStringList supportedPackers();
    // returns `true` if the url is an archive (ie: tar:/home/test/file.tar.bz2)
    static bool isArchive(const QUrl &url);
    // used to determine the type of the archive
    QString getType(bool &encrypted, QString fileName, const QString& mime, bool checkEncrypted = true, bool fast = false);
    // queries the password from the user
    static QString getPassword(const QString& path);
    // detects the archive type
    void checkIf7zIsEncrypted(bool &, QString) Q_DECL_OVERRIDE;
private:
    //! checks if a returned status ("exit code") of an archiving-related process is OK
    static bool checkStatus(const QString& type, int exitCode);

    static bool openWallet();

    static KWallet::Wallet * wallet;
};

#endif
