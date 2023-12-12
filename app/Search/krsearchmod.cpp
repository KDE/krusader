/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include "../krglobal.h"

#define EVENT_PROCESS_DELAY 250 // milliseconds

static const QStringList TAR_TYPES = QStringList() << "tbz"
                                                   << "tgz"
                                                   << "tarz"
                                                   << "tar"
                                                   << "tlz";

KrSearchMod::KrSearchMod(const KrQuery *query)
    : m_defaultFileSystem(nullptr)
    , m_virtualFileSystem(nullptr)
    , m_stopSearch(false)
{
    m_query = new KrQuery(*query);
    connect(m_query, &KrQuery::status, this, &KrSearchMod::searching);
    connect(m_query, &KrQuery::processEvents, this, &KrSearchMod::slotProcessEvents);
}

KrSearchMod::~KrSearchMod()
{
    delete m_query;
    if (m_defaultFileSystem)
        delete m_defaultFileSystem;
    if (m_virtualFileSystem)
        delete m_virtualFileSystem;
}

void KrSearchMod::start()
{
    m_unScannedUrls.clear();
    m_scannedUrls.clear();
    m_timer.start();

    const QList<QUrl> whereToSearch = m_query->searchInDirs();

    // search every dir that needs to be searched
    for (int i = 0; i < whereToSearch.count(); ++i)
        scanUrl(whereToSearch[i]);

    emit finished();
}

void KrSearchMod::stop()
{
    m_stopSearch = true;
}

void KrSearchMod::scanUrl(const QUrl &url)
{
    if (m_stopSearch)
        return;

    m_unScannedUrls.push(url);
    while (!m_unScannedUrls.isEmpty()) {
        const QUrl url = m_unScannedUrls.pop();

        if (m_stopSearch)
            return;

        if (m_query->isExcluded(url)) {
            if (!m_query->searchInDirs().contains(url))
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

void KrSearchMod::scanDirectory(const QUrl &url)
{
    FileSystem *fileSystem = getFileSystem(url);

    // create file items
    const bool refreshed = fileSystem->scanDir(url);
    if (!refreshed) {
        emit error(url);
        return;
    }

    for (FileItem *fileItem : fileSystem->fileItems()) {
        const QUrl fileUrl = fileItem->getUrl();

        if (m_query->isRecursive() && ((!fileItem->isSymLink() && fileItem->isDir()) || (fileItem->isSymLink() && m_query->followLinks()))) {
            // query search in subdirectory
            m_unScannedUrls.push(fileUrl);
        }

        if (m_query->searchInArchives() && fileUrl.isLocalFile() && KrArcHandler::arcSupported(fileItem->getMime())) {
            // query search in archive; NOTE: only supported for local files
            QUrl archiveURL = fileUrl;
            bool encrypted;
            const QString type = krArcMan.getType(encrypted, fileUrl.path(), fileItem->getMime());

            if (!encrypted) {
                archiveURL.setScheme(TAR_TYPES.contains(type) ? "tar" : "krarc");
                m_unScannedUrls.push(archiveURL);
            }
        }

        if (m_query->match(fileItem)) {
            // found!
            emit found(*fileItem, m_query->foundText()); // emitting copy of file item
        }

        if (m_timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            m_timer.start();
            if (m_stopSearch)
                return;
        }
    }
}

FileSystem *KrSearchMod::getFileSystem(const QUrl &url)
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

void KrSearchMod::slotProcessEvents(bool &stopped)
{
    qApp->processEvents();
    stopped = m_stopSearch;
}
