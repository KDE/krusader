/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef KRSEARCHMOD_H
#define KRSEARCHMOD_H

// QtCore
#include <QDateTime>
#include <QObject>
#include <QStack>
#include <QStringList>
#include <QUrl>
#include <QElapsedTimer>

#include <KIO/Global>

class KrQuery;
class FileItem;
class FileSystem;
class DefaultFileSystem;
class VirtualFileSystem;

/**
 * Search for files based on a search query.
 *
 * Subdirectories are included if query->isRecursive() is true.
 */
class KrSearchMod : public QObject
{
    Q_OBJECT
public:
    explicit KrSearchMod(const KrQuery *query);
    ~KrSearchMod() override;

    void start();
    void stop();

private:
    void scanUrl(const QUrl &url);
    void scanDirectory(const QUrl &url);
    FileSystem *getFileSystem(const QUrl &url);

signals:
    void searching(const QString &url);
    void found(const FileItem &file, const QString &foundText);
    // NOTE: unused
    void error(const QUrl &url);
    void finished();

private slots:
    void slotProcessEvents(bool &stopped);

private:
    KrQuery *m_query;
    DefaultFileSystem *m_defaultFileSystem;
    VirtualFileSystem *m_virtualFileSystem;

    bool m_stopSearch;

    QStack<QUrl> m_scannedUrls;
    QStack<QUrl> m_unScannedUrls;
    QElapsedTimer m_timer;
};

#endif
