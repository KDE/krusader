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

#include "filesearcher.h"

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

#define EVENT_PROCESS_DELAY 250 // milliseconds

static const QStringList TAR_TYPES = QStringList() << "tbz" << "tgz" << "tarz" << "tar" << "tlz";

extern KRarcHandler arcHandler;

FileSearcher::FileSearcher(const KRQuery &query)
    : m_query(query), m_defaultFileSystem(nullptr), m_virtualFileSystem(nullptr)
{
    connect(&m_query, &KRQuery::status, this, &FileSearcher::searching);
    connect(&m_query, &KRQuery::processEvents, this, &FileSearcher::slotProcessEvents);
}

FileSearcher::~FileSearcher()
{
    if (m_defaultFileSystem)
        delete m_defaultFileSystem;
    if (m_virtualFileSystem)
        delete m_virtualFileSystem;
}

void FileSearcher::start(const QUrl &url)
{
    m_foundFiles.clear();
    m_unScannedUrls.clear();
    m_scannedUrls.clear();
    m_timer.start();
    m_stopSearch = false;

    // search every dir that needs to be searched
    const QList<QUrl> urls = url.isEmpty() ? m_query.searchInDirs() : QList<QUrl>() << url;
    for (const QUrl url : urls) {
        if (m_stopSearch)
            break;

        scanUrl(url);
    }

    emit finished();
}

void FileSearcher::stop() { m_stopSearch = true; }

void FileSearcher::scanUrl(const QUrl &url)
{

    m_unScannedUrls.push(url);
    while (!m_unScannedUrls.isEmpty()) {
        if (m_stopSearch)
            return;

        const QUrl url = m_unScannedUrls.pop();

        if (m_query.isExcluded(url)) {
            if (!m_query.searchInDirs().contains(url))
                continue;
        }

        if (m_scannedUrls.contains(url))
            // avoid endless loop
            continue;

        m_scannedUrls.push(url);

        emit searching(url.toDisplayString(QUrl::PreferLocalFile));

        scanDirectory(url);
    }
}

void FileSearcher::scanDirectory(const QUrl &url)
{
    FileSystem *fileSystem = getFileSystem(url);

    // create file items, deletes old ones
    const bool refreshed = fileSystem->scanDir(url);
    if (!refreshed) {
        emit error(url);
        return;
    }

    for (FileItem *fileItem : fileSystem->fileItems()) {
        const QUrl fileUrl = fileItem->getUrl();

        if (m_query.ignoreHidden() && fileUrl.fileName().startsWith('.'))
            // ignore hidden files and directories
            continue;

        if (m_query.isRecursive() &&
            (fileItem->isDir() || (fileItem->isSymLink() && m_query.followLinks()))) {
            // query search in subdirectory
            m_unScannedUrls.push(fileUrl);
        }

        if (m_query.searchInArchives() && fileUrl.isLocalFile() &&
            KRarcHandler::arcSupported(fileItem->getMime())) {
            // query search in archive; NOTE: only supported for local files
            QUrl archiveURL = fileUrl;
            bool encrypted;
            const QString type = arcHandler.getType(encrypted, fileUrl.path(), fileItem->getMime());

            if (!encrypted) {
                archiveURL.setScheme(TAR_TYPES.contains(type) ? "tar" : "krarc");
                m_unScannedUrls.push(archiveURL);
            }
        }

        if (m_query.match(fileItem)) {
            // found!
            m_foundFiles.append(fileItem);
            emit found(*fileItem, m_query.foundText()); // emitting copy of file item
        }

        if (m_timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            m_timer.start();
            if (m_stopSearch)
                return;
        }
    }
}

FileSystem *FileSearcher::getFileSystem(const QUrl &url)
{
    FileSystem *fileSystem;
    if (url.scheme() == QStringLiteral("virt")) {
        if (!m_virtualFileSystem)
            m_virtualFileSystem = new VirtualFileSystem();
        fileSystem = m_virtualFileSystem;
    } else {
        if (!m_defaultFileSystem)
            m_defaultFileSystem = new DefaultFileSystem();
        fileSystem = m_defaultFileSystem;
    }
    return fileSystem;
}

void FileSearcher::slotProcessEvents(bool &stopped)
{
    qApp->processEvents();
    stopped = m_stopSearch;
}
