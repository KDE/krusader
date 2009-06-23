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

#include <QtCore/QObject>

#include <kurl.h>

#include "vfs.h"

class KrVfsHandler : public QObject
{
    Q_OBJECT
public:
    KrVfsHandler();
    ~KrVfsHandler();

    static vfs::VFS_TYPE getVfsType(const KUrl& url);
    static vfs* getVfs(const KUrl& url, QObject* parent = 0, vfs* oldVfs = 0);
};

#endif

