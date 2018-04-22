/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2009-2018 Krusader Krew [https://krusader.org]              *
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

#include "krtrashhandler.h"

// QtCore
#include <QDir>
#include <QStandardPaths>

#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>
#include <KIO/EmptyTrashJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/RestoreJob>
#include <KJobWidgets/KJobWidgets>

#include "filesystemprovider.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../icon.h"


KrTrashWatcher * KrTrashHandler::_trashWatcher = 0;

bool KrTrashHandler::isTrashEmpty()
{
    KConfig trashConfig("trashrc");
    KConfigGroup cfg(&trashConfig, "Status");
    return cfg.readEntry("Empty", false);
}

QString KrTrashHandler::trashIconName()
{
    return isTrashEmpty() ? "user-trash" : "user-trash-full";
}

void KrTrashHandler::emptyTrash()
{
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(krMainWindow);
    if (!uiDelegate.askDeleteConfirmation(QList<QUrl>(), KIO::JobUiDelegate::EmptyTrash,
                                          KIO::JobUiDelegate::DefaultConfirmation))
        return;

    KIO::Job *job = KIO::emptyTrash();
    KJobWidgets::setWindow(job, krMainWindow);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    const QUrl url = QUrl("trash:/");
    QObject::connect(job, &KIO::Job::result,
                     [=]() { FileSystemProvider::instance().refreshFilesystems(url, false); });
}

void KrTrashHandler::restoreTrashedFiles(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    KIO::RestoreJob *job = KIO::restoreFromTrash(urls);
    KJobWidgets::setWindow(job, krMainWindow);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    const QUrl url = urls.first().adjusted(QUrl::RemoveFilename);
    QObject::connect(job, &KIO::Job::result,
                     [=]() { FileSystemProvider::instance().refreshFilesystems(url, false); });
}

void KrTrashHandler::startWatcher()
{
    if (!_trashWatcher)
        _trashWatcher = new KrTrashWatcher();
}

void KrTrashHandler::stopWatcher()
{
    delete _trashWatcher;
    _trashWatcher = 0;
}


KrTrashWatcher::KrTrashWatcher()
{
    _watcher = new KDirWatch();
    connect(_watcher, &KDirWatch::created, this, &KrTrashWatcher::slotTrashChanged);
    connect(_watcher, &KDirWatch::dirty, this, &KrTrashWatcher::slotTrashChanged);
    const QString trashrcFile =
        QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath("trashrc");
    _watcher->addFile(trashrcFile);
    _watcher->startScan(true);
}

KrTrashWatcher::~KrTrashWatcher()
{
    delete _watcher;
    _watcher = 0;
}

void KrTrashWatcher::slotTrashChanged()
{
    KrActions::actTrashBin->setIcon(Icon(KrTrashHandler::trashIconName()));
}
