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

#ifndef VIRT_VFS_H
#define VIRT_VFS_H

#include <QtCore/QHash>

#include <KConfigCore/KConfig>

#include "vfs.h"

class virt_vfs : public vfs
{
    Q_OBJECT
public:
    virt_vfs(QObject* panel, bool quiet = false);
    ~virt_vfs();

    /// Copy a file to the vfs (physical).
    void vfs_addFiles(QList<QUrl> *fileUrls, KIO::CopyJob::CopyMode mode, QObject* toNotify, QString dir = "",  PreserveMode pmode = PM_DEFAULT);
    /// Remove a file from the vfs (physical)
    void vfs_delFiles(QStringList *fileNames, bool reallyDelete = true);
    /// Remove a file from the collection (only its link, not the file)
    void vfs_removeFiles(QStringList *fileNames);
    /// Return a list of URLs for multiple files
    QList<QUrl>* vfs_getFiles(QStringList* names);
    /// Return a URL to a single file
    QUrl vfs_getFile(const QString& name);
    /// Create a new directory
    void vfs_mkdir(const QString& name);
    /// Rename file
    void vfs_rename(const QString& fileName, const QString& newName);
    /// Calculate the amount of space occupied by a file or directory (recursive).
    virtual void vfs_calcSpace(QString name , KIO::filesize_t *totalSize, unsigned long *totalFiles, unsigned long *totalDirs, bool * stop);

    /// Return the VFS working dir
    QString vfs_workingDir() {
        return QString();
    }

    virtual QString metaInformation() {
        return metaInfo;
    }
    virtual void setMetaInformation(QString info);

protected slots:
    void slotStatResult(KJob *job);

protected:
    /// Save the dictionary to file
    bool save();
    /// Restore the dictionary from file
    bool restore();
    /// return the URLs DB
    KConfig*  getVirtDB();

    bool populateVfsList(const QUrl &origin, bool showHidden);
    vfile* stat(const QUrl &url);

    static QHash<QString, QList<QUrl> *> virtVfsDict;
    static QHash<QString, QString> metaInfoDict;
    static KConfig* virt_vfs_db;
    bool busy;
    QString path;
    QString metaInfo;
    KIO::UDSEntry entry;
};

#endif
