/*****************************************************************************
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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

#include "fileitem.h"

// QtCore
#include <QCache>
#include <QDateTime>
#include <QMimeDatabase>
#include <QMimeType>

#include <KConfigCore/KDesktopFile>

#include "krpermhandler.h"
#include "filesystemprovider.h"

bool FileItem::userDefinedFolderIcons = true;

// wrapper class; QCache needs objects
class FileSize
{
public:
    const KIO::filesize_t m_size;
    FileSize(KIO::filesize_t size) : m_size(size) {}
};

// cache for calculated directory sizes;
static QCache<const QUrl, FileSize> s_fileSizeCache(1000);

FileItem::FileItem(const QString &name, const QUrl &url, bool isDir,
             KIO::filesize_t size, mode_t mode,
             time_t mtime, time_t ctime, time_t atime,
             uid_t uid, gid_t gid, const QString &owner, const QString &group,
             bool isLink, const QString &linkDest, bool isBrokenLink,
             const QString &acl, const QString &defaultAcl)
    : m_name(name), m_url(url), m_isDir(isDir),
      m_size(size), m_mode(mode),
      m_mtime(mtime), m_ctime(ctime), m_atime(atime),
      m_uid(uid), m_gid(gid), m_owner(owner), m_group(group),
      m_isLink(isLink), m_linkDest(linkDest), m_isBrokenLink(isBrokenLink),
      m_acl(acl), m_defaulfAcl(defaultAcl), m_AclLoaded(false),
      m_mimeType(), m_iconName()
{
    m_permissions = KRpermHandler::mode2QString(mode);

    if (m_owner.isEmpty())
        m_owner = KRpermHandler::uid2user(m_uid);

    if (m_group.isEmpty())
        m_group = KRpermHandler::gid2group(m_gid);

    if (m_isDir && !m_isLink) {
        m_size = s_fileSizeCache.contains(m_url) ? s_fileSizeCache[m_url]->m_size : -1;
    }
}

FileItem *FileItem::createDummy()
{
    FileItem *file = new FileItem("..", QUrl(), true,
                            0, 0,
                            0, 0, 0);
    file->setIconName("go-up");
    return file;
}

FileItem *FileItem::createBroken(const QString &name, const QUrl &url)
{
    FileItem *file = new FileItem(name, url, false,
                                  0, 0, 0, 0, 0);
    file->setIconName("file-broken");
    return file;
}

FileItem *FileItem::createVirtualDir(const QString &name, const QUrl &url)
{
    return new FileItem(name, url, true,
                     0, 0700,
                     time(0), time(0), time(0),
                     getuid(), getgid());
}

FileItem *FileItem::createCopy(const FileItem &file, const QString &newName)
{
    return new FileItem(newName, file.getUrl(), file.isDir(),
                     file.getSize(), file.getMode(),
                     file.getTime_t(), file.getChangedTime(), file.getAccessTime(),
                     file.m_uid, file.m_gid, file.getOwner(), file.getGroup(),
                     file.isSymLink(), file.getSymDest(), file.isBrokenLink());
}

char FileItem::isReadable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KRpermHandler::readable(m_permissions, m_gid, m_uid);
    else
        return KRpermHandler::ftpReadable(m_owner, m_url.userName(), m_permissions);
}

char FileItem::isWriteable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KRpermHandler::writeable(m_permissions, m_gid, m_uid);
    else
        return KRpermHandler::ftpWriteable(m_owner, m_url.userName(), m_permissions);
}

char FileItem::isExecutable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KRpermHandler::executable(m_permissions, m_gid, m_uid);
    else
        return KRpermHandler::ftpExecutable(m_owner, m_url.userName(), m_permissions);
}

void FileItem::setSize(KIO::filesize_t size)
{
    m_size = size;
    s_fileSizeCache.insert(m_url, new FileSize(size));
}

const QString &FileItem::getMime()
{
    if (m_mimeType.isEmpty()) {
        if (m_isDir) {
            m_mimeType = "inode/directory";
            m_iconName = "inode-directory";
        } else if (isBrokenLink()) {
            m_mimeType = "unknown";
            m_iconName = "file-broken";
        } else {
            const QMimeDatabase db;
            const QMimeType mt = db.mimeTypeForUrl(getUrl());
            m_mimeType = mt.isValid() ? mt.name() : "unknown";
            m_iconName = mt.isValid() ? mt.iconName() : "file-broken";

            if (m_mimeType == "inode/directory") {
                // TODO view update needed? and does this ever happen?
                m_isDir = true;
            }
        }

        if (m_isDir && userDefinedFolderIcons) {
            const QUrl url = getUrl();
            if (url.isLocalFile()) {
                const QString file = url.toLocalFile() + "/.directory";
                const KDesktopFile cfg(file);
                const QString iconName = cfg.readIcon();
                if (!iconName.isEmpty())
                    m_iconName = iconName.startsWith(QLatin1String("./")) ?
                                     // relative path
                                     url.toLocalFile() + '/' + iconName :
                                     iconName;
            }
        }
    }
    return m_mimeType;
}

const QString &FileItem::getIcon()
{
    if (m_iconName.isEmpty()) {
        getMime(); // sets the icon
    }
    return m_iconName;
}

const QString &FileItem::getACL()
{
    if (!m_AclLoaded)
        loadACL();
    return m_acl;
}

void FileItem::loadACL()
{
    if (m_url.isLocalFile()) {
        FileSystemProvider::getACL(this, m_acl, m_defaulfAcl);
    }
    m_AclLoaded = true;
}

const KIO::UDSEntry FileItem::getEntry()
{
    KIO::UDSEntry entry;

    entry.insert(KIO::UDSEntry::UDS_NAME, getName());
    entry.insert(KIO::UDSEntry::UDS_SIZE, getSize());
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, getTime_t());
    entry.insert(KIO::UDSEntry::UDS_CREATION_TIME, getChangedTime());
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, getAccessTime());
    entry.insert(KIO::UDSEntry::UDS_USER, getOwner());
    entry.insert(KIO::UDSEntry::UDS_GROUP, getGroup());
    entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, getMime());
    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, getMode() & S_IFMT);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, getMode() & 07777);

    if (isSymLink())
        entry.insert(KIO::UDSEntry::UDS_LINK_DEST, getSymDest());

    if (!m_AclLoaded)
        loadACL();
    if (!m_acl.isNull() || !m_defaulfAcl.isNull()) {
        entry.insert(KIO::UDSEntry::UDS_EXTENDED_ACL, 1);

        if (!m_acl.isNull())
            entry.insert(KIO::UDSEntry::UDS_ACL_STRING, m_acl);

        if (!m_defaulfAcl.isNull())
            entry.insert(KIO::UDSEntry::UDS_DEFAULT_ACL_STRING, m_defaulfAcl);
    }

    return entry;
}
