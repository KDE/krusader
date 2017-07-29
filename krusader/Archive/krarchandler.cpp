/***************************************************************************
                                krarchandler.cpp
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
   web site   : http://krusader.sourceforge.net
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

#include "krarchandler.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QFile>
// QtWidgets
#include <QApplication>

#include <KArchive/KTar>
#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KIO/Global>
#include <KWallet/KWallet>
#include <KWidgetsAddons/KMessageBox>
#include <KWidgetsAddons/KPasswordDialog>

#include "kr7zencryptionchecker.h"
#include "../krglobal.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../Dialogs/krpleasewait.h"
#include "../../krArc/krlinecountingprocess.h"

#if 0
class DefaultKRarcObserver : public KRarcObserver
{
public:
    DefaultKRarcObserver() {}
    virtual ~DefaultKRarcObserver() {}

    virtual void processEvents() Q_DECL_OVERRIDE {
        usleep(1000);
        qApp->processEvents();
    }

    virtual void subJobStarted(const QString & jobTitle, int count) Q_DECL_OVERRIDE {
        krApp->startWaiting(jobTitle, count, true);
    }

    virtual void subJobStopped() Q_DECL_OVERRIDE {
        krApp->stopWait();
    }

    virtual bool wasCancelled() Q_DECL_OVERRIDE {
        return krApp->wasWaitingCancelled();
    }

    virtual void error(const QString & error) Q_DECL_OVERRIDE {
        KMessageBox::error(krApp, error, i18n("Error"));
    }

    virtual void detailedError(const QString & error, const QString & details) Q_DECL_OVERRIDE {
        KMessageBox::detailedError(krApp, error, details, i18n("Error"));
    }

    virtual void incrementProgress(int c) Q_DECL_OVERRIDE {
        krApp->plzWait->incProgress(c);
    }
};
#endif

static QStringList arcProtocols = QString("tar;bzip;bzip2;lzma;xz;gzip;krarc;zip").split(';');

KWallet::Wallet * KRarcHandler::wallet = 0;

QStringList KRarcHandler::supportedPackers()
{
    QStringList packers;

    // we will simply try to find the packers here..
    if (KrServices::cmdExist("tar")) packers.append("tar");
    if (KrServices::cmdExist("gzip")) packers.append("gzip");
    if (KrServices::cmdExist("bzip2")) packers.append("bzip2");
    if (KrServices::cmdExist("lzma")) packers.append("lzma");
    if (KrServices::cmdExist("xz")) packers.append("xz");
    if (KrServices::cmdExist("unzip")) packers.append("unzip");
    if (KrServices::cmdExist("zip")) packers.append("zip");
    if (KrServices::cmdExist("zip")) packers.append("cbz");
    if (KrServices::cmdExist("lha")) packers.append("lha");
    if (KrServices::cmdExist("cpio")) packers.append("cpio");
    if (KrServices::cmdExist("unrar")) packers.append("unrar");
    if (KrServices::cmdExist("rar")) packers.append("rar");
    if (KrServices::cmdExist("rar")) packers.append("cbr");
    if (KrServices::cmdExist("arj")) packers.append("arj");
    if (KrServices::cmdExist("unarj")) packers.append("unarj");
    if (KrServices::cmdExist("unace")) packers.append("unace");
    if (KrServices::cmdExist("dpkg")) packers.append("dpkg");
    if (KrServices::cmdExist("7z") || KrServices::cmdExist("7za")) packers.append("7z");
    if (KrServices::cmdExist("rpm") && KrServices::cmdExist("rpm2cpio")) packers.append("rpm");
    // qDebug() << "Supported Packers:";
    //QStringList::Iterator it;
    //for( it = packers.begin(); it != packers.end(); ++it )
    // qDebug() << *it;

    return packers;
}

bool KRarcHandler::arcSupported(QString type)
{
    // lst will contain the supported unpacker list...
    const KConfigGroup group(krConfig, "Archives");
    const QStringList lst = group.readEntry("Supported Packers", QStringList());

    // Let's notice that in some cases the QString `type` that arrives here
    // represents a mimetype, and in some other cases it represents
    // a short identifier.
    // If `type` is not a short identifier then it's supposed that `type` is a mime type
    if (type.length() > maxLenType) {
        type = getShortTypeFromMime(type);
    }

    return (type == "zip" && lst.contains("unzip"))
           || (type == "tar" && lst.contains("tar"))
           || (type == "tbz" && lst.contains("tar"))
           || (type == "tgz" && lst.contains("tar"))
           || (type == "tlz" && lst.contains("tar"))
           || (type == "txz" && lst.contains("tar"))
           || (type == "tarz" && lst.contains("tar"))
           || (type == "gzip" && lst.contains("gzip"))
           || (type == "bzip2" && lst.contains("bzip2"))
           || (type == "lzma" && lst.contains("lzma"))
           || (type == "xz" && lst.contains("xz"))
           || (type == "lha" && lst.contains("lha"))
           || (type == "ace" && lst.contains("unace"))
           || (type == "rpm" && lst.contains("cpio"))
           || (type == "cpio" && lst.contains("cpio"))
           || (type == "rar" && (lst.contains("unrar") || lst.contains("rar")))
           || (type == "arj" && (lst.contains("unarj") || lst.contains("arj")))
           || (type == "deb" && (lst.contains("dpkg") && lst.contains("tar")))
           || (type == "7z" && lst.contains("7z"));
}

long KRarcHandler::arcFileCount(QString archive, QString type, QString password, KRarcObserver *observer)
{
    int divideWith = 1;

    // first check if supported
    if (!arcSupported(type))
        return 0;

    // bzip2, gzip, etc. archives contain only one file
    if (type == "bzip2" || type == "gzip" || type == "lzma" || type == "xz")
        return 1L;

    // set the right lister to do the job
    QStringList lister;

    if (type == "zip") lister << KrServices::fullPathName("unzip") << "-ZTs";
    else if (type == "tar") lister << KrServices::fullPathName("tar") << "-tvf";
    else if (type == "tgz") lister << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tarz") lister << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tbz") lister << KrServices::fullPathName("tar") << "-tjvf";
    else if (type == "tlz") lister << KrServices::fullPathName("tar") << "--lzma" << "-tvf";
    else if (type == "txz") lister << KrServices::fullPathName("tar") << "--xz" << "-tvf";
    else if (type == "lha") lister << KrServices::fullPathName("lha") << "l";
    else if (type == "rar") lister << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "l" << "-v";
    else if (type == "ace") lister << KrServices::fullPathName("unace") << "l";
    else if (type == "arj") {
        if (KrServices::cmdExist("arj"))
            lister << KrServices::fullPathName("arj") << "v" << "-y" << "-v",
            divideWith = 4;
        else
            lister << KrServices::fullPathName("unarj") << "l";
    } else if (type == "rpm") lister << KrServices::fullPathName("rpm") << "--dump" << "-lpq";
    else if (type == "deb") lister << KrServices::fullPathName("dpkg") << "-c";
    else if (type == "7z")  lister << KrServices::fullPathName("7z") << "-y" << "l";
    else return 0L;

    if (!password.isNull()) {
        if (type == "arj")
            lister << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            lister << QString("-p%1").arg(password);
    }

    // tell the user to wait
    observer->subJobStarted(i18n("Counting files in archive"), 0);

    // count the number of files in the archive
    long count = 1;
    KProcess list;
    list << lister << archive;
    if (type == "ace" && QFile("/dev/ptmx").exists())     // Don't remove, unace crashes if missing!!!
        list.setStandardInputFile("/dev/ptmx");
    list.setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect
    list.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (list.waitForStarted()) while (list.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                list.kill();
        }
    ; // busy wait - need to find something better...

    observer->subJobStopped();

    if (list.exitStatus() != QProcess::NormalExit || !checkStatus(type, list.exitCode())) {
        observer->detailedError(i18n("Failed to list the content of the archive (%1).", archive),
                                QString::fromLocal8Bit(list.readAllStandardError()));
        return 0;
    }

    count = list.readAllStandardOutput().count('\n');

    //make sure you call stopWait after this function return...
    //  observer->subJobStopped();

    return count / divideWith;
}

bool KRarcHandler::unpack(QString archive, QString type, QString password, QString dest, KRarcObserver *observer)
{
    KConfigGroup group(krConfig, "Archives");
    if (group.readEntry("Test Before Unpack", _TestBeforeUnpack)) {
        // test first - or be sorry later...
        if (type != "rpm" && type != "deb" && !test(archive, type, password, observer, 0)) {
            observer->error(i18n("Failed to unpack %1.", archive));
            return false;
        }
    }

    // count the files in the archive
    long count = arcFileCount(archive, type, password, observer);
    if (count == 0)
        return false;   // not supported
    if (count == 1)
        count = 0;

    // choose the right packer for the job
    QString cpioName;
    QStringList packer;

    // set the right packer to do the job
    if (type == "zip") packer << KrServices::fullPathName("unzip") << "-o";
    else if (type == "tar") packer << KrServices::fullPathName("tar") << "-xvf";
    else if (type == "tgz") packer << KrServices::fullPathName("tar") << "-xvzf";
    else if (type == "tarz") packer << KrServices::fullPathName("tar") << "-xvzf";
    else if (type == "tbz") packer << KrServices::fullPathName("tar") << "-xjvf";
    else if (type == "tlz") packer << KrServices::fullPathName("tar") << "--lzma" << "-xvf";
    else if (type == "txz") packer << KrServices::fullPathName("tar") << "--xz" << "-xvf";
    else if (type == "gzip") packer << KrServices::fullPathName("gzip") << "-cd";
    else if (type == "bzip2") packer << KrServices::fullPathName("bzip2") << "-cdk";
    else if (type == "lzma") packer << KrServices::fullPathName("lzma") << "-cdk";
    else if (type == "xz")  packer << KrServices::fullPathName("xz") << "-cdk";
    else if (type == "lha") packer << KrServices::fullPathName("lha") << "xf";
    else if (type == "rar") packer << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "-y" << "x";
    else if (type == "ace") packer << KrServices::fullPathName("unace") << "x";
    else if (type == "arj") {
        if (KrServices::cmdExist("arj"))
            packer << KrServices::fullPathName("arj") << "-y" << "-v" << "x";
        else
            packer << KrServices::fullPathName("unarj") << "x";
    } else if (type == "7z")  packer << KrServices::fullPathName("7z") << "-y" << "x";
    else if (type == "rpm") {
        // TODO use QTemporaryFile (setAutoRemove(false) when asynchrone)
        cpioName = QDir::tempPath() + QStringLiteral("/contents.cpio");

        KrLinecountingProcess cpio;
        cpio << KrServices::fullPathName("rpm2cpio") << archive;
        cpio.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
        cpio.start();
        if (!cpio.waitForFinished() || cpio.exitStatus() != QProcess::NormalExit || !checkStatus("cpio", cpio.exitCode())) {
            observer->detailedError(i18n("Failed to convert rpm (%1) to cpio.", archive), cpio.getErrorMsg());
            return 0;
        }

        archive = cpioName;
        packer << KrServices::fullPathName("cpio") << "--force-local" << "--no-absolute-filenames" <<  "-iuvdF";
    } else if (type == "deb") {
        // TODO use QTemporaryFile (setAutoRemove(false) when asynchrone)
        cpioName = QDir::tempPath() + QStringLiteral("/contents.tar");

        KrLinecountingProcess dpkg;
        dpkg << KrServices::fullPathName("dpkg") << "--fsys-tarfile" << archive;
        dpkg.setStandardOutputFile(cpioName); // TODO maybe no tmpfile but a pipe (setStandardOutputProcess(packer))
        dpkg.start();
        if (!dpkg.waitForFinished() || dpkg.exitStatus() != QProcess::NormalExit || !checkStatus("deb", dpkg.exitCode())) {
            observer->detailedError(i18n("Failed to convert deb (%1) to tar.", archive), dpkg.getErrorMsg());
            return 0;
        }

        archive = cpioName;
        packer << KrServices::fullPathName("tar") << "xvf";
    } else return false;

    if (!password.isNull()) {
        if (type == "zip")
            packer << "-P" << password;
        if (type == "arj")
            packer << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            packer << QString("-p%1").arg(password);
    }

    // unpack the files
    KrLinecountingProcess proc;
    proc << packer << archive;
    if (type == "bzip2" || type == "gzip" || type == "lzma" || type == "xz") {
        QString arcname = archive.mid(archive.lastIndexOf("/") + 1);
        if (arcname.contains(".")) arcname = arcname.left(arcname.lastIndexOf("."));
            proc.setStandardOutputFile(dest + '/' + arcname);
    }
    if (type == "ace" && QFile("/dev/ptmx").exists())    // Don't remove, unace crashes if missing!!!
        proc.setStandardInputFile("/dev/ptmx");

    proc.setWorkingDirectory(dest);

    // tell the user to wait
    observer->subJobStarted(i18n("Unpacking File(s)"), count);
    if (count != 0) {
        connect(&proc, SIGNAL(newOutputLines(int)),
                observer, SLOT(incrementProgress(int)));
        if (type == "rpm")
            connect(&proc, SIGNAL(newErrorLines(int)),
                    observer, SLOT(incrementProgress(int)));
    }

    // start the unpacking process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }
    ; // busy wait - need to find something better...
    observer->subJobStopped();

    if (!cpioName.isEmpty())
        QFile(cpioName).remove();      /* remove the cpio file */

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode())) {
        observer->detailedError(i18n("Failed to unpack %1.", archive),
                                observer->wasCancelled() ? i18n("User cancelled.") : proc.getErrorMsg());
        return false;
    }
    return true; // SUCCESS
}

bool KRarcHandler::test(QString archive, QString type, QString password, KRarcObserver *observer, long count)
{
    // choose the right packer for the job
    QStringList packer;

    // set the right packer to do the job
    if (type == "zip") packer << KrServices::fullPathName("unzip") << "-t";
    else if (type == "tar") packer << KrServices::fullPathName("tar") << "-tvf";
    else if (type == "tgz") packer << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tarz") packer << KrServices::fullPathName("tar") << "-tvzf";
    else if (type == "tbz") packer << KrServices::fullPathName("tar") << "-tjvf";
    else if (type == "tlz") packer << KrServices::fullPathName("tar") << "--lzma" << "-tvf";
    else if (type == "txz") packer << KrServices::fullPathName("tar") << "--xz" << "-tvf";
    else if (type == "gzip") packer << KrServices::fullPathName("gzip") << "-tv";
    else if (type == "bzip2") packer << KrServices::fullPathName("bzip2") << "-tv";
    else if (type == "lzma") packer << KrServices::fullPathName("lzma") << "-tv";
    else if (type == "xz")  packer << KrServices::fullPathName("xz") << "-tv";
    else if (type == "rar") packer << KrServices::fullPathName(KrServices::cmdExist("rar") ? "rar" : "unrar") << "t";
    else if (type == "ace") packer << KrServices::fullPathName("unace") << "t";
    else if (type == "lha") packer << KrServices::fullPathName("lha") << "t";
    else if (type == "arj") packer << KrServices::fullPathName(KrServices::cmdExist("arj") ? "arj" : "unarj") << "t";
    else if (type == "cpio") packer << KrServices::fullPathName("cpio") << "--only-verify-crc" << "-tvF";
    else if (type == "7z")  packer << KrServices::fullPathName("7z") << "-y" << "t";
    else return false;

    if (!password.isNull()) {
        if (type == "zip")
            packer << "-P" << password;
        if (type == "arj")
            packer << QString("-g%1").arg(password);
        if (type == "ace" || type == "rar" || type == "7z")
            packer << QString("-p%1").arg(password);
    }

    // unpack the files
    KrLinecountingProcess proc;
    proc << packer << archive;

    if (type == "ace" && QFile("/dev/ptmx").exists())    // Don't remove, unace crashes if missing!!!
        proc.setStandardInputFile("/dev/ptmx");

    // tell the user to wait
    observer->subJobStarted(i18n("Testing Archive"), count);
    if (count != 0)
        connect(&proc, SIGNAL(newOutputLines(int)),
                observer, SLOT(incrementProgress(int)));

    // start the unpacking process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }
    ; // busy wait - need to find something better...
    observer->subJobStopped();

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode()))
        return false;

    return true; // SUCCESS
}

bool KRarcHandler::pack(QStringList fileNames, QString type, QString dest, long count, QMap<QString, QString> extraProps, KRarcObserver *observer)
{
    // set the right packer to do the job
    QStringList packer;

    if (type == "zip") {
        packer << KrServices::fullPathName("zip") << "-ry";
    } else if (type == "cbz") {
        packer << KrServices::fullPathName("zip") << "-ry";
        type = "zip";
    } else if (type == "tar") {
        packer << KrServices::fullPathName("tar") << "-cvf";
    } else if (type == "tar.gz") {
        packer << KrServices::fullPathName("tar") << "-cvzf";
        type = "tgz";
    } else if (type == "tar.bz2") {
        packer << KrServices::fullPathName("tar") << "-cvjf";
        type = "tbz";
    } else if (type == "tar.lzma") {
        packer << KrServices::fullPathName("tar") << "--lzma" << "-cvf";
        type = "tlz";
    } else if (type == "tar.xz") {
        packer << KrServices::fullPathName("tar") << "--xz" << "-cvf";
        type = "txz";
    } else if (type == "rar") {
        packer << KrServices::fullPathName("rar") << "-r" << "a";
    } else if (type == "cbr") {
        packer << KrServices::fullPathName("rar") << "-r" << "a";
        type = "rar";
    } else if (type == "lha") {
        packer << KrServices::fullPathName("lha") << "a";
    } else if (type == "arj") {
        packer << KrServices::fullPathName("arj") << "-r" << "-y" << "a";
    } else if (type == "7z") {
        packer << KrServices::fullPathName("7z") << "-y" << "a";
    } else return false;

    QString password;

    if (extraProps.count("Password") > 0) {
        password = extraProps[ "Password" ];

        if (!password.isNull()) {
            if (type == "zip")
                packer << "-P" << password;
            else if (type == "arj")
                packer << QString("-g%1").arg(password);
            else if (type == "ace" || type == "7z")
                packer << QString("-p%1").arg(password);
            else if (type == "rar") {
                if (extraProps.count("EncryptHeaders") > 0)
                    packer << QString("-hp%1").arg(password);
                else
                    packer << QString("-p%1").arg(password);
            } else
                password.clear();
        }
    }

    if (extraProps.count("VolumeSize") > 0) {
        QString sizeStr = extraProps[ "VolumeSize" ];
        KIO::filesize_t size = sizeStr.toLongLong();

        if (size >= 10000) {
            if (type == "arj" || type == "rar")
                packer << QString("-v%1b").arg(sizeStr);
        }
    }

    if (extraProps.count("CompressionLevel") > 0) {
        int level = extraProps[ "CompressionLevel" ].toInt() - 1;
        if (level < 0)
            level = 0;
        if (level > 8)
            level = 8;

        if (type == "rar") {
            static const int rarLevels[] = { 0, 1, 2, 2, 3, 3, 4, 4, 5 };
            packer << QString("-m%1").arg(rarLevels[ level ]);
        } else if (type == "arj") {
            static const int arjLevels[] = { 0, 4, 4, 3, 3, 2, 2, 1, 1 };
            packer << QString("-m%1").arg(arjLevels[ level ]);
        } else if (type == "zip") {
            static const int zipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
            packer << QString("-%1").arg(zipLevels[ level ]);
        } else if (type == "7z") {
            static const int sevenZipLevels[] = { 0, 1, 2, 4, 5, 6, 7, 8, 9 };
            packer << QString("-mx%1").arg(sevenZipLevels[ level ]);
        }
    }

    if (extraProps.count("CommandLineSwitches") > 0)
        packer << QString("%1").arg(extraProps[ "CommandLineSwitches" ]);

    // prepare to pack
    KrLinecountingProcess proc;
    proc << packer << dest;

    for (QStringList::Iterator file = fileNames.begin(); file != fileNames.end(); ++file) {
        proc << *file;
    }

    // tell the user to wait
    observer->subJobStarted(i18n("Packing File(s)"), count);
    if (count != 0)
        connect(&proc, SIGNAL(newOutputLines(int)),
                observer, SLOT(incrementProgress(int)));

    // start the packing process
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            observer->processEvents();
            if (observer->wasCancelled())
                proc.kill();
        }
    ; // busy wait - need to find something better...
    observer->subJobStopped();

    // check the return value
    if (proc.exitStatus() != QProcess::NormalExit || !checkStatus(type, proc.exitCode())) {
        observer->detailedError(i18n("Failed to pack %1.", dest),
                                observer->wasCancelled() ? i18n("User cancelled.") : proc.getErrorMsg());
        return false;
    }

    KConfigGroup group(krConfig, "Archives");
    if (group.readEntry("Test Archives", _TestArchives) &&
            !test(dest, type, password, observer, count)) {
        observer->error(i18n("Failed to pack %1.", dest));
        return false;
    }
    return true; // SUCCESS
}

bool KRarcHandler::openWallet()
{
    if (!wallet) {
        // find a suitable parent window
        QWidget *actWindow = QApplication::activeWindow();
        if (!actWindow)
            actWindow = (QWidget*) QApplication::desktop();

        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), actWindow->effectiveWinId());
    }
    return (wallet != 0);
}

QString KRarcHandler::getPassword(QString path)
{
    QString password;

    QString key = "krarc-" + path;

    if (!KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(), KWallet::Wallet::PasswordFolder(), key)) {
        if (!KWallet::Wallet::isOpen(KWallet::Wallet::NetworkWallet())  && wallet != 0) {
            delete wallet;
            wallet = 0;
        }
        if (openWallet() && wallet->hasFolder(KWallet::Wallet::PasswordFolder())) {
            wallet->setFolder(KWallet::Wallet::PasswordFolder());
            QMap<QString, QString> map;
            if (wallet->readMap(key, map) == 0) {
                QMap<QString, QString>::const_iterator it = map.constFind("password");
                if (it != map.constEnd())
                    password = it.value();
            }
        }
    }

    bool keep = true;
    QString user = "archive";
    QPointer<KPasswordDialog> passDlg = new KPasswordDialog(0L, KPasswordDialog::ShowKeepPassword);
            passDlg->setPrompt(i18n("This archive is encrypted, please supply the password:") ),
            passDlg->setUsername(user);
    passDlg->setPassword(password);
    if (passDlg->exec() == KPasswordDialog::Accepted) {
        password = passDlg->password();
        if (keep) {
            if (!KWallet::Wallet::isOpen(KWallet::Wallet::NetworkWallet()) && wallet != 0) {
                delete wallet;
                wallet = 0;
            }
            if (openWallet()) {
                bool ok = true;
                if (!wallet->hasFolder(KWallet::Wallet::PasswordFolder()))
                    ok = wallet->createFolder(KWallet::Wallet::PasswordFolder());
                if (ok) {
                    wallet->setFolder(KWallet::Wallet::PasswordFolder());
                    QMap<QString, QString> map;
                    map.insert("login", "archive");
                    map.insert("password", password);
                    wallet->writeMap(key, map);
                }
            }
        }
        delete passDlg;
        return password;
    }
    delete passDlg;

    return "";
}

bool KRarcHandler::isArchive(const QUrl &url)
{
    QString protocol = url.scheme();
    if (arcProtocols.indexOf(protocol) != -1)
        return true;
    else return false;
}

QString KRarcHandler::getType(bool &encrypted, QString fileName, QString mime, bool checkEncrypted, bool fast)
{
    QString result = detectArchive(encrypted, fileName, checkEncrypted, fast);
    if (result.isNull()) {
        // Then the type is based on the mime type
        return getShortTypeFromMime(mime);
    }
    return result;
}

bool KRarcHandler::checkStatus(QString type, int exitCode)
{
    return KrArcBaseManager::checkStatus(type, exitCode);
}

void KRarcHandler::checkIf7zIsEncrypted(bool &encrypted, QString fileName)
{
    Kr7zEncryptionChecker proc;
    // TODO incorporate all this in Kr7zEncryptionChecker
    proc << KrServices::fullPathName("7z") << "-y" << "t";
    proc << fileName;
    proc.start();
    proc.waitForFinished();
    encrypted = proc.isEncrypted();
}

