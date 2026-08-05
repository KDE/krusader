/*
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sf.net>
    SPDX-FileCopyrightText: 2003 Shie Erlich <yanai@users.sf.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRARC_H
#define KRARC_H

// QtCore
#include <QFile>
#include <QHash>
#include <QString>
#include <QUrl>

#include <KIO/Global>
#include <KIO/WorkerBase>

#include <KProcess>

#include "../../app/krdebuglogger.h"
#include "krarcbasemanager.h"
#include "krlinecountingprocess.h"

class KFileItem;
class QByteArray;
class QTextCodec;

class kio_krarcProtocol : public QObject, public KIO::WorkerBase, public KrArcBaseManager
{
    Q_OBJECT
public:
    kio_krarcProtocol(const QByteArray &pool_socket, const QByteArray &app_socket);
    ~kio_krarcProtocol() override;
    KIO::WorkerResult stat(const QUrl &url) override;
    KIO::WorkerResult get(const QUrl &url) override;
    KIO::WorkerResult put(const QUrl &url, int permissions, KIO::JobFlags flags) override;
    KIO::WorkerResult mkdir(const QUrl &url, int permissions) override;
    KIO::WorkerResult listDir(const QUrl &url) override;
    KIO::WorkerResult del(QUrl const &url, bool isFile) override;
    KIO::WorkerResult copy(const QUrl &src, const QUrl &dest, int permissions, KIO::JobFlags flags) override;
    KIO::WorkerResult rename(const QUrl &src, const QUrl &dest, KIO::JobFlags flags) override;
public slots:
    void receivedData(KProcess *, QByteArray &);
    void check7zOutputForPassword(KProcess *, QByteArray &);

protected:
    Q_REQUIRED_RESULT virtual bool initDirDict(const QUrl &url, bool forced = false);
    Q_REQUIRED_RESULT virtual KIO::WorkerResult initArcParameters();
    void checkIf7zIsEncrypted(bool &, QString) override;
    Q_REQUIRED_RESULT virtual KIO::WorkerResult setArcFile(const QUrl &url);
    Q_REQUIRED_RESULT virtual QString getPassword();
    virtual void invalidatePassword();
    QString getPath(const QUrl &url, QUrl::FormattingOptions options = QUrl::None);
    /** parses a text line from the listing of an archive. */
    virtual void parseLine(int lineNo, QString line);

    QByteArray encodeString(const QString &);
    QString decodeString(char *);

    // archive specific commands
    QString cmd; ///< the archiver name.
    QStringList listCmd; ///< list files.
    QStringList getCmd; ///< unpack files command.
    QStringList delCmd; ///< delete files command.
    QStringList putCmd; ///< add file command.
    QStringList copyCmd; ///< copy to file command.
    QStringList renCmd; ///< rename file command.

private:
    KIO::WorkerResult get(const QUrl &url, int tries);
    /** checks if a returned status ("exit code") of an archiving-related process is OK. */
    bool checkStatus(int exitCode);
    /** service function for parseLine. */
    QString nextWord(QString &s, char d = ' ');
    /** translate permission string to mode_t. */
    mode_t parsePermString(QString perm);
    /** return the name of the directory inside the archive. */
    QString findArcDirectory(const QUrl &url);
    /** find the UDSEntry of a file in a directory. */
    KIO::UDSEntry *findFileEntry(const QUrl &url);
    /** add a new directory (file list container). */
    KIO::UDSEntryList *addNewDir(const QString &path);
    Q_REQUIRED_RESULT KIO::WorkerResult checkWriteSupport();

    QHash<QString, KIO::UDSEntryList *> m_dirDict; //< the directories data structure.
    bool m_encrypted; //< tells whether the archive is encrypted
    bool m_archiveChanged; //< true if the archive was changed.
    bool m_archiveChanging; //< true if the archive is currently changing.
    bool m_newArchiveURL; //< true if new archive was entered for the protocol
    bool m_noencoding; //< 7z files use UTF-16, so encoding is unnecessary
    KIO::filesize_t m_decompressedLen; //< the number of the decompressed bytes
    KFileItem *m_arcFile; //< the archive file item.
    QString m_arcPath; //< the archive location
    QString m_arcTempDir; //< the currently used temp directory.
    QString m_arcType; //< the archive type.
    bool m_extArcReady; //< Used for RPM & DEB files.
    QString m_password; //< Password for the archives

    QString m_lastData;
    QString m_encryptedArchPath;

    QString m_currentCharset;
    QTextCodec *m_codec;
};

#ifdef Q_OS_WIN
#define DIR_SEPARATOR "/"
#define DIR_SEPARATOR2 "\\"
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_CHAR2 '\\'
#define REPLACE_DIR_SEP2(x) x = x.replace(DIR_SEPARATOR2, DIR_SEPARATOR);
#define ROOT_DIR "C:\\"
#define EXEC_SUFFIX ".exe"
#else
#define DIR_SEPARATOR "/"
#define DIR_SEPARATOR2 "/"
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_CHAR2 '/'
#define REPLACE_DIR_SEP2(x)
#define ROOT_DIR "/"
#define EXEC_SUFFIX ""
#endif

#endif
