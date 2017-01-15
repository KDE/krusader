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
#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QStandardPaths>

#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>
#include <KCoreAddons/KDirWatch>
#include <KCoreAddons/KJobTrackerInterface>
#include <KIO/EmptyTrashJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KJobWidgets/KJobWidgets>

#include "kractions.h"
#include "../krglobal.h"
#include "Panel/krpanel.h"
#include "Panel/panelfunc.h"

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
    job->ui()->setAutoErrorHandlingEnabled(true);
    QObject::connect(job, &KIO::Job::result, ACTIVE_PANEL->func, &ListPanelFunc::refresh);
}

void KrTrashHandler::restoreTrashedFiles(const QList<QUrl> &urls)
{
    KonqMultiRestoreJob* job = new KonqMultiRestoreJob(urls);
    KIO::JobUiDelegate *ui = static_cast<KIO::JobUiDelegate*>(job->uiDelegate());
    ui->setWindow(krMainWindow);
    KIO::getJobTracker()->registerJob(job);
    QObject::connect(job, SIGNAL(result(KJob *)), ACTIVE_PANEL->func, SLOT(refresh()));
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

KonqMultiRestoreJob::KonqMultiRestoreJob(const QList<QUrl>& urls)
        : KIO::Job(),
        m_urls(urls), m_urlsIterator(m_urls.begin()),
        m_progress(0)
{
    QTimer::singleShot(0, this, SLOT(slotStart()));
    setUiDelegate(new KIO::JobUiDelegate);
}

void KonqMultiRestoreJob::slotStart()
{
    if (m_urlsIterator == m_urls.begin())   // first time: emit total
        setTotalAmount(KJob::Files, m_urls.count());

    if (m_urlsIterator != m_urls.end()) {
        const QUrl &url = *m_urlsIterator;

        QUrl new_url = url;
        if (new_url.scheme() == QStringLiteral("system") &&
                new_url.path().startsWith(QLatin1String("/trash"))) {
            QString path = new_url.path();
            path.remove(0, 6);
            new_url.setScheme(QStringLiteral("trash"));
            new_url.setPath(path);
        }

        Q_ASSERT(new_url.scheme() == QStringLiteral("trash"));
        QByteArray packedArgs;
        QDataStream stream(&packedArgs, QIODevice::WriteOnly);
        stream << (int)3 << new_url;
        KIO::Job* job = KIO::special(new_url, packedArgs, KIO::HideProgressInfo);
        addSubjob(job);
        setProcessedAmount(KJob::Files, processedAmount(KJob::Files) + 1);
    } else { // done!
        emitResult();
    }
}

void KonqMultiRestoreJob::slotResult(KJob *job)
{
    if (job->error()) {
        KIO::Job::slotResult(job);   // will set the error and emit result(this)
        return;
    }
    removeSubjob(job);
    // Move on to next one
    ++m_urlsIterator;
    ++m_progress;
    //emit processedSize( this, m_progress );
    emitPercent(m_progress, m_urls.count());
    slotStart();
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
