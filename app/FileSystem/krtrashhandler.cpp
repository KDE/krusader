/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krtrashhandler.h"

// QtCore
#include <QDir>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KIO/DeleteOrTrashJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KIO/RestoreJob>
#include <KJobWidgets>

#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "filesystemprovider.h"

KrTrashWatcher *KrTrashHandler::_trashWatcher = nullptr;

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
    using IFace = KIO::AskUserActionInterface;
    auto deleteJob = new KIO::DeleteOrTrashJob({}, IFace::EmptyTrash, IFace::DefaultConfirmation, krMainWindow);
    deleteJob->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::Flags{}, krMainWindow));
    deleteJob->uiDelegate()->setAutoErrorHandlingEnabled(true);

    QObject::connect(deleteJob, &KIO::Job::result, deleteJob, [](KJob *job) {
        if (job->error() != KJob::NoError) {
            return;
        }
        FileSystemProvider::instance().refreshFilesystems(QUrl("trash:/"), false);
    });
    deleteJob->start();
}

void KrTrashHandler::restoreTrashedFiles(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    KIO::RestoreJob *job = KIO::restoreFromTrash(urls);
    KJobWidgets::setWindow(job, krMainWindow);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    const QUrl url = urls.first().adjusted(QUrl::RemoveFilename);
    QObject::connect(job, &KIO::Job::result, [=]() {
        FileSystemProvider::instance().refreshFilesystems(url, false);
    });
}

void KrTrashHandler::startWatcher()
{
    if (!_trashWatcher)
        _trashWatcher = new KrTrashWatcher();
}

void KrTrashHandler::stopWatcher()
{
    delete _trashWatcher;
    _trashWatcher = nullptr;
}

KrTrashWatcher::KrTrashWatcher()
{
    _watcher = new KDirWatch();
    connect(_watcher, &KDirWatch::created, this, &KrTrashWatcher::slotTrashChanged);
    connect(_watcher, &KDirWatch::dirty, this, &KrTrashWatcher::slotTrashChanged);
    const QString trashrcFile = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath("trashrc");
    _watcher->addFile(trashrcFile);
    _watcher->startScan(true);
}

KrTrashWatcher::~KrTrashWatcher()
{
    delete _watcher;
    _watcher = nullptr;
}

void KrTrashWatcher::slotTrashChanged()
{
    KrActions::actTrashBin->setIcon(Icon(KrTrashHandler::trashIconName()));
}
