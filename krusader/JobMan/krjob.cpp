/*****************************************************************************
 * Copyright (C) 2016 Krusader Krew                                          *
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

#include "krjob.h"

#include <KI18n/KLocalizedString>
#include <KIO/DeleteJob>
#include <KIO/FileUndoManager>

#include "jobman.h"
#include "../krglobal.h"

KrJob *KrJob::copyJob(KIO::CopyJob::CopyMode mode, const QList<QUrl> &src, const QUrl &destination,
                     KIO::JobFlags flags, bool enqueue)
{
    Type type;
    QString description;
    switch (mode) {
    case KIO::CopyJob::Copy:
        type = Copy;
        description = i18n("Copy to %1", destination.toDisplayString());
        break;
    case KIO::CopyJob::Move:
        type = Move;
        description = i18n("Move to %1", destination.toDisplayString());
        break;
    case KIO::CopyJob::Link:
        description = i18n("Link to %1", destination.toDisplayString());
        type = Link;
        break;
    }

    KrJob *krJob = new KrJob(type, src, destination, flags, description);
    krJobMan->manageJob(krJob, enqueue);

    return krJob;
}

KrJob *KrJob::deleteJob(const QList<QUrl> &urls, bool moveToTrash)
{
    const Type type = moveToTrash ? Trash : Delete;
    const bool oneFile = urls.length() == 1;
    const QString description =
        moveToTrash ? (oneFile ? i18n("Move %1 to trash", urls.first().toDisplayString()) :
                                 i18n("Move %1 files to trash", urls.length())) :
                      (oneFile ? i18n("Delete %1", urls.first().toDisplayString()) :
                                 i18n("Delete %1 files", urls.length()));

    KrJob *krJob = new KrJob(type, urls, QUrl(), KIO::DefaultFlags, description);
    krJobMan->manageJob(krJob, false);

    return krJob;
}

KrJob::KrJob(Type type, const QList<QUrl> &urls, const QUrl &dest, KIO::JobFlags flags,
             const QString &description)
    : _type(type), _urls(urls), _dest(dest), _flags(flags), _description(description), _job(0)
{
}

void KrJob::start()
{
    if (_job) {
        // job was already started
        _job->resume();
        return;
    }

    switch (_type) {
    case Copy: {
        KIO::CopyJob *job = KIO::copy(_urls, _dest, _flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        _job = job;
        break;
    }
    case Move: {
        KIO::CopyJob *job = KIO::move(_urls, _dest, _flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        _job = job;
        break;
    }
    case Link: {
        KIO::CopyJob *job = KIO::link(_urls, _dest, _flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        _job = job;
        break;
    }
    case Trash: {
        _job = KIO::trash(_urls);
        KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, _urls, QUrl("trash:/"),
                                                _job);
        break;
    }
    case Delete:
        _job = KIO::del(_urls);
    }

    connect(_job, &KIO::Job::finished, [=]() {
        emit terminated(this);
        deleteLater();
    });

    emit started(_job);
}

void KrJob::cancel()
{
    if (_job)
        _job->kill();
    else {
        emit terminated(this);
        deleteLater();
    }
}

void KrJob::pause()
{
    if (_job)
        _job->suspend();
}
