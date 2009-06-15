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

#include "virt.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QFile>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>

using namespace KIO;

#define VIRT_VFS_DB "virt_vfs.db"
#define VIRT_PROTOCOL "virt"

extern "C"
{
    int KDE_EXPORT kdemain(int argc, char **argv);
}

#define KrDEBUG(X...){\
        FILE* f = fopen("/tmp/kio_virt.log","a+");\
        fprintf(f,X);\
        fclose(f);\
    }

QDict<KUrl::List> VirtProtocol::kioVirtDict;
KConfig* VirtProtocol::kio_virt_db;

int kdemain(int argc, char **argv)
{
    KInstance instance("kio_virt");

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_virt protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    VirtProtocol slave(argv[ 2 ], argv[ 3 ]);
    slave.dispatchLoop();

    return 0;
}

VirtProtocol::VirtProtocol(const QCString &pool, const QCString &app) : SlaveBase("virt", pool, app)
{
    kio_virt_db = new KConfig(VIRT_VFS_DB, false, "data");
}

VirtProtocol::~VirtProtocol()
{
    delete kio_virt_db;
}

void VirtProtocol::del(KUrl const & /*url */, bool /* isFile */)
{
// KRDEBUG(url.path());

    messageBox(KIO::SlaveBase::QuestionYesNo,
               i18n(""),
               i18n("Virtulal delete"),
               i18n("remove from virtual space"),
               i18n("really delete")
              );

    finished();
}

void VirtProtocol::copy(const KUrl &src, const KUrl &dest, int /* permissions */, bool /* overwrite */)
{
    QString path = dest.path(KUrl::RemoveTrailingSlash).mid(1);
    path = path.left(path.lastIndexOf("/"));
    if (path.isEmpty()) path = '/';

    if (addDir(path)) {
        kioVirtDict[ path ]->append(src);
        save();
    }

    finished();
}

bool VirtProtocol::addDir(QString& path)
{

    if (kioVirtDict[ path ]) return true;

    QString updir;
    if (!path.contains("/")) updir = '/';
    else updir = path.left(path.lastIndexOf("/"));
    QString name = path.mid(path.lastIndexOf("/") + 1);

    if (addDir(updir)) {
        KUrl url;
        if (updir == "/") url = QString("virt:/") + name;
        else url = QString("virt:/") + updir + '/' + name;
        kioVirtDict[ updir ]->append(url);

        KUrl::List* temp = new KUrl::List();
        kioVirtDict.replace(path, temp);

        return true;
    }
    return false;
}

void VirtProtocol::mkdir(const KUrl& url, int)
{
    if (url.protocol() != VIRT_PROTOCOL) {
        redirection(url);
        finished();
        return;
    }

    QString path = url.path(KUrl::RemoveTrailingSlash).mid(1);
    if (path.isEmpty()) path = '/';

    if (kioVirtDict[ path ]) {
        error(KIO::ERR_DIR_ALREADY_EXIST, url.path());
        return;
    }

    addDir(path);

    save();

    finished();
}

void VirtProtocol::listDir(const KUrl & url)
{
    if (url.protocol() != VIRT_PROTOCOL) {
        redirection(url);
        finished();
        return;
    }

    load();

    QString path = url.path(KUrl::RemoveTrailingSlash).mid(1);
    if (path.isEmpty()) path = '/';

    KUrl::List* urlList = kioVirtDict[ path ];
    if (!urlList) {
        error(ERR_CANNOT_ENTER_DIRECTORY, url.path());
        return;
    }

    UDSEntryList dirList;
    KUrl::List::iterator it;
    for (it = urlList->begin() ; it != urlList->end() ; ++it) {
        KUrl entry_url = *it;
        // translate url->UDS_ENTRY
        UDSEntry entry;
        if (entry_url.protocol() == VIRT_PROTOCOL) {
            local_entry(entry_url, entry);
        } else {
            UDSAtom atom;

            atom.m_uds = UDS_NAME;
            atom.m_str = url.isLocalFile() ? url.path() : entry_url.prettyUrl();
            entry.append(atom);

            atom.m_uds = UDS_URL;
            atom.m_str = entry_url.url();
            entry.append(atom);
        }

        dirList.append(entry);
    }

    totalSize(dirList.size());
    listEntries(dirList);

    finished();
}

void VirtProtocol::stat(const KUrl & url)
{
    if (url.protocol() != VIRT_PROTOCOL) {
        redirection(url);
        finished();
        return;
    }

    UDSEntry entry;
    local_entry(url, entry);

    statEntry(entry);

    finished();
}

void VirtProtocol::get(const KUrl & url)
{
    if (url.protocol() != VIRT_PROTOCOL) {
        redirection(url);
        finished();
        return;
    }

    finished();
}

bool VirtProtocol::rewriteURL(const KUrl& /* src */, KUrl&)
{
    return true;
}

bool VirtProtocol::save()
{
    lock();

    KConfig* db = new KConfig(VIRT_VFS_DB, false, "data");;

    db->setGroup("virt_db");
    QDictIterator<KUrl::List> it(kioVirtDict);   // See QDictIterator
    for (; it.current(); ++it) {
        KUrl::List::iterator url;
        QStringList entry;
        for (url = it.current()->begin() ; url != it.current()->end() ; ++url) {
            entry.append((*url).url());
        }
        db->writeEntry(it.currentKey(), entry);
    }

    db->sync();
    delete(db);

    unlock();

    return true;
}

bool VirtProtocol::load()
{
    lock();

    KConfig* db = new KConfig(VIRT_VFS_DB, false, "data");
    db->setGroup("virt_db");

    QMap<QString, QString> map = db->entryMap("virt_db");
    QMap<QString, QString>::Iterator it;
    KUrl::List* urlList;
    for (it = map.begin(); it != map.end(); ++it) {
        urlList = new KUrl::List(db->readListEntry(it.key()));
        kioVirtDict.replace(it.key(), urlList);
    }

    if (!kioVirtDict["/" ]) {
        urlList = new KUrl::List();
        kioVirtDict.replace('/', urlList);
    }

    unlock();

    delete(db);

    return true;
}

void VirtProtocol::local_entry(const KUrl& url, UDSEntry& entry)
{
    QString path = url.path(KUrl::RemoveTrailingSlash).mid(1);
    if (path.isEmpty()) path = '/';

    UDSAtom atom;

    atom.m_uds = UDS_NAME;
    atom.m_str = url.fileName();
    entry.append(atom);

    atom.m_uds = UDS_URL;
    atom.m_str = url.url();
    entry.append(atom);

    atom.m_uds = UDS_FILE_TYPE;
    atom.m_long = S_IFDIR;
    entry.append(atom);

    atom.m_uds = UDS_ACCESS;
    atom.m_long = 0700;
    entry.append(atom);

    atom.m_uds = UDS_MIME_TYPE;
    atom.m_str = "inode/system_directory";
    entry.append(atom);
}

bool VirtProtocol::lock()
{
    return true;
}

bool VirtProtocol::unlock()
{
    return true;
}
