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

// QtCore
#include <QDir>
#include <QEventLoop>
#include <QUrl>
// QtWidgets
#include <QApplication>

#include <KConfigCore/KConfig>
#include <KCoreAddons/KUrlMimeData>
#include <KI18n/KLocalizedString>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/DirectorySizeJob>
#include <KIO/StatJob>
#include <KIOCore/KFileItem>
#include <KWidgetsAddons/KMessageBox>

#include "../defaults.h"
#include "../krglobal.h"
#include "../krservices.h"

#define VIRT_VFS_DB "virt_vfs.db"

QHash<QString, QList<QUrl> *> virt_vfs::_virtVfsDict;
QHash<QString, QString> virt_vfs::_metaInfoDict;

virt_vfs::virt_vfs() : vfs()
{
    if (_virtVfsDict.isEmpty()) {
        restore();
    }

    _type = VFS_VIRT;
}

void virt_vfs::copyFiles(const QList<QUrl> &urls, const QUrl &destination,
                         KIO::CopyJob::CopyMode /*mode*/, bool /*showProgressInfo*/,
                         bool /*reverseQueueMode*/, bool /*startPaused*/)
{
    const QString dir = QDir(destination.path()).absolutePath().remove('/');

    if (dir.isEmpty()) {
        showError(i18n("You cannot copy files directly to the 'virt:/' folder.\n"
                       "You can create a sub folder and copy your files into it."));
        return;
    }

    if (!_virtVfsDict.contains(dir)) {
        mkDirInternal(dir);
    }

    QList<QUrl> *urlList = _virtVfsDict[dir];
    for (const QUrl &fileUrl : urls) {
        if (!urlList->contains(fileUrl)) {
            urlList->push_back(fileUrl);
        }
    }

    emit filesystemChanged(QUrl("virt:///" + dir)); // may call refresh()
}

void virt_vfs::dropFiles(const QUrl &destination, QDropEvent *event)
{
    const QList<QUrl> &urls = KUrlMimeData::urlsFromMimeData(event->mimeData());
    // dropping on virtual vfs (sic!) is always copy operation
    copyFiles(urls, destination);
}

void virt_vfs::addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode /*mode*/, QString dir)
{
    QUrl destination(_currentDirectory);
    if (!dir.isEmpty()) {
        destination.setPath(QDir::cleanPath(destination.path() + '/' + dir));
    }
    copyFiles(fileUrls, destination);
}

void virt_vfs::remove(const QStringList &fileNames)
{
    const QString parentDir = currentDir();
    if (parentDir == "/") { // remove virtual directory
        for (const QString &filename : fileNames) {
            _virtVfsDict["/"]->removeAll(QUrl(QStringLiteral("virt:/") + filename));
            delete _virtVfsDict[filename];
            _virtVfsDict.remove(filename);
            _metaInfoDict.remove(filename);
        }
    } else {
        // remove the URLs from the collection
        for (const QString name : fileNames) {
            if (_virtVfsDict.find(parentDir) != _virtVfsDict.end()) {
                QList<QUrl> *urlList = _virtVfsDict[parentDir];
                urlList->removeAll(getUrl(name));
            }
        }
    }

    emit filesystemChanged(currentDirectory()); // will call refresh()
}

QUrl virt_vfs::getUrl(const QString &name)
{
    vfile *vf = getVfile(name);
    if (!vf) {
        return QUrl(); // not found
    }

    return vf->vfile_getUrl();
}

void virt_vfs::mkDir(const QString &name)
{
    if (currentDir() != "/") {
        showError(i18n("Creating new folders is allowed only in the 'virt:/' folder."));
        return;
    }

    mkDirInternal(name);

    emit filesystemChanged(currentDirectory()); // will call refresh()
}

void virt_vfs::rename(const QString &fileName, const QString &newName)
{
    vfile *vf = getVfile(fileName);
    if (!vf)
        return; // not found

    if (currentDir() == "/") { // rename virtual directory
        _virtVfsDict["/"]->append(QUrl(QStringLiteral("virt:/") + newName));
        _virtVfsDict["/"]->removeAll(QUrl(QStringLiteral("virt:/") + fileName));
        _virtVfsDict.insert(newName, _virtVfsDict.take(fileName));
        refresh();
        return;
    }

    // newName can be a (local) path or a full url
    QUrl dest(newName);
    if (dest.scheme().isEmpty())
        dest.setScheme("file");

    // add the new url to the list
    // the list is refreshed, only existing files remain -
    // so we don't have to worry if the job was successful
    _virtVfsDict[currentDir()]->append(dest);

    KIO::Job *job = KIO::moveAs(vf->vfile_getUrl(), dest, KIO::HideProgressInfo);
    connect(job, &KIO::Job::result, this, [=](KJob* job) { slotJobResult(job, false); });
    connect(job, &KIO::Job::result, [=]() { emit filesystemChanged(currentDirectory()); });
}

void virt_vfs::calcSpace(const QString &name, KIO::filesize_t *totalSize, unsigned long *totalFiles,
                         unsigned long *totalDirs, bool *stop)
{
    if (currentDir() == "/") {
        if (!_virtVfsDict.contains(name)) {
            return; // virtual folder not found
        }

        const QList<QUrl> *urlList = _virtVfsDict[name];
        if (urlList) {
            for (int i = 0; (i != urlList->size()) && !(*stop); i++) {
                vfs::calcSpace((*urlList)[i], totalSize, totalFiles, totalDirs, stop);
            }
        }
        return;
    }

    vfs::calcSpace(name, totalSize, totalFiles, totalDirs, stop);
}

bool virt_vfs::canMoveToTrash(const QStringList &fileNames)
{
    if (isRoot())
        return false;

    for (const QString fileName : fileNames) {
        if (!getUrl(fileName).isLocalFile()) {
            return false;
        }
    }
    return true;
}

void virt_vfs::setMetaInformation(const QString &info)
{
    _metaInfoDict[currentDir()] = info;
}

// ==== protected ====

bool virt_vfs::refreshInternal(const QUrl &directory, bool /*showHidden*/)
{
    _currentDirectory = cleanUrl(directory);
    _currentDirectory.setHost("");
    // remove invalid subdirectories
    _currentDirectory.setPath("/" + _currentDirectory.path().remove('/'));

    if (!_virtVfsDict.contains(currentDir())) {
        // NOTE: silently creating non-existing directories here. The search and locate tools expect
        // this. (And user can enter some directory and it will be created).
        mkDirInternal(currentDir());
        save();
        // infinite loop possible
        //emit filesystemChanged(currentDirectory());
        return true;
    }

    QList<QUrl> *urlList = _virtVfsDict[currentDir()];

    const QString metaInfo = _metaInfoDict[currentDir()];
    emit filesystemInfoChanged(metaInfo.isEmpty() ? i18n("Virtual filesystem") : metaInfo, "", 0, 0);

    QMutableListIterator<QUrl> it(*urlList);
    while (it.hasNext()) {
        const QUrl url = it.next();
        vfile *vf = createVFile(url);
        if (!vf) { // remove URL from the list for a file that no longer exists
            it.remove();
        } else {
            addVfile(vf);
        }
    }

    save();
    return true;
}

// ==== private ====

void virt_vfs::mkDirInternal(const QString &name)
{
    // clean path, consistent with currentDir()
    QString dirName = name;
    dirName = dirName.remove('/');
    if (dirName.isEmpty())
        dirName = "/";

    _virtVfsDict.insert(dirName, new QList<QUrl>());
    _virtVfsDict["/"]->append(QUrl(QStringLiteral("virt:/") + dirName));
}

void virt_vfs::save()
{
    KConfig *db = &virt_vfs::getVirtDB();
    db->deleteGroup("virt_db");
    KConfigGroup group(db, "virt_db");

    QHashIterator<QString, QList<QUrl> *> it(_virtVfsDict);
    while (it.hasNext()) {
        it.next();
        QList<QUrl> *urlList = it.value();

        QList<QUrl>::iterator url;
        QStringList entry;
        for (url = urlList->begin(); url != urlList->end(); ++url) {
            entry.append((*url).toDisplayString());
        }
        // KDE 4.0 workaround: 'Item_' prefix is added as KConfig fails on 1 char names (such as /)
        group.writeEntry("Item_" + it.key(), entry);
        group.writeEntry("MetaInfo_" + it.key(), _metaInfoDict[it.key()]);
    }

    db->sync();
}

void virt_vfs::restore()
{
    KConfig *db = &virt_vfs::getVirtDB();
    const KConfigGroup dbGrp(db, "virt_db");

    const QMap<QString, QString> map = db->entryMap("virt_db");
    QMapIterator<QString, QString> it(map);
    while (it.hasNext()) {
        it.next();

        // KDE 4.0 workaround: check and remove 'Item_' prefix
        if (!it.key().startsWith(QLatin1String("Item_")))
            continue;
        const QString key = it.key().mid(5);

        const QList<QUrl> urlList = KrServices::toUrlList(dbGrp.readEntry(it.key(), QStringList()));
        _virtVfsDict.insert(key, new QList<QUrl>(urlList));
        _metaInfoDict.insert(key, dbGrp.readEntry("MetaInfo_" + key, QString()));
    }

    if (!_virtVfsDict["/"]) { // insert root element if missing for some reason
        _virtVfsDict.insert("/", new QList<QUrl>());
    }
}

vfile *virt_vfs::createVFile(const QUrl &url)
{
    if (url.scheme() == "virt") { // return a virtual directory in root
        QString path = url.path().mid(1);
        if (path.isEmpty())
            path = '/';
        return vfile::createVirtualDir(path, url);
    }

    const QUrl directory = url.adjusted(QUrl::RemoveFilename);

    if (url.isLocalFile()) {
        QFileInfo file(url.path());
        return file.exists() ? vfs::createLocalVFile(url.fileName(), directory.path(), true) : 0;
    }

    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    connect(statJob, &KIO::Job::result, this, &virt_vfs::slotStatResult);

    // ugly: we have to wait here until the stat job is finished
    QEventLoop eventLoop;
    connect(statJob, &KJob::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); // blocking until quit()

    if (_fileEntry.count() == 0) {
        return 0; // stat job failed
    }

    if (!_fileEntry.contains(KIO::UDSEntry::UDS_MODIFICATION_TIME)) {
        // TODO this also happens for FTP directories
        return 0; // file not found
    }

    return vfs::createVFileFromKIO(_fileEntry, directory, true);
}

KConfig &virt_vfs::getVirtDB()
{
    //virt_vfs_db = new KConfig("data",VIRT_VFS_DB,KConfig::NoGlobals);
    static KConfig db(VIRT_VFS_DB, KConfig::CascadeConfig, QStandardPaths::AppDataLocation);
    return db;
}

void virt_vfs::slotStatResult(KJob *job)
{
    _fileEntry = job->error() ? KIO::UDSEntry() : static_cast<KIO::StatJob *>(job)->statResult();
}

void virt_vfs::showError(const QString &error)
{
    QWidget *window = QApplication::activeWindow();
    KMessageBox::sorry(window, error); // window can be null, is allowed
}
