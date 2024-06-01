/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "synchronizerdirlist.h"

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

#include <qplatformdefs.h>
// QtCore
#include <QDir>
// QtWidgets
#include <QApplication>

#include <KFileItem>
#include <KIO/JobUiDelegate>
#include <KIO/ListJob>
#include <KLocalizedString>
#include <KMessageBox>

#include "../FileSystem/filesystem.h"
#include "../FileSystem/krpermhandler.h"
#include "../krservices.h"

SynchronizerDirList::SynchronizerDirList(QWidget *w, bool hidden)
    : fileIterator(nullptr)
    , parentWidget(w)
    , busy(false)
    , result(false)
    , ignoreHidden(hidden)
    , currentUrl()
{
}

SynchronizerDirList::~SynchronizerDirList()
{
    if (fileIterator)
        delete fileIterator;

    QHashIterator<QString, FileItem *> lit(*this);
    while (lit.hasNext())
        delete lit.next().value();
}

FileItem *SynchronizerDirList::search(const QString &name, bool ignoreCase)
{
    if (!ignoreCase) {
        if (!contains(name))
            return nullptr;
        return (*this)[name];
    }

    QHashIterator<QString, FileItem *> iter(*this);
    iter.toFront();

    QString file = name.toLower();

    while (iter.hasNext()) {
        FileItem *item = iter.next().value();
        if (file == item->getName().toLower())
            return item;
    }
    return nullptr;
}

FileItem *SynchronizerDirList::first()
{
    if (fileIterator == nullptr)
        fileIterator = new QHashIterator<QString, FileItem *>(*this);

    fileIterator->toFront();
    if (fileIterator->hasNext())
        return fileIterator->next().value();
    return nullptr;
}

FileItem *SynchronizerDirList::next()
{
    if (fileIterator == nullptr)
        fileIterator = new QHashIterator<QString, FileItem *>(*this);

    if (fileIterator->hasNext())
        return fileIterator->next().value();
    return nullptr;
}

bool SynchronizerDirList::load(const QString &urlIn, bool wait)
{
    if (busy)
        return false;

    currentUrl = urlIn;
    const QUrl url = QUrl::fromUserInput(urlIn, QString(), QUrl::AssumeLocalFile);

    QHashIterator<QString, FileItem *> lit(*this);
    while (lit.hasNext())
        delete lit.next().value();
    clear();
    if (fileIterator) {
        delete fileIterator;
        fileIterator = nullptr;
    }

    if (url.isLocalFile()) {
        const QString dir = FileSystem::ensureTrailingSlash(url).path();

        QT_DIR *qdir = QT_OPENDIR(dir.toLocal8Bit().data());
        if (!qdir) {
            KMessageBox::error(parentWidget, i18n("Cannot open the folder %1.", dir), i18n("Error"));
            emit finished(result = false);
            return false;
        }

        QT_DIRENT *dirEnt;

        while ((dirEnt = QT_READDIR(qdir)) != nullptr) {
            const QString name = QString::fromLocal8Bit(dirEnt->d_name);

            if (name == "." || name == "..")
                continue;
            if (ignoreHidden && name.startsWith('.'))
                continue;

            FileItem *item = FileSystem::createLocalFileItem(name, dir);

            insert(name, item);
        }

        QT_CLOSEDIR(qdir);
        emit finished(result = true);
        return true;
    } else {
        KIO::ListJob *job = KIO::listDir(KrServices::escapeFileUrl(url), KIO::HideProgressInfo, KIO::ListJob::ListFlag::IncludeHidden);
        connect(job, &KIO::ListJob::entries, this, &SynchronizerDirList::slotEntries);
        connect(job, &KIO::ListJob::result, this, &SynchronizerDirList::slotListResult);
        busy = true;

        if (!wait)
            return true;

        while (busy)
            qApp->processEvents();
        return result;
    }
}

void SynchronizerDirList::slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    auto *listJob = dynamic_cast<KIO::ListJob *>(job);
    for (const KIO::UDSEntry &entry : entries) {
        FileItem *item = FileSystem::createFileItemFromKIO(entry, listJob->url());
        if (item) {
            insert(item->getName(), item);
        }
    }
}

void SynchronizerDirList::slotListResult(KJob *job)
{
    busy = false;
    if (job && job->error()) {
        job->uiDelegate()->showErrorMessage();
        emit finished(result = false);
        return;
    }
    emit finished(result = true);
}
