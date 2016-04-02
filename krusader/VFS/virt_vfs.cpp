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

#include "virt_vfs.h"

#include <QtCore/QUrl>

#include <KI18n/KLocalizedString>
#include <KIOCore/KFileItem>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/StatJob>
#include <KIO/DirectorySizeJob>
#include <KWidgetsAddons/KMessageBox>

#include "krpermhandler.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../defaults.h"

#define VIRT_VFS_DB "virt_vfs.db"

QHash<QString, QList<QUrl> *> virt_vfs::virtVfsDict;
QHash<QString, QString> virt_vfs::metaInfoDict;
KConfig* virt_vfs::virt_vfs_db = 0;

virt_vfs::virt_vfs(QObject* panel, bool quiet) : vfs(panel, quiet)
{
    // set the writable attribute
    isWritable = true;

    if (virtVfsDict.isEmpty()) {
        restore();
    }

    vfs_type = VFS_VIRT;
}

virt_vfs::~virt_vfs() {}

bool virt_vfs::populateVfsList(const QUrl &origin, bool /*showHidden*/)
{
    vfs_origin = origin.adjusted(QUrl::StripTrailingSlash);
    path = origin.adjusted(QUrl::StripTrailingSlash).path().mid(1);
    if (path.isEmpty()) path = '/';

    QList<QUrl>* urlList;
    if (virtVfsDict.find(path) == virtVfsDict.end()) {
        urlList = new QList<QUrl>();
        virtVfsDict.insert(path, urlList);
        virtVfsDict[ "/" ] ->append(QUrl("virt:/" + path));
    } else {
        urlList = virtVfsDict[ path ];
        metaInfo = metaInfoDict[ path ];
    }

    save();
    if (urlList->isEmpty()) return true;
    QList<QUrl>::iterator it;
    for (it = urlList->begin() ; it != urlList->end() ; /*++it*/) {
        QUrl url = *it;
        // translate url->vfile and remove urls that no longer exist from the list
        vfile* vf = stat(url);
        if (!vf) {
            it = urlList->erase(it);
            // the iterator is advanced automatically
            continue;
        }
        foundVfile(vf);
        ++it;
    }
    return true;
}

void virt_vfs::vfs_addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode /*mode*/, QObject* /*toNotify*/, QString /*dir*/, PreserveMode /*pmode*/)
{
    if (path == "/") {
        if (!quietMode)
            KMessageBox::error(parentWindow, i18n("You cannot copy files directly to the 'virt:/' directory.\nYou can create a sub directory and copy your files into it."), i18n("Error"));
        return ;
    }

    if (virtVfsDict.find(path) == virtVfsDict.end()) {
        QList<QUrl> *urlList = new QList<QUrl>();
        virtVfsDict.insert(path, urlList);
        virtVfsDict[ "/" ] ->append(QUrl("virt:/" + path));
    }
    QList<QUrl>* urlList = virtVfsDict[ path ];
    foreach (const QUrl &fileUrl, fileUrls) {
        if (!urlList->contains(fileUrl))
            urlList->push_back(fileUrl);
    }

    vfs_refresh();
}

void virt_vfs::vfs_delFiles(const QStringList &fileNames, bool reallyDelete)
{
    if (path == "/") {
        foreach (const QString &filename, fileNames) {
            virtVfsDict[ "/" ] ->removeAll(QUrl(QStringLiteral("virt:/") + filename));
            delete virtVfsDict[ filename ];
            virtVfsDict.remove(filename);
            metaInfoDict.remove(filename);
        }
        vfs_refresh();
        return ;
    }

    // names -> urls
    QList<QUrl> filesUrls = vfs_getFiles(fileNames);

    KIO::Job *job;

    // delete of move to trash ?
    KConfigGroup group(krConfig, "General");
    if (!reallyDelete && group.readEntry("Move To Trash", _MoveToTrash)) {
        job = KIO::trash(filesUrls);
        if(parentWindow)
            emit trashJobStarted(job);
    } else
        job = KIO::del(filesUrls);

    // refresh will remove the deleted files...
    connect(job, SIGNAL(result(KJob*)), this, SLOT(vfs_refresh(KJob*)));
}

void virt_vfs::vfs_removeFiles(QStringList *fileNames)
{
    if (path == "/")
        return;

    // removing the URLs from the collection
    for (int i = 0 ; i < fileNames->count(); ++i) {
        if (virtVfsDict.find(path) != virtVfsDict.end()) {
            QList<QUrl>* urlList = virtVfsDict[ path ];
            urlList->removeAll(vfs_getFile((*fileNames)[ i ]));
        }
    }

    vfs_refresh();
}

QList<QUrl> virt_vfs::vfs_getFiles(const QStringList &names)
{
    QUrl url;
    QList<QUrl> urls;
    foreach (const QString &name, names) {
        urls.append(vfs_getFile(name));
    }
    return urls;
}

QUrl virt_vfs::vfs_getFile(const QString& name)
{
    vfile * vf = vfs_search(name);
    if (!vf) return QUrl();   // empty

    return vf->vfile_getUrl();
}

void virt_vfs::vfs_mkdir(const QString& name)
{
    if (path != "/") {
        if (!quietMode)
            KMessageBox::error(parentWindow, i18n("Creating new directories is allowed only in the 'virt:/' directory."), i18n("Error"));
        return ;
    }
    QList<QUrl>* temp = new QList<QUrl>();
    virtVfsDict.insert(name, temp);
    virtVfsDict[ "/" ] ->append(QUrl(QStringLiteral("virt:/") + name));

    vfs_refresh();
}

void virt_vfs::vfs_rename(const QString& fileName, const QString& newName)
{
    vfile* vf = vfs_search(fileName);
    if (!vf) return ;   // not found

    if (path == "/") {
        virtVfsDict[ "/" ] ->append(QUrl(QStringLiteral("virt:/") + newName));
        virtVfsDict[ "/" ] ->removeAll(QUrl(QStringLiteral("virt:/") + fileName));
        virtVfsDict.insert(newName, virtVfsDict.take(fileName));
        vfs_refresh();
        return ;
    }

    QUrl url = vf->vfile_getUrl();

    QList<QUrl> fileUrls;
    fileUrls.append(url);

    QUrl dest = url; // keep same url scheme and host as source.
    dest.setPath(newName);
    // add the new url to the list
    // the list is refreshed only existing files remain -
    // so we don't have to worry if the job was successful
    virtVfsDict[ path ] ->append(dest);

    KIO::Job *job = KIO::move(fileUrls, dest);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(vfs_refresh(KJob*)));
}

void virt_vfs::slotStatResult(KJob* job)
{
    if (!job || job->error()) entry = KIO::UDSEntry();
    else entry = static_cast<KIO::StatJob*>(job)->statResult();
    busy = false;
}

vfile* virt_vfs::stat(const QUrl &url)
{
    if (url.scheme() == "virt") {
        QString path = url.path().mid(1);
        if (path.isEmpty()) path = '/';
        vfile * temp = new vfile(path, 0, "drwxr-xr-x", time(0), false, false, getuid(), getgid(), "inode/directory", "", 0);
        temp->vfile_setUrl(url);
        return temp;
    }
    KFileItem* kfi;
    if (url.isLocalFile()) {
        kfi = new KFileItem(url);
        kfi->setDelayedMimeTypes(true);
    } else {
        busy = true;
        KIO::StatJob* statJob = KIO::stat(url, KIO::HideProgressInfo);
        connect(statJob, SIGNAL(result(KJob*)), this, SLOT(slotStatResult(KJob*)));
        while (busy && vfs_processEvents());
        if (entry.count() == 0) return 0;  // statJob failed

        kfi = new KFileItem(entry, url, true);
    }

    if (kfi->time(KFileItem::ModificationTime).isNull()) {
        delete kfi;
        return 0; // file not found
    }

    vfile *temp;

    // get file statistics
    QString name;
    if (url.isLocalFile())
        name = url.path();
    else
        name = url.toDisplayString();

    KIO::filesize_t size = kfi->size();
    time_t mtime = kfi->time(KFileItem::ModificationTime).toTime_t();
    bool symLink = kfi->isLink();
    mode_t mode = kfi->mode() | kfi->permissions();
    QString perm = KRpermHandler::mode2QString(mode);
    // set the mimetype
    QString mime;
    QString symDest;
    if (symLink) {
        symDest = kfi->linkDest();
        if (kfi->isDir()) perm[ 0 ] = 'd';
    }

    // create a new virtual file object
    if (kfi->user().isEmpty())
        temp = new vfile(name, size, perm, mtime, symLink, false, getuid(), getgid(), mime, symDest, mode);
    else {
        QString currentUser = url.userName();
        if (currentUser.contains("@"))      /* remove the FTP proxy tags from the username */
            currentUser.truncate(currentUser.indexOf('@'));
        if (currentUser.isEmpty())
            currentUser = KRpermHandler::uid2user(getuid());
        temp = new vfile(name, size, perm, mtime, symLink, false, kfi->user(), kfi->group(), currentUser, mime, symDest, mode);
    }

    temp->vfile_setUrl(kfi->url());
    delete kfi;
    return temp;
}

KConfig*  virt_vfs::getVirtDB()
{
    if (!virt_vfs_db) {
//  virt_vfs_db = new KConfig("data",VIRT_VFS_DB,KConfig::NoGlobals);
        virt_vfs_db = new KConfig(VIRT_VFS_DB, KConfig:: CascadeConfig, QStandardPaths::DataLocation);
    }
    return virt_vfs_db;
}

bool virt_vfs::save()
{
    KConfig* db = getVirtDB();
    db->deleteGroup("virt_db");
    KConfigGroup group(db, "virt_db");

    QHashIterator< QString, QList<QUrl> *> lit(virtVfsDict);
    while (lit.hasNext()) {
        QList<QUrl> * curr = lit.next().value();

        QList<QUrl>::iterator url;
        QStringList entry;
        for (url = curr->begin() ; url != curr->end() ; ++url) {
            entry.append((*url).toDisplayString());
        }
        // KDE 4.0 workaround, Item_ is added as KConfig fails on 1 char names (such as /)
        group.writeEntry("Item_" + lit.key(), entry);
        group.writeEntry("MetaInfo_" + lit.key(), metaInfoDict[lit.key()]);
    }

    db->sync();

    return true;
}

bool virt_vfs::restore()
{
    KConfig* db = getVirtDB();
    KConfigGroup dbGrp(db, "virt_db");

    QMap<QString, QString> map = db->entryMap("virt_db");
    QMap<QString, QString>::Iterator it;
    QList<QUrl>* urlList;
    for (it = map.begin(); it != map.end(); ++it) {
        if(!it.key().startsWith(QLatin1String("Item_")))
            continue;
        urlList = new QList<QUrl>(KrServices::toUrlList(dbGrp.readEntry(it.key(), QStringList())));
        // KDE 4.0 workaround, Item_ is removed (KConfig fails on 1 char names (such as /))
        QString key = it.key().mid(5);   // remove Item_
        virtVfsDict.insert(key, urlList);
        metaInfoDict.insert(key, dbGrp.readEntry("MetaInfo_" + key, QString()));
    }

    if (!virtVfsDict["/" ]) {
        urlList = new QList<QUrl>();
        virtVfsDict.insert("/", urlList);
    }

    return true;
}

void virt_vfs::vfs_calcSpace(QString name , KIO::filesize_t* totalSize, unsigned long* totalFiles, unsigned long* totalDirs, bool* stop)
{
    if (stop && *stop) return ;
    if (path == "/") {
        QList<QUrl>* urlList = virtVfsDict[ name ];
        if (urlList)
            for (int i = 0; (i != urlList->size()) && !(*stop); i++)
                calculateURLSize((*urlList)[ i ], totalSize, totalFiles, totalDirs, stop);
        return;
    }
    return vfs::vfs_calcSpace(name, totalSize, totalFiles, totalDirs, stop);
}

void virt_vfs::setMetaInformation(QString info) {
    metaInfo = info;
    metaInfoDict[path] = metaInfo;
    vfs_refresh();
}

