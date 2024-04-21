/*
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fileitem.h"

// QtCore
#include <QCache>
#include <QDateTime>
#include <QMimeDatabase>
#include <QMimeType>

#include <KDesktopFile>

#include "../compat.h"
#include "filesystemprovider.h"
#include "krpermhandler.h"

bool FileItem::userDefinedFolderIcons = true;

// wrapper class; QCache needs objects
class FileSize
{
public:
    const KIO::filesize_t m_size;
    FileSize(KIO::filesize_t size)
        : m_size(size)
    {
    }
};

// cache for calculated directory sizes;
static QCache<const QUrl, FileSize> s_fileSizeCache(1000);

FileItem::FileItem(const QString &name,
                   const QUrl &url,
                   bool isDir,
                   KIO::filesize_t size,
                   mode_t mode,
                   time_t mtime,
                   time_t ctime,
                   time_t atime,
                   time_t btime,
                   uid_t uid,
                   gid_t gid,
                   const QString &owner,
                   const QString &group,
                   bool isLink,
                   const QString &linkDest,
                   bool isBrokenLink,
                   const QString &acl,
                   const QString &defaultAcl)
    : m_name(name)
    , m_url(url)
    , m_isDir(isDir)
    , m_size(size)
    , m_mode(mode)
    , m_mtime(mtime)
    , m_ctime(ctime)
    , m_atime(atime)
    , m_btime(btime)
    , m_uid(uid)
    , m_gid(gid)
    , m_owner(owner)
    , m_group(group)
    , m_isLink(isLink)
    , m_linkDest(linkDest)
    , m_isBrokenLink(isBrokenLink)
    , m_acl(acl)
    , m_defaulfAcl(defaultAcl)
    , m_AclLoaded(false)
    , m_mimeType()
    , m_iconName()
{
    m_permissions = KrPermHandler::mode2QString(mode);

    if (m_owner.isEmpty())
        m_owner = KrPermHandler::uid2user(m_uid);

    if (m_group.isEmpty())
        m_group = KrPermHandler::gid2group(m_gid);

    if (m_isDir && !m_isLink) {
        m_size = s_fileSizeCache.contains(m_url) ? s_fileSizeCache[m_url]->m_size : -1;
    }
}

FileItem *FileItem::createDummy()
{
    FileItem *file = new FileItem("..", QUrl(), true, 0, 0, -1, -1, -1, -1);
    file->setIconName("go-up");
    return file;
}

FileItem *FileItem::createBroken(const QString &name, const QUrl &url)
{
    FileItem *file = new FileItem(name, url, false, 0, 0, -1, -1, -1, -1);
    file->setIconName("file-broken");
    return file;
}

FileItem *FileItem::createVirtualDir(const QString &name, const QUrl &url)
{
    return new FileItem(name, url, true, 0, 0700, -1, -1, -1, -1, getuid(), getgid());
}

FileItem *FileItem::createCopy(const FileItem &file, const QString &newName)
{
    return new FileItem(newName,
                        file.getUrl(),
                        file.isDir(),
                        file.getSize(),
                        file.getMode(),
                        file.getModificationTime(),
                        file.getChangeTime(),
                        file.getAccessTime(),
                        file.getCreationTime(),
                        file.m_uid,
                        file.m_gid,
                        file.getOwner(),
                        file.getGroup(),
                        file.isSymLink(),
                        file.getSymDest(),
                        file.isBrokenLink());
}

char FileItem::isReadable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KrPermHandler::readable(m_permissions, m_gid, m_uid);
    else
        return KrPermHandler::ftpReadable(m_owner, m_url.userName(), m_permissions);
}

char FileItem::isWriteable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KrPermHandler::writeable(m_permissions, m_gid, m_uid);
    else
        return KrPermHandler::ftpWriteable(m_owner, m_url.userName(), m_permissions);
}

char FileItem::isExecutable() const
{
    if (m_uid != (uid_t)-1 && m_gid != (gid_t)-1)
        return KrPermHandler::executable(m_permissions, m_gid, m_uid);
    else
        return KrPermHandler::ftpExecutable(m_owner, m_url.userName(), m_permissions);
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
                        url.toLocalFile() + '/' + iconName
                                                                          : iconName;
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

    entry.fastInsert(KIO::UDSEntry::UDS_NAME, getName());
    entry.fastInsert(KIO::UDSEntry::UDS_SIZE, getSize());
    entry.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, getModificationTime());
    if (m_btime != -1) {
        entry.fastInsert(KIO::UDSEntry::UDS_CREATION_TIME, getCreationTime());
    }
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS_TIME, getAccessTime());
    entry.fastInsert(KIO::UDSEntry::UDS_USER, getOwner());
    entry.fastInsert(KIO::UDSEntry::UDS_GROUP, getGroup());
    entry.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, getMime());
    entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, getMode() & S_IFMT);
    entry.fastInsert(KIO::UDSEntry::UDS_ACCESS, getMode() & 07777);

    if (isSymLink())
        entry.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, getSymDest());

    if (!m_AclLoaded)
        loadACL();
    if (!m_acl.isNull() || !m_defaulfAcl.isNull()) {
        entry.fastInsert(KIO::UDSEntry::UDS_EXTENDED_ACL, 1);

        if (!m_acl.isNull())
            entry.fastInsert(KIO::UDSEntry::UDS_ACL_STRING, m_acl);

        if (!m_defaulfAcl.isNull())
            entry.fastInsert(KIO::UDSEntry::UDS_DEFAULT_ACL_STRING, m_defaulfAcl);
    }

    return entry;
}
