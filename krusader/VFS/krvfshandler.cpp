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

#include "krvfshandler.h"

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

// QtCore
#include <QDir>

#include "default_vfs.h"
#include "virt_vfs.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../JobMan/jobman.h"


vfs* KrVfsHandler::getVfs(const QUrl &url, vfs* oldVfs)
{
    if (oldVfs && oldVfs->type() == getVfsType(url)) {
        return oldVfs;
    }

    vfs *newVfs = createVfs(url);

    QPointer<vfs> vfsPointer(newVfs);
    _vfs_list.append(vfsPointer);
    connect(newVfs, &vfs::filesystemChanged, this, &KrVfsHandler::refreshVfs);
    connect(newVfs, &vfs::newJob, krJobMan, &JobMan::manageJob);

    return newVfs;
}

void KrVfsHandler::startCopyFiles(const QList<QUrl> &urls, const QUrl &destination,
                                  KIO::CopyJob::CopyMode mode, bool showProgressInfo, bool enqueue)
{
    vfs *vfs = getVfs(destination); // implementation depends on filesystem of destination
    vfs->copyFiles(urls, destination, mode, showProgressInfo, enqueue);
}

void KrVfsHandler::refreshVfs(const QUrl &directory)
{
    QMutableListIterator<QPointer<vfs>> it(_vfs_list);
    while (it.hasNext()) {
        if (it.next().isNull()) {
            it.remove();
        }
    }

    // refresh all vfs currently showing this directory
    for(QPointer<vfs> vfsPointer: _vfs_list) {
        // always refresh virtual vfs showing a virtual directory; it can contain files from various
        // places, we don't know if they were (re)moved, refreshing is also fast enough
        vfs *vfs = vfsPointer.data();
        const QUrl vfsDir = vfs->currentDirectory();
        if (vfsDir == vfs::cleanUrl(directory) || (vfsDir.scheme() == "virt" && !vfs->isRoot())) {
            vfs->mayRefresh();
        }
    }
}

// ==== static ====

KrVfsHandler &KrVfsHandler::instance()
{
    static KrVfsHandler instance;
    return instance;
}

vfs::VFS_TYPE KrVfsHandler::getVfsType(const QUrl &url)
{
    return url.scheme() == QStringLiteral("virt") ? vfs::VFS_VIRT : vfs::VFS_DEFAULT;
}

vfs* KrVfsHandler::createVfs(const QUrl &url)
{
    vfs::VFS_TYPE newType = getVfsType(url);
    switch (newType) {
    case (vfs::VFS_VIRT):
        return new virt_vfs();
    default:
        return new default_vfs();
    }
}

void KrVfsHandler::getACL(vfile *file, QString &acl, QString &defAcl)
{
    Q_UNUSED(file);
    acl.clear();
    defAcl.clear();
#ifdef HAVE_POSIX_ACL
    QString fileName = vfs::cleanUrl(file->vfile_getUrl()).path();
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
    if (acl_extended_file(fileName)) {
#endif
        acl = getACL(fileName, ACL_TYPE_ACCESS);
        if (file->vfile_isDir())
            defAcl = getACL(fileName, ACL_TYPE_DEFAULT);
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
    }
#endif
#endif
}

QString KrVfsHandler::getACL(const QString & path, int type)
{
    Q_UNUSED(path);
    Q_UNUSED(type);
#ifdef HAVE_POSIX_ACL
    acl_t acl = 0;
    // do we have an acl for the file, and/or a default acl for the dir, if it is one?
    if ((acl = acl_get_file(path.toLocal8Bit(), type)) != 0) {
        bool aclExtended = false;

#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
        aclExtended = acl_equiv_mode(acl, 0);
#else
        acl_entry_t entry;
        int ret = acl_get_entry(acl, ACL_FIRST_ENTRY, &entry);
        while (ret == 1) {
            acl_tag_t currentTag;
            acl_get_tag_type(entry, &currentTag);
            if (currentTag != ACL_USER_OBJ &&
                    currentTag != ACL_GROUP_OBJ &&
                    currentTag != ACL_OTHER) {
                aclExtended = true;
                break;
            }
            ret = acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
        }
#endif

        if (!aclExtended) {
            acl_free(acl);
            acl = 0;
        }
    }

    if (acl == 0)
        return QString();

    char *aclString = acl_to_text(acl, 0);
    QString ret = QString::fromLatin1(aclString);
    acl_free((void*)aclString);
    acl_free(acl);

    return ret;
#else
    return QString();
#endif
}
