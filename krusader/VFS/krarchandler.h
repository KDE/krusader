/***************************************************************************
                                 krarchandler.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description: this class will supply static archive handling functions.
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KRARCHANDLER_H
#define KRARCHANDLER_H

#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <KCoreAddons/KProcess>

#include "../../krArc/krarcbasemanager.h"
#include "../../krArc/krlinecountingprocess.h"
#include "kr7zencryptionchecker.h"

namespace KWallet {
class Wallet;
}

class KRarcObserver : public QObject
{
    Q_OBJECT
public:
    virtual ~KRarcObserver() {}

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
    static long arcFileCount(QString archive, QString type, QString password, KRarcObserver *observer);
    // unpack an archive to destination directory
    static bool unpack(QString archive, QString type, QString password, QString dest, KRarcObserver *observer );
    // pack an archive to destination directory
    static bool pack(QStringList fileNames, QString type, QString dest, long count, QMap<QString, QString> extraProps, KRarcObserver *observer );
    // test an archive
    static bool test(QString archive, QString type, QString password, KRarcObserver *observer, long count = 0L );
    // returns `true` if the right unpacker exist in the system
    static bool arcSupported(QString type);
    // return the list of supported packers
    static QStringList supportedPackers();
    // returns `true` if the url is an archive (ie: tar:/home/test/file.tar.bz2)
    static bool isArchive(const QUrl &url);
    // used to determine the type of the archive
    QString getType(bool &encrypted, QString fileName, QString mime, bool checkEncrypted = true, bool fast = false);
    // queries the password from the user
    static QString getPassword(QString path);
    // detects the archive type
    void checkIf7zIsEncrypted(bool &, QString);
private:
    // checks if the returned status is correct
    static bool checkStatus(QString type, int exitCode);
    static bool openWallet();

    static KWallet::Wallet * wallet;
};

#endif
