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

#ifndef KRVFSHANDLER_H
#define KRVFSHANDLER_H

// QtCore
#include <QObject>
#include <QUrl>

#include <KIO/Job>

#include "vfs.h"

/**
 * @brief Provider for all IO and filesystem operations.
 *
 * This is a singleton.
 */
class KrVfsHandler : public QObject {
    Q_OBJECT

public:
    /**
     * Get a directory for the filesystem target specified by url
     */
    vfs *getVfs(const QUrl &url, QObject *parent = 0, vfs *oldVfs = 0);

    /**
     * Create a copy job for copying moving or linking files to destination.
     *
     * Note: the job is done async and not done when returning
     *
     * @param urls the source files
     * @param destination the destination folder
     * @param mode copy, move or link
     * @param showProgressInfo
     */
    KIO::Job *createCopyJob(const QList<QUrl> &urls, const QUrl &destination,
                            KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy,
                            bool showProgressInfo = true);

    static KrVfsHandler &instance();
    static vfs::VFS_TYPE getVfsType(const QUrl &url);
    /** Get ACL permissions */
    static void getACL(vfile *file, QString &acl, QString &defAcl);

private:
    KrVfsHandler() {}
    static QString getACL(const QString & path, int type);
};

#endif

