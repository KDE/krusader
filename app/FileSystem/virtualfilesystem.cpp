/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "virtualfilesystem.h"

// QtCore
#include <QDir>
#include <QEventLoop>
#include <QUrl>
// QtWidgets
#include <QApplication>

#include <KFileItem>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/DirectorySizeJob>
#include <KIO/StatJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlMimeData>

#include "../defaults.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "fileitem.h"

#define VIRTUALFILESYSTEM_DB "virtualfilesystem.db"

QHash<QString, QList<QUrl> *> VirtualFileSystem::_virtFilesystemDict;
QHash<QString, QString> VirtualFileSystem::_metaInfoDict;

VirtualFileSystem::VirtualFileSystem()
{
    if (_virtFilesystemDict.isEmpty()) {
        restore();
    }

    _type = FS_VIRTUAL;
}

void VirtualFileSystem::copyFiles(const QList<QUrl> &urls,
                                  const QUrl &destination,
                                  KIO::CopyJob::CopyMode /*mode*/,
                                  bool /*showProgressInfo*/,
                                  JobMan::StartMode /*startMode*/)
{
    const QString dir = QDir(destination.path()).absolutePath().remove('/');

    if (dir.isEmpty()) {
        showError(
            i18n("You cannot copy files directly to the 'virt:/' folder.\n"
                 "You can create a sub folder and copy your files into it."));
        return;
    }

    if (!_virtFilesystemDict.contains(dir)) {
        mkDirInternal(dir);
    }

    QList<QUrl> *urlList = _virtFilesystemDict[dir];
    for (const QUrl &fileUrl : urls) {
        if (!urlList->contains(fileUrl)) {
            urlList->push_back(fileUrl);
        }
    }

    emit fileSystemChanged(QUrl("virt:///" + dir), false); // may call refresh()
}

void VirtualFileSystem::dropFiles(const QUrl &destination, QDropEvent *event, QWidget *)
{
    const QList<QUrl> &urls = KUrlMimeData::urlsFromMimeData(event->mimeData());
    // dropping on virtual filesystem is always copy operation
    copyFiles(urls, destination);
}

void VirtualFileSystem::addFiles(const QList<QUrl> &fileUrls, KIO::CopyJob::CopyMode /*mode*/, const QString &dir)
{
    QUrl destination(_currentDirectory);
    if (!dir.isEmpty()) {
        destination.setPath(QDir::cleanPath(destination.path() + '/' + dir));
    }
    copyFiles(fileUrls, destination);
}

void VirtualFileSystem::remove(const QStringList &fileNames)
{
    const QString parentDir = currentDir();
    if (parentDir == "/") { // remove virtual directory
        for (const QString &filename : fileNames) {
            _virtFilesystemDict["/"]->removeAll(QUrl(QStringLiteral("virt:/") + filename));
            delete _virtFilesystemDict[filename];
            _virtFilesystemDict.remove(filename);
            _metaInfoDict.remove(filename);
        }
    } else {
        // remove the URLs from the collection
        for (const QString &name : fileNames) {
            if (_virtFilesystemDict.find(parentDir) != _virtFilesystemDict.end()) {
                QList<QUrl> *urlList = _virtFilesystemDict[parentDir];
                urlList->removeAll(getUrl(name));
            }
        }
    }

    emit fileSystemChanged(currentDirectory(), true); // will call refresh()
}

QUrl VirtualFileSystem::getUrl(const QString &name) const
{
    FileItem *item = getFileItem(name);
    if (!item) {
        return QUrl(); // not found
    }

    return item->getUrl();
}

void VirtualFileSystem::mkDir(const QString &name)
{
    if (currentDir() != "/") {
        showError(i18n("Creating new folders is allowed only in the 'virt:/' folder."));
        return;
    }

    mkDirInternal(name);

    emit fileSystemChanged(currentDirectory(), false); // will call refresh()
}

void VirtualFileSystem::rename(const QString &fileName, const QString &newName)
{
    FileItem *item = getFileItem(fileName);
    if (!item)
        return; // not found

    if (currentDir() == "/") { // rename virtual directory
        _virtFilesystemDict["/"]->append(QUrl(QStringLiteral("virt:/") + newName));
        _virtFilesystemDict["/"]->removeAll(QUrl(QStringLiteral("virt:/") + fileName));
        _virtFilesystemDict.insert(newName, _virtFilesystemDict.take(fileName));
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
    _virtFilesystemDict[currentDir()]->append(dest);

    KIO::Job *job = KIO::moveAs(item->getUrl(), dest, KIO::HideProgressInfo);
    connect(job, &KIO::Job::result, this, [=](KJob *job) {
        slotJobResult(job, false);
    });
    connect(job, &KIO::Job::result, this, [=]() {
        emit fileSystemChanged(currentDirectory(), false);
    });
}

bool VirtualFileSystem::canMoveToTrash(const QStringList &fileNames) const
{
    if (isRoot())
        return false;

    for (const QString &fileName : fileNames) {
        if (!getUrl(fileName).isLocalFile()) {
            return false;
        }
    }
    return true;
}

void VirtualFileSystem::setMetaInformation(const QString &info)
{
    _metaInfoDict[currentDir()] = info;
}

// ==== protected ====

bool VirtualFileSystem::refreshInternal(const QUrl &directory, bool onlyScan)
{
    _currentDirectory = cleanUrl(directory);
    _currentDirectory.setHost("");
    // remove invalid subdirectories
    _currentDirectory.setPath('/' + _currentDirectory.path().remove('/'));

    if (!_virtFilesystemDict.contains(currentDir())) {
        if (onlyScan) {
            return false; // virtual dir does not exist
        } else {
            // Silently creating non-existing directories here. The search and locate tools
            // expect this. And the user can enter some directory and it will be created.
            mkDirInternal(currentDir());
            save();
            // infinite loop possible
            // emit fileSystemChanged(currentDirectory());
            return true;
        }
    }

    QList<QUrl> *urlList = _virtFilesystemDict[currentDir()];

    if (!onlyScan) {
        const QString metaInfo = _metaInfoDict[currentDir()];
        emit fileSystemInfoChanged(metaInfo.isEmpty() ? i18n("Virtual filesystem") : metaInfo, "", 0, 0);
    }

    QMutableListIterator<QUrl> it(*urlList);
    while (it.hasNext()) {
        const QUrl url = it.next();
        FileItem *item = createFileItem(url);
        if (!item) { // remove URL from the list for a file that no longer exists
            it.remove();
        } else {
            addFileItem(item);
        }
    }

    save();
    return true;
}

// ==== private ====

void VirtualFileSystem::mkDirInternal(const QString &name)
{
    // clean path, consistent with currentDir()
    QString dirName = name;
    dirName = dirName.remove('/');
    if (dirName.isEmpty())
        dirName = '/';

    _virtFilesystemDict.insert(dirName, new QList<QUrl>());
    _virtFilesystemDict["/"]->append(QUrl(QStringLiteral("virt:/") + dirName));
}

void VirtualFileSystem::save()
{
    KConfig *db = &VirtualFileSystem::getVirtDB();
    db->deleteGroup("virt_db");
    KConfigGroup group(db, "virt_db");

    QHashIterator<QString, QList<QUrl> *> it(_virtFilesystemDict);
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

void VirtualFileSystem::restore()
{
    KConfig *db = &VirtualFileSystem::getVirtDB();
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
        _virtFilesystemDict.insert(key, new QList<QUrl>(urlList));
        _metaInfoDict.insert(key, dbGrp.readEntry("MetaInfo_" + key, QString()));
    }

    if (!_virtFilesystemDict["/"]) { // insert root element if missing for some reason
        _virtFilesystemDict.insert("/", new QList<QUrl>());
    }
}

FileItem *VirtualFileSystem::createFileItem(const QUrl &url)
{
    if (url.scheme() == "virt") { // return a virtual directory in root
        QString path = url.path().mid(1);
        if (path.isEmpty())
            path = '/';
        return FileItem::createVirtualDir(path, url);
    }

    const QUrl directory = url.adjusted(QUrl::RemoveFilename);

    if (url.isLocalFile()) {
        QFileInfo file(url.path());
        return file.exists() ? FileSystem::createLocalFileItem(url.fileName(), directory.path(), true) : nullptr;
    }

    KIO::StatJob *statJob = KIO::stat(url, KIO::HideProgressInfo);
    connect(statJob, &KIO::Job::result, this, &VirtualFileSystem::slotStatResult);

    // ugly: we have to wait here until the stat job is finished
    QEventLoop eventLoop;
    connect(statJob, &KJob::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); // blocking until quit()

    if (_fileEntry.count() == 0) {
        return nullptr; // stat job failed
    }

    if (!_fileEntry.contains(KIO::UDSEntry::UDS_MODIFICATION_TIME)) {
        // TODO this also happens for FTP directories
        return nullptr; // file not found
    }

    return FileSystem::createFileItemFromKIO(_fileEntry, directory, true);
}

KConfig &VirtualFileSystem::getVirtDB()
{
    // virtualfilesystem_db = new KConfig("data",VIRTUALFILESYSTEM_DB,KConfig::NoGlobals);
    static KConfig db(VIRTUALFILESYSTEM_DB, KConfig::CascadeConfig, QStandardPaths::AppDataLocation);
    return db;
}

void VirtualFileSystem::slotStatResult(KJob *job)
{
    _fileEntry = job->error() ? KIO::UDSEntry() : dynamic_cast<KIO::StatJob *>(job)->statResult();
}

void VirtualFileSystem::showError(const QString &error)
{
    QWidget *window = QApplication::activeWindow();
    KMessageBox::error(window, error); // window can be null, is allowed
}
