/*
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sf.net>
    SPDX-FileCopyrightText: 2003 Shie Erlich <yanai@users.sf.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRARC_H
#define KRARC_H

// QtCore
#include <QString>
#include <QHash>
#include <QFile>
#include <QUrl>

#include <KIO/Global>
#include <KIO/SlaveBase>
#include <KCoreAddons/KProcess>

#include "krarcbasemanager.h"
#include "krlinecountingprocess.h"
#include "../krusader/krdebuglogger.h"

class KFileItem;
class QByteArray;
class QTextCodec;

class kio_krarcProtocol : public QObject, public KIO::SlaveBase, public KrArcBaseManager
{
    Q_OBJECT
public:
    kio_krarcProtocol(const QByteArray &pool_socket, const QByteArray &app_socket);
    ~kio_krarcProtocol() override;
    void stat(const QUrl &url) override;
    void get(const QUrl &url) override;
    void put(const QUrl &url, int permissions, KIO::JobFlags flags) override;
    void mkdir(const QUrl &url, int permissions) override;
    void listDir(const QUrl &url) override;
    void del(QUrl const & url, bool isFile) override;
    void copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    void rename(const QUrl &src, const QUrl & dest, KIO::JobFlags flags) override;

public slots:
    void receivedData(KProcess *, QByteArray &);
    void check7zOutputForPassword(KProcess *, QByteArray &);

protected:
    virtual bool   initDirDict(const QUrl &url, bool forced = false);
    virtual bool   initArcParameters();
    void checkIf7zIsEncrypted(bool &, QString) override;
    virtual bool setArcFile(const QUrl &url);
    virtual QString getPassword();
    virtual void invalidatePassword();
    QString getPath(const QUrl &url, QUrl::FormattingOptions options = nullptr);
    /** parses a text line from the listing of an archive. */
    virtual void parseLine(int lineNo, QString line);

    QString localeEncodedString(QString str);
    QByteArray encodeString(const QString&);
    QString decodeString(char *);

    // archive specific commands
    QString     cmd;     ///< the archiver name.
    QStringList listCmd; ///< list files.
    QStringList getCmd;  ///< unpack files command.
    QStringList delCmd;  ///< delete files command.
    QStringList putCmd;  ///< add file command.
    QStringList copyCmd; ///< copy to file command.
    QStringList renCmd;  ///< rename file command.

private:
    void get(const QUrl &url, int tries);
    /** checks if a returned status ("exit code") of an archiving-related process is OK. */
    bool checkStatus(int exitCode);
    /** service function for parseLine. */
    QString nextWord(QString &s, char d = ' ');
    /** translate permission string to mode_t. */
    mode_t parsePermString(QString perm);
    /** return the name of the directory inside the archive. */
    QString findArcDirectory(const QUrl &url);
    /** find the UDSEntry of a file in a directory. */
    KIO::UDSEntry* findFileEntry(const QUrl &url);
    /** add a new directory (file list container). */
    KIO::UDSEntryList* addNewDir(const QString& path);
    bool checkWriteSupport();

    QHash<QString, KIO::UDSEntryList *> dirDict; //< the directories data structure.
    bool encrypted;                   //< tells whether the archive is encrypted
    bool archiveChanged;              //< true if the archive was changed.
    bool archiveChanging;             //< true if the archive is currently changing.
    bool newArchiveURL;               //< true if new archive was entered for the protocol
    bool noencoding;                  //< 7z files use UTF-16, so encoding is unnecessary
    KIO::filesize_t decompressedLen;  //< the number of the decompressed bytes
    KFileItem* arcFile;               //< the archive file item.
    QString arcPath;                  //< the archive location
    QString arcTempDir;               //< the currently used temp directory.
    QString arcType;                  //< the archive type.
    bool extArcReady;                 //< Used for RPM & DEB files.
    QString password;                 //< Password for the archives

    QString lastData;
    QString encryptedArchPath;

    QString currentCharset;
    QTextCodec * codec;
};

#ifdef Q_OS_WIN
#define DIR_SEPARATOR       "/"
#define DIR_SEPARATOR2      "\\"
#define DIR_SEPARATOR_CHAR  '/'
#define DIR_SEPARATOR_CHAR2 '\\'
#define REPLACE_DIR_SEP2(x) x = x.replace( DIR_SEPARATOR2, DIR_SEPARATOR );
#define ROOT_DIR            "C:\\"
#define EXEC_SUFFIX         ".exe"
#else
#define DIR_SEPARATOR       "/"
#define DIR_SEPARATOR2      "/"
#define DIR_SEPARATOR_CHAR  '/'
#define DIR_SEPARATOR_CHAR2 '/'
#define REPLACE_DIR_SEP2(x)
#define ROOT_DIR            "/"
#define EXEC_SUFFIX         ""
#endif

#endif
