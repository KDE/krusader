/***************************************************************************
                                   krarc.h
                             -------------------
    begin                : Sat Jun 14 14:42:49 IDT 2003
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : yanai@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KRARC_H
#define KRARC_H

#include <sys/types.h>

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QFile>
#include <QtCore/QUrl>

#include <KIO/Global>
#include <KIO/SlaveBase>
#include <KCoreAddons/KProcess>
#include <KConfigCore/KConfigGroup>

class KFileItem;
class QByteArray;
class QTextCodec;

class kio_krarcProtocol : public QObject, public KIO::SlaveBase
{
    Q_OBJECT
public:
    kio_krarcProtocol(const QByteArray &pool_socket, const QByteArray &app_socket);
    virtual ~kio_krarcProtocol();
    virtual void stat(const QUrl &url) Q_DECL_OVERRIDE;
    virtual void get(const QUrl &url) Q_DECL_OVERRIDE;
    virtual void put(const QUrl &url, int permissions, KIO::JobFlags flags) Q_DECL_OVERRIDE;
    virtual void mkdir(const QUrl &url, int permissions) Q_DECL_OVERRIDE;
    virtual void listDir(const QUrl &url) Q_DECL_OVERRIDE;
    virtual void del(QUrl const & url, bool isFile) Q_DECL_OVERRIDE;
    virtual void copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) Q_DECL_OVERRIDE;

public slots:
    void receivedData(KProcess *, QByteArray &);
    void checkOutputForPassword(KProcess *, QByteArray &);

protected:
    virtual bool   initDirDict(const QUrl &url, bool forced = false);
    virtual bool   initArcParameters();
    QString detectArchive(bool &encrypted, QString fileName);
    virtual void parseLine(int lineNo, QString line);
    virtual bool setArcFile(const QUrl &url);
    virtual QString getPassword();
    virtual void invalidatePassword();
    QString getPath(const QUrl &url, QUrl::FormattingOptions options = 0);

    QString localeEncodedString(QString str);
    QByteArray encodeString(QString);
    QString decodeString(char *);

    // archive specific commands
    QString     cmd;     ///< the archiver name.
    QStringList listCmd; ///< list files.
    QStringList getCmd;  ///< unpack files command.
    QStringList delCmd;  ///< delete files command.
    QStringList putCmd;  ///< add file command.
    QStringList copyCmd; ///< copy to file command.

private:
    void get(const QUrl &url, int tries);
    /** checks if the exit code is OK. */
    bool checkStatus(int exitCode);
    /** service function for parseLine. */
    QString nextWord(QString &s, char d = ' ');
    /** translate permittion string to mode_t. */
    mode_t parsePermString(QString perm);
    /** return the name of the directory inside the archive. */
    QString findArcDirectory(const QUrl &url);
    /** find the UDSEntry of a file in a directory. */
    KIO::UDSEntry* findFileEntry(const QUrl &url);
    /** add a new directory (file list container). */
    KIO::UDSEntryList* addNewDir(QString path);
    QString fullPathName(QString name);
    static QString detectFullPathName(QString name);
    bool checkWriteSupport();

    QHash<QString, KIO::UDSEntryList *> dirDict; //< the directoris data structure.
    bool encrypted;                   //< tells whether the archive is encrypted
    bool archiveChanged;              //< true if the archive was changed.
    bool archiveChanging;             //< true if the archive is currently changing.
    bool newArchiveURL;               //< true if new archive was entered for the protocol
    bool noencoding;                   //< 7z files use UTF-16, so encoding is unnecessary
    KIO::filesize_t decompressedLen;  //< the number of the decompressed bytes
    KFileItem* arcFile;               //< the archive file item.
    QString arcPath;                  //< the archive location
    QString arcTempDir;               //< the currently used temp directory.
    QString arcType;                  //< the archive type.
    bool extArcReady;                 //< Used for RPM & DEB files.
    QString password;                 //< Password for the archives
    KConfig *krConfig;                //< The configuration file for krusader
    KConfigGroup confGrp;             //< the 'Dependencies' config group

    QString lastData;
    QString encryptedArchPath;

    QString currentCharset;
    QTextCodec * codec;
};

class KrLinecountingProcess : public KProcess
{
    Q_OBJECT
public:
    KrLinecountingProcess() : KProcess() {
        setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
        connect(this, SIGNAL(readyReadStandardError()), SLOT(receivedError()));
        connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()));
        mergedOutput = true;
    }

    void setMerge(bool value) {
        mergedOutput = value;
    }

    QString getErrorMsg() {
        if (errorData.trimmed().isEmpty())
            return QString::fromLocal8Bit(outputData);
        else
            return QString::fromLocal8Bit(errorData);
    }

public slots:
    void receivedError() {
        QByteArray newData(this->readAllStandardError());
        emit newErrorLines(newData.count('\n'));
        errorData += newData;
        if (errorData.length() > 500)
            errorData = errorData.right(500);
        if (mergedOutput)
            receivedOutput(newData);
    }

    void receivedOutput(QByteArray newData = QByteArray()) {
        if (newData.isEmpty())
            newData = this->readAllStandardOutput();
        emit newOutputLines(newData.count('\n'));
        emit newOutputData(this, newData);
        outputData += newData;
        if (outputData.length() > 500)
            outputData = outputData.right(500);
    }

signals:
    void newOutputLines(int count);
    void newErrorLines(int count);
    void newOutputData(KProcess *, QByteArray &);

private:
    QByteArray errorData;
    QByteArray outputData;

    bool mergedOutput;
};

#ifdef Q_WS_WIN
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
