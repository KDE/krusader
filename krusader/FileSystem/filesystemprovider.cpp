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

#include "filesystemprovider.h"

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

// QtCore
#include <QDir>

#include <KIOCore/KMountPoint>

#include "defaultfilesystem.h"
#include "virtualfilesystem.h"
#include "../krservices.h"


FileSystemProvider::FileSystemProvider() : _defaultVFS(0), _virtVFS(0) {}

vfs *FileSystemProvider::getVfs(const QUrl &url, vfs *oldVfs)
{
    const vfs::VFS_TYPE type = getVfsType(url);
    return oldVfs && oldVfs->type() == type ? oldVfs : createVfs(type);
}

void FileSystemProvider::startCopyFiles(const QList<QUrl> &urls, const QUrl &destination,
                                  KIO::CopyJob::CopyMode mode, bool showProgressInfo, bool reverseQueueMode, bool startPaused)
{
    const vfs::VFS_TYPE type = getVfsType(destination);
    vfs *vfs;
    switch (type) {
    case vfs::VFS_VIRT:
        if (!_virtVFS)
            _virtVFS = createVfs(type);
        vfs = _virtVFS;
        break;
    default:
        if (!_defaultVFS)
            _defaultVFS = createVfs(type);
        vfs = _defaultVFS;
    }

    vfs->copyFiles(urls, destination, mode, showProgressInfo, reverseQueueMode, startPaused);
}

void FileSystemProvider::refreshVfs(const QUrl &directory)
{
    QMutableListIterator<QPointer<vfs>> it(_vfs_list);
    while (it.hasNext()) {
        if (it.next().isNull()) {
            it.remove();
        }
    }

    QString mountPoint = "";
    if (directory.isLocalFile()) {
        KMountPoint::Ptr kMountPoint = KMountPoint::currentMountPoints().findByPath(directory.path());
        if (kMountPoint)
            mountPoint = kMountPoint->mountPoint();
    }

    for(QPointer<vfs> vfsPointer: _vfs_list) {
        // always refresh virtual vfs showing a virtual directory; it can contain files from various
        // places, we don't know if they were (re)moved, refreshing is also fast enough
        vfs *vfs = vfsPointer.data();
        const QUrl vfsDir = vfs->currentDirectory();
        if ((vfsDir == vfs::cleanUrl(directory) || (vfsDir.scheme() == "virt" && !vfs->isRoot()))
            && !vfs->hasAutoUpdate()) {
            // refresh all vfs currently showing this directory...
            vfs->refresh();
        } else if (!mountPoint.isEmpty() && mountPoint == vfs->mountPoint()) {
            // ..or refresh filesystem info if mount point is the same (for free space update)
            vfs->updateFilesystemInfo();
        }
    }
}

vfs *FileSystemProvider::createVfs(vfs::VFS_TYPE type)
{
    vfs *newVfs;
    switch (type) {
    case (vfs::VFS_VIRT): newVfs = new VirtualFileSystem(); break;
    default: newVfs = new DefaultFileSystem();
    }

    QPointer<vfs> vfsPointer(newVfs);
    _vfs_list.append(vfsPointer);
    connect(newVfs, &vfs::filesystemChanged, this, &FileSystemProvider::refreshVfs);
    return newVfs;
}

// ==== static ====

FileSystemProvider &FileSystemProvider::instance()
{
    static FileSystemProvider instance;
    return instance;
}

vfs::VFS_TYPE FileSystemProvider::getVfsType(const QUrl &url)
{
    return url.scheme() == QStringLiteral("virt") ? vfs::VFS_VIRT : vfs::VFS_DEFAULT;
}

void FileSystemProvider::getACL(vfile *file, QString &acl, QString &defAcl)
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

QString FileSystemProvider::getACL(const QString & path, int type)
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
