/***************************************************************************
                          vfile.h
                      -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef VFILE_H
#define VFILE_H

#include <sys/types.h>

// QtCore
#include <QString>
#include <QUrl>

#include <KIO/Global>
#include <KIO/UDSEntry>

/**
 * The Virtual File class handles all the details of maintaining a single
 * file component within the virtual file system (filesystem). a vfile object
 * contains the necessary details about a file and member functions which
 * allow the object to give out the needed details about the file.
 */
class vfile
{

public:
    /**
     * Create a new file item.
     *
     * Don't use this constructor outside of FileSystem! If you really need to, create a new static
     * factory method below.
     *
     * NOTE: According to Unix standard uid and gid CAN have signed or unsigned type. We use (e.g.)
     * "(uid_t) -1" as a special invalid user ID for non-local files.
     * Note: ACLs are currently only used by Synchronizer.
     *
     * @param name the display name of this file. Don't have to be the real filename.
     * @param url (real) absolute URL of this file
     * @param isDir true if this file is a directory. Else false.
     * @param size size of file
     * @param mode mode of file (file type and permissions)
     * @param mtime file modification time
     * @param uid Unix user id of file owner. Use -1 here and provide an owner name for non-local files.
     * @param gid Unix group id of file group. Use -1 here and provide a group name for non-local files.
     * @param owner user name of file owner. Can be empty for local files
     * @param group group name of file group. Cam be empty for local files.
     * @param isLink true if file is a symbolic link. Else false.
     * @param linkDest link destination path if file is a link. Relative or absolute. Empty by default.
     * @param isBrokenLink true if file is a symbolic link and destination file does not exists. Else false.
     * @param acl ACL string of file. Can be empty and is loaded on demand.
     * @param defaultAcl default ACL string of file (only for directories). Can be empty and is loaded on demand.
     */
    vfile(const QString &name, const QUrl &url, bool isDir,
          KIO::filesize_t size, mode_t mode, time_t mtime,
          uid_t uid = -1, gid_t gid = -1,
          const QString &owner = QString(), const QString &group = QString(),
          bool isLink = false, const QString &linkDest = QString(), bool isBrokenLink = false,
          const QString &acl = QString(), const QString &defaultAcl = QString());

    /** Create a new ".." dummy file item. */
    static vfile *createDummy();
    /** Create a new virtual directory. */
    static vfile *createVirtualDir(const QString &name, const QUrl &url);
    /** Create a new file item copy with a different name. */
    static vfile *createCopy(const vfile &file, const QString &newName);

    // following functions give-out file details
    inline const QString &vfile_getName() const { return m_name; }
    inline KIO::filesize_t vfile_getSize() const { return m_size; }
    inline const QString &vfile_getPerm() const { return m_permissions; }
    inline bool vfile_isDir() const { return m_isDir; }
    inline bool vfile_isSymLink() const { return m_isLink; }
    inline bool vfile_isBrokenLink() const { return m_isBrokenLink; }
    inline const QString &vfile_getSymDest() const { return m_linkDest; }
    inline mode_t vfile_getMode() const { return m_mode; }
    inline time_t vfile_getTime_t() const { return m_mtime; }
    inline const QUrl &vfile_getUrl() const { return m_url; }
    inline const QString &vfile_getOwner() const { return m_owner; }
    inline const QString &vfile_getGroup() const { return m_group; }

    const QString &vfile_getMime();
    const QString &vfile_getIcon();

    const QString &vfile_getACL();
    const QString &vfile_getDefaultACL();
    const KIO::UDSEntry vfile_getEntry(); //< return the UDSEntry from the vfile
    char vfile_isReadable() const;
    char vfile_isWriteable() const;
    char vfile_isExecutable() const;
    /**
     * Set the file size.
     * used ONLY when calculating a directory's space, needs to change the
     * displayed size of the viewitem and thus the vfile. For INTERNAL USE !
     */
    void vfile_setSize(KIO::filesize_t size) { m_size = size; }

    inline static void vfile_loadUserDefinedFolderIcons(bool load) {
        vfile_userDefinedFolderIcons = load;
    }

private:
    void vfile_setIcon(const QString &icon) { m_icon = icon; m_mimeType = "?"; }
    void vfile_loadACL();

    QString m_name;             //< file name
    QUrl m_url;                 //< file URL
    bool m_isDir;               //< flag, true if it's a directory

    KIO::filesize_t m_size;     //< file size
    mode_t m_mode;              //< file mode (file type and permissions)
    time_t m_mtime;             //< file modification time

    uid_t m_uid;                //< file owner id
    gid_t m_gid;                //< file group id
    QString m_owner;            //< file owner name
    QString m_group;            //< file group name

    bool m_isLink;              //< true if the file is a symlink
    QString m_linkDest;         //< if it's a sym link - its detination
    bool m_isBrokenLink;        //< true if the link destianation does not exist

    QString m_permissions;      //< file permissions string

    QString m_acl;              //< ACL permission string, may lazy initialized
    QString m_defaulfAcl;       //< ACL default string, may lazy initialized
    bool m_AclLoaded;           //< flag, indicates that ACL permissions already loaded

    QString m_mimeType;         //< file mimetype, lazy initialized
    QString m_icon;             //< the name of the icon file, lazy initialized

    static bool vfile_userDefinedFolderIcons;
};

#endif
