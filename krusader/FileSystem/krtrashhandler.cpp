/***************************************************************************
                     krtrashhandler.cpp  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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


KrTrashWatcher * KrTrashHandler::_trashWatcher = 0;

bool KrTrashHandler::isTrashEmpty()
{
    KConfig trashConfig("trashrc");
    KConfigGroup cfg(&trashConfig, "Status");
    return cfg.readEntry("Empty", false);
}

QString KrTrashHandler::trashIcon()
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
    QObject::connect(job, &KIO::Job::result, [=]() { FileSystemProvider::instance().refreshFilesystem(url); });
}

void KrTrashHandler::restoreTrashedFiles(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    KIO::RestoreJob *job = KIO::restoreFromTrash(urls);
    KJobWidgets::setWindow(job, krMainWindow);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    const QUrl url = urls.first().adjusted(QUrl::RemoveFilename);
    QObject::connect(job, &KIO::Job::result, [=]() { FileSystemProvider::instance().refreshFilesystem(url); });
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
    KrActions::actTrashBin->setIcon(QIcon::fromTheme(KrTrashHandler::trashIcon()));
}
