/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

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

#include <KI18n/KLocalizedString>
#include <KIOCore/KFileItem>
#include <KIO/JobUiDelegate>
#include <KWidgetsAddons/KMessageBox>

#include "../FileSystem/filesystem.h"
#include "../FileSystem/krpermhandler.h"
#include "../krservices.h"

SynchronizerDirList::SynchronizerDirList(QWidget *w, bool hidden) : QObject(), QHash<QString, FileItem *>(), fileIterator(0),
        parentWidget(w), busy(false), result(false), ignoreHidden(hidden), currentUrl()
{
}

SynchronizerDirList::~SynchronizerDirList()
{
    if (fileIterator)
        delete fileIterator;

    QHashIterator< QString, FileItem *> lit(*this);
    while (lit.hasNext())
        delete lit.next().value();
}

FileItem *SynchronizerDirList::search(const QString &name, bool ignoreCase)
{
    if (!ignoreCase) {
        if (!contains(name))
            return 0;
        return (*this)[ name ];
    }

    QHashIterator<QString, FileItem *> iter(*this);
    iter.toFront();

    QString file = name.toLower();

    while (iter.hasNext()) {
        FileItem *item = iter.next().value();
        if (file == item->getName().toLower())
            return item;
    }
    return 0;
}

FileItem *SynchronizerDirList::first()
{
    if (fileIterator == 0)
        fileIterator = new QHashIterator<QString, FileItem *> (*this);

    fileIterator->toFront();
    if (fileIterator->hasNext())
        return fileIterator->next().value();
    return 0;
}

FileItem *SynchronizerDirList::next()
{
    if (fileIterator == 0)
        fileIterator = new QHashIterator<QString, FileItem *> (*this);

    if (fileIterator->hasNext())
        return fileIterator->next().value();
    return 0;
}

bool SynchronizerDirList::load(const QString &urlIn, bool wait)
{
    if (busy)
        return false;

    currentUrl = urlIn;
    const QUrl url = QUrl::fromUserInput(urlIn, QString(), QUrl::AssumeLocalFile);

    QHashIterator< QString, FileItem *> lit(*this);
    while (lit.hasNext())
        delete lit.next().value();
    clear();
    if (fileIterator) {
        delete fileIterator;
        fileIterator = 0;
    }

    if (url.isLocalFile()) {
        const QString dir = FileSystem::ensureTrailingSlash(url).path();

        QT_DIR* qdir = QT_OPENDIR(dir.toLocal8Bit());
        if (!qdir)  {
            KMessageBox::error(parentWidget, i18n("Cannot open the folder %1.", dir), i18n("Error"));
            emit finished(result = false);
            return false;
        }

        QT_DIRENT* dirEnt;

        while ((dirEnt = QT_READDIR(qdir)) != NULL) {
            const QString name = QString::fromLocal8Bit(dirEnt->d_name);

            if (name == "." || name == "..") continue;
            if (ignoreHidden && name.startsWith('.')) continue;

            FileItem *item = FileSystem::createLocalFileItem(name, dir);

            insert(name, item);
        }

        QT_CLOSEDIR(qdir);
        emit finished(result = true);
        return true;
    } else {
        KIO::ListJob *job = KIO::listDir(KrServices::escapeFileUrl(url), KIO::HideProgressInfo, true);
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

void SynchronizerDirList::slotEntries(KIO::Job *job, const KIO::UDSEntryList& entries)
{
    KIO::ListJob *listJob = static_cast<KIO::ListJob *>(job);
    for (const KIO::UDSEntry entry : entries) {
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

