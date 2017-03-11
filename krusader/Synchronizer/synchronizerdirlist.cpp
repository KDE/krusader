/***************************************************************************
                   synchronizerdirlist.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
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
        KIO::Job *job = KIO::listDir(KrServices::escapeFileUrl(url), KIO::HideProgressInfo, true);
        connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
                this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotListResult(KJob*)));
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

