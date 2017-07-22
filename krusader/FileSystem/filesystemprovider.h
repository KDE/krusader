/*****************************************************************************
 * Copyright (C) 2003 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2003 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef FILESYSTEMPROVIDER_H
#define FILESYSTEMPROVIDER_H

// QtCore
#include <QObject>
#include <QUrl>

#include <KIO/Job>

#include "filesystem.h"

/**
 * @brief Provider for virtual file systems.
 *
 * This is a singleton.
 */
class FileSystemProvider : public QObject {
    Q_OBJECT

public:
    /**
     * Get a filesystem implementation for the filesystem target specified by URL. oldFilesystem is returned if
     * the filesystem did not change.
     *
     * The filesystem instances returned by this method are already connected with this handler and will
     * notify each other about filesystem changes.
     */
    FileSystem *getFilesystem(const QUrl &url, FileSystem *oldFilesystem = 0);

    /**
     * Start a copy job for copying, moving or linking files to a destination directory.
     * Operation may be implemented async depending on destination filesystem.
     */
    void startCopyFiles(const QList<QUrl> &urls, const QUrl &destination,
                        KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                        bool showProgressInfo = true,
                        JobMan::StartMode startMode = JobMan::Default);

    /**
     * Handle file dropping. Starts a copy job for copying, moving or linking files to a destination
     * directory after use choose the action in a context menu.
     *
     * Operation may implemented async depending on destination filesystem.
     */
    void startDropFiles(QDropEvent *event, const QUrl &destination);

    /**
     * Start a delete job for trashing or deleting files.
     *
     * Operation implemented async.
     */
    void startDeleteFiles(const QList<QUrl> &urls, bool moveToTrash = true);

    static FileSystemProvider &instance();
    static FileSystem::FS_TYPE getFilesystemType(const QUrl &url);
    /** Get ACL permissions for a file */
    static void getACL(FileItem *file, QString &acl, QString &defAcl);

public slots:
    void refreshFilesystem(const QUrl &directory);

private:
    FileSystem *getFilesystemInstance(const QUrl &directory);
    FileSystem *createFilesystem(const FileSystem::FS_TYPE type);
    FileSystemProvider();

    // filesystem instances for directory independent file operations, lazy initialized
    FileSystem *_defaultFileSystem;
    FileSystem *_virtFileSystem;

    QList<QPointer<FileSystem>> _fileSystems;

    static QString getACL(const QString & path, int type);
};

#endif

