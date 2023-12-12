/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSEARCHMOD_H
#define KRSEARCHMOD_H

// QtCore
#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QStack>
#include <QStringList>
#include <QUrl>

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
