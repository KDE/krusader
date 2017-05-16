/***************************************************************************
                                krsearchmod.cpp
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
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

#include "krsearchmod.h"

// QtCore
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegExp>
// QtWidgets
#include <QApplication>
#include <qplatformdefs.h>

#include <KIO/Global>

#include "../Archive/krarchandler.h"
#include "../FileSystem/defaultfilesystem.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/krpermhandler.h"
#include "../FileSystem/krquery.h"
#include "../FileSystem/virtualfilesystem.h"

#define  EVENT_PROCESS_DELAY     250

extern KRarcHandler arcHandler;

KRSearchMod::KRSearchMod(const KRQuery *query)
    : m_defaultFileSystem(nullptr), m_virtualFileSystem(nullptr), m_stopSearch(false)
{
    m_query = new KRQuery(*query);
    connect(m_query, &KRQuery::status, this, &KRSearchMod::searching);
    connect(m_query, &KRQuery::processEvents, this, &KRSearchMod::slotProcessEvents);
}

KRSearchMod::~KRSearchMod()
{
    delete m_query;
    if (m_defaultFileSystem)
        delete m_defaultFileSystem;
    if (m_virtualFileSystem)
        delete m_virtualFileSystem;
}

void KRSearchMod::start()
{
    m_unScannedUrls.clear();
    m_scannedUrls.clear();
    m_timer.start();

    const QList<QUrl> whereToSearch = m_query->searchInDirs();

    // search every dir that needs to be searched
    for (int i = 0; i < whereToSearch.count(); ++i)
        scanURL(whereToSearch [ i ]);

    emit finished();
}

void KRSearchMod::stop()
{
    m_stopSearch = true;
}

void KRSearchMod::scanURL(const QUrl &url)
{
    if (m_stopSearch) return;

    m_unScannedUrls.push(url);
    while (!m_unScannedUrls.isEmpty()) {
        const QUrl urlToCheck = m_unScannedUrls.pop();

        if (m_stopSearch) return;

        if (m_query->isExcluded(urlToCheck)) {
            if (!m_query->searchInDirs().contains(urlToCheck))
                continue;
        }

        if (m_scannedUrls.contains(urlToCheck))
            continue;
        m_scannedUrls.push(urlToCheck);

        emit searching(urlToCheck.toDisplayString(QUrl::PreferLocalFile));

        if (urlToCheck.isLocalFile())
            scanLocalDir(urlToCheck);
        else
            scanRemoteDir(urlToCheck);
    }
}

void KRSearchMod::scanLocalDir(const QUrl &url)
{
    const QString dir = FileSystem::ensureTrailingSlash(url).path();

    QT_DIR *qdir = QT_OPENDIR(dir.toLocal8Bit());
    if (!qdir)
        return;

    QT_DIRENT *dirEnt;

    while ((dirEnt = QT_READDIR(qdir)) != NULL) {
        const QString name = QString::fromLocal8Bit(dirEnt->d_name);

        // we don't scan the ".",".." enteries
        if (name == "." || name == "..")
            continue;

        const QString fullName = dir + name;
        const QUrl fileUrl = QUrl::fromLocalFile(fullName);

        QT_STATBUF stat_p;
        QT_LSTAT(fullName.toLocal8Bit(), &stat_p);

        if (m_query->isRecursive()) {
            if (S_ISLNK(stat_p.st_mode) && m_query->followLinks())
                m_unScannedUrls.push(QUrl::fromLocalFile(QDir(fullName).canonicalPath()));
            else if (S_ISDIR(stat_p.st_mode))
                m_unScannedUrls.push(fileUrl);
        }

        // creating a file item object for matching with krquery
        FileItem *fileitem = FileSystem::createLocalFileItem(name, dir);

        if (m_query->searchInArchives()) {
            const QString mime = fileitem->getMime();
            if (KRarcHandler::arcSupported(mime)) {
                QUrl archiveURL = fileUrl;
                bool encrypted;
                const QString realType = arcHandler.getType(encrypted, fileUrl.path(), mime);

                if (!encrypted) {
                    if (realType == "tbz" || realType == "tgz" || realType == "tarz" ||
                        realType == "tar" || realType == "tlz")
                        archiveURL.setScheme("tar");
                    else
                        archiveURL.setScheme("krarc");

                    m_unScannedUrls.push(archiveURL);
                }
            }
        }

        if (m_query->match(fileitem)) {
            // if we got here - we got a winner
            m_results.append(fullName);

            emit found(*fileitem, m_query->foundText()); // emitting copy of file item
        }
        delete fileitem;

        if (m_timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            m_timer.start();
            if (m_stopSearch)
                return;
        }
    }
    // clean up
    QT_CLOSEDIR(qdir);
}

void KRSearchMod::scanRemoteDir(const QUrl &url)
{
    FileSystem * fileSystem_;

    if (url.scheme() == QStringLiteral("virt")) {
        if (m_virtualFileSystem == 0)
            m_virtualFileSystem = new VirtualFileSystem();
        fileSystem_ = m_virtualFileSystem;
    } else {
        if (m_defaultFileSystem == 0)
            m_defaultFileSystem = new DefaultFileSystem();
        fileSystem_ = m_defaultFileSystem;
    }

    if (!fileSystem_->refresh(url)) return ;

    for (FileItem *fileitem : fileSystem_->fileItems()) {
        QUrl fileURL = fileitem->getUrl();

        if (m_query->isRecursive() && ((fileitem->isSymLink() && m_query->followLinks()) || fileitem->isDir()))
            m_unScannedUrls.push(fileURL);

        if (m_query->match(fileitem)) {
            // if we got here - we got a winner
            m_results.append(fileURL.toDisplayString(QUrl::PreferLocalFile));

            emit found(*fileitem, m_query->foundText()); // emitting copy of file item
        }

        if (m_timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            m_timer.start();
            if (m_stopSearch) return;
        }
    }
}

void KRSearchMod::slotProcessEvents(bool & stopped)
{
    qApp->processEvents();
    stopped = m_stopSearch;
}

