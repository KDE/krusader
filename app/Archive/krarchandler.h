/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRARCHANDLER_H
#define KRARCHANDLER_H

// QtCore
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QUrl>

#include <KProcess>

#include "../../plugins/krarc/krarcbasemanager.h"

namespace KWallet
{
class Wallet;
}

class KrArcObserver : public QObject
{
    Q_OBJECT
public:
    ~KrArcObserver() override = default;

    virtual void processEvents() = 0;
    virtual void subJobStarted(const QString &jobTitle, qulonglong count) = 0;
    virtual void subJobStopped() = 0;
    virtual bool wasCancelled() = 0;
    virtual void error(const QString &error) = 0;
    virtual void detailedError(const QString &error, const QString &details) = 0;

public slots:
    virtual void incrementProgress(qsizetype) = 0;
};

class KrArcHandler : public QObject, public KrArcBaseManager
{
    Q_OBJECT
public:
    explicit KrArcHandler(QObject *parent = nullptr);

    // return the number of files in the archive
    qulonglong arcFileCount(const QString &archive, const QString &type, const QString &password, KrArcObserver *observer);
    // unpack an archive to destination directory
    bool unpack(QString archive, const QString &type, const QString &password, const QString &dest, KrArcObserver *observer);
    // pack an archive to destination directory
    bool pack(QStringList fileNames, QString type, const QString &dest, qulonglong count, QMap<QString, QString> extraProps, KrArcObserver *observer);
    // test an archive
    bool test(const QString &archive, const QString &type, const QString &password, KrArcObserver *observer, qulonglong count = 0L);
    // returns `true` if the right unpacker exist in the system
    static bool arcSupported(QString type);
    // return the list of supported packers
    static QStringList supportedPackers();
    // returns `true` if the url is an archive (ie: tar:/home/test/file.tar.bz2)
    static bool isArchive(const QUrl &url);
    // used to determine the type of the archive
    QString getType(bool &encrypted, QString fileName, const QString &mime, bool check7zEncrypted = true, bool fast = false);
    // queries the password from the user
    static QString getPassword(const QString &path);
    // detects the archive type
    void checkIf7zIsEncrypted(bool &, QString) override;
    // returns the registered protocol associated with the mimetype
    QString registeredProtocol(const QString &mimetype);
    // clear the cache of handled protocols
    static void clearProtocolCache();

private:
    //! checks if a returned status ("exit code") of an archiving-related process is OK
    static bool checkStatus(const QString &type, int exitCode);

    static bool openWallet();

    //! the list of archive mimetypes that are openable by the krarc protocol
    QSet<QString> krarcArchiveMimetypes;

    //! the cache of handled protocols
    static QMap<QString, QString> *slaveMap;

    static KWallet::Wallet *wallet;
};

#endif
