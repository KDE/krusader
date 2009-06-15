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
#include "normal_vfs.h"
#include "ftp_vfs.h"
#include "virt_vfs.h"
#include "../krservices.h"

#include <QtCore/QDir>

#include <kdebug.h>


KrVfsHandler::KrVfsHandler()
{
}
KrVfsHandler::~KrVfsHandler()
{
}

vfs::VFS_TYPE KrVfsHandler::getVfsType(const KUrl& url)
{
    QString protocol = url.protocol();

    if ((protocol == "krarc" || protocol == "tar" || protocol == "zip") &&
            QDir(KrServices::getPath(url, KUrl::RemoveTrailingSlash)).exists())
        return vfs::VFS_NORMAL;

    if (url.isLocalFile()) {
        return vfs::VFS_NORMAL;
    } else {
        if (url.protocol() == "virt") return vfs::VFS_VIRT;
        else return vfs::VFS_FTP;
    }
    return vfs::VFS_ERROR;
}

vfs* KrVfsHandler::getVfs(const KUrl& url, QObject* parent, vfs* oldVfs)
{
    vfs::VFS_TYPE newType, oldType = vfs::VFS_ERROR;

    if (oldVfs) oldType = oldVfs->vfs_getType();
    newType = getVfsType(url);


    vfs* newVfs = oldVfs;

    if (oldType != newType) {
        switch (newType) {
        case(vfs::VFS_NORMAL) : newVfs = new normal_vfs(parent); break;
        case(vfs::VFS_FTP) : newVfs = new ftp_vfs(parent)   ; break;
        case(vfs::VFS_VIRT) : newVfs = new virt_vfs(parent)  ; break;
        case(vfs::VFS_ERROR) : newVfs = 0                     ; break;
        }
    }

    return newVfs;
}
