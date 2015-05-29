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

#ifndef VIRT_H
#define VIRT_H

#include <sys/types.h>

#include <QtCore/QHash>
#include <QtCore/QByteArray>

#include <KConfigCore/KConfig>
#include <KIO/SlaveBase>

class VirtProtocol : public KIO::SlaveBase
{
public:
    VirtProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~VirtProtocol();

    virtual void listDir(const QUrl &url);
    virtual void stat(const QUrl &url);
    virtual void get(const QUrl &url);
    virtual void mkdir(const QUrl &url, int permissions);
    virtual void copy(const QUrl &src, const QUrl &dest, int permissions, bool overwrite);
    virtual void del(QUrl const & url, bool isFile);

protected:
    bool lock();
    bool unlock();
    bool save();
    bool load();

    void local_entry(const QUrl &url, KIO::UDSEntry& entry);
    bool addDir(QString& path);


    static QHash<QString, QList<QUrl>*> kioVirtDict;
    static KConfig* kio_virt_db;

    bool rewriteURL(const QUrl&, QUrl&);

};

#endif
