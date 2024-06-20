/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filesystemprovider.h"

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

// QtCore
#include <QDebug>
#include <QDir>

#include <KMountPoint>

#include "../JobMan/jobman.h"
#include "../krservices.h"
#include "defaultfilesystem.h"
#include "fileitem.h"
#include "virtualfilesystem.h"

FileSystemProvider::FileSystemProvider()
    : _defaultFileSystem(nullptr)
    , _virtFileSystem(nullptr)
{
}

FileSystem *FileSystemProvider::getFilesystem(const QUrl &url, FileSystem *oldFilesystem)
{
    const FileSystem::FS_TYPE type = getFilesystemType(url);
    return oldFilesystem && oldFilesystem->type() == type ? oldFilesystem : createFilesystem(type);
}

void FileSystemProvider::startCopyFiles(const QList<QUrl> &urls,
                                        const QUrl &destination,
                                        KIO::CopyJob::CopyMode mode,
                                        bool showProgressInfo,
                                        JobMan::StartMode startMode)
{
    FileSystem *fs = getFilesystemInstance(destination);
    fs->copyFiles(urls, destination, mode, showProgressInfo, startMode);
}

void FileSystemProvider::startDropFiles(QDropEvent *event, const QUrl &destination, QWidget *targetWidget)
{
    FileSystem *fs = getFilesystemInstance(destination);
    fs->dropFiles(destination, event, targetWidget);
}

void FileSystemProvider::startDeleteFiles(const QList<QUrl> &urls, bool moveToTrash)
{
    if (urls.isEmpty())
        return;

    // assume all URLs use the same filesystem
    FileSystem *fs = getFilesystemInstance(urls.first());
    fs->deleteFiles(urls, moveToTrash);
}

void FileSystemProvider::refreshFilesystems(const QUrl &directory, bool removed)
{
    qDebug() << "changed=" << directory.toDisplayString();

    QMutableListIterator<QPointer<FileSystem>> it(_fileSystems);
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

    for (const QPointer<FileSystem> &fileSystemPointer : _fileSystems) {
        FileSystem *fs = fileSystemPointer.data();
        // refresh all filesystems currently showing this directory
        // and always refresh filesystems showing a virtual directory; it can contain files from
        // various places, we don't know if they were (re)moved. Refreshing is also fast enough.
        const QUrl fileSystemDir = fs->currentDirectory();
        if ((!fs->hasAutoUpdate() && (fileSystemDir == FileSystem::cleanUrl(directory) || (fs->type() == FileSystem::FS_VIRTUAL && !fs->isRoot())))
            // also refresh if a parent directory was (re)moved (not detected by file watcher)
            || (removed && directory.isParentOf(fileSystemDir))) {
            fs->refresh();
            // ..or refresh filesystem info if mount point is the same (for free space update)
        } else if (!mountPoint.isEmpty() && mountPoint == fs->mountPoint()) {
            fs->updateFilesystemInfo();
        }
    }
}

FileSystem *FileSystemProvider::getFilesystemInstance(const QUrl &directory)
{
    const FileSystem::FS_TYPE type = getFilesystemType(directory);
    switch (type) {
    case FileSystem::FS_VIRTUAL:
        if (!_virtFileSystem)
            _virtFileSystem = createFilesystem(type);
        return _virtFileSystem;
        break;
    default:
        if (!_defaultFileSystem)
            _defaultFileSystem = createFilesystem(type);
        return _defaultFileSystem;
    }
}

FileSystem *FileSystemProvider::createFilesystem(FileSystem::FS_TYPE type)
{
    FileSystem *newFilesystem;
    switch (type) {
    case (FileSystem::FS_VIRTUAL):
        newFilesystem = new VirtualFileSystem();
        break;
    default:
        newFilesystem = new DefaultFileSystem();
    }

    QPointer<FileSystem> fileSystemPointer(newFilesystem);
    _fileSystems.append(fileSystemPointer);
    connect(newFilesystem, &FileSystem::fileSystemChanged, this, &FileSystemProvider::refreshFilesystems);
    return newFilesystem;
}

// ==== static ====

FileSystemProvider &FileSystemProvider::instance()
{
    static FileSystemProvider instance;
    return instance;
}

FileSystem::FS_TYPE FileSystemProvider::getFilesystemType(const QUrl &url)
{
    return url.scheme() == QStringLiteral("virt") ? FileSystem::FS_VIRTUAL : FileSystem::FS_DEFAULT;
}

void FileSystemProvider::getACL(FileItem *file, QString &acl, QString &defAcl)
{
    Q_UNUSED(file);
    acl.clear();
    defAcl.clear();
#ifdef HAVE_POSIX_ACL
    QString fileName = FileSystem::cleanUrl(file->getUrl()).path();
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
    if (acl_extended_file(fileName)) {
#endif
        acl = getACL(fileName, ACL_TYPE_ACCESS);
        if (file->isDir())
            defAcl = getACL(fileName, ACL_TYPE_DEFAULT);
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
    }
#endif
#endif
}

QString FileSystemProvider::getACL(const QString &path, int type)
{
    Q_UNUSED(path);
    Q_UNUSED(type);
#ifdef HAVE_POSIX_ACL
    acl_t acl = nullptr;
    // do we have an acl for the file, and/or a default acl for the dir, if it is one?
    if ((acl = acl_get_file(path.toLocal8Bit().constData(), type)) != nullptr) {
        bool aclExtended = false;

#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
        aclExtended = acl_equiv_mode(acl, 0);
#else
        acl_entry_t entry;
        int ret = acl_get_entry(acl, ACL_FIRST_ENTRY, &entry);
        while (ret == 1) {
            acl_tag_t currentTag;
            acl_get_tag_type(entry, &currentTag);
            if (currentTag != ACL_USER_OBJ && currentTag != ACL_GROUP_OBJ && currentTag != ACL_OTHER) {
                aclExtended = true;
                break;
            }
            ret = acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
        }
#endif

        if (!aclExtended) {
            acl_free(acl);
            acl = nullptr;
        }
    }

    if (acl == nullptr)
        return QString();

    char *aclString = acl_to_text(acl, nullptr);
    QString ret = QString::fromLatin1(aclString);
    acl_free((void *)aclString);
    acl_free(acl);

    return ret;
#else
    return QString();
#endif
}
