/*
    SPDX-FileCopyrightText: 2016-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krjob.h"

#include <KIO/DeleteJob>
#include <KIO/FileUndoManager>
#include <KLocalizedString>

KrJob *KrJob::createCopyJob(KIO::CopyJob::CopyMode mode, const QList<QUrl> &src, const QUrl &destination, KIO::JobFlags flags)
{
    return createKrCopyJob(mode, src, destination, flags);
}

KrJob *KrJob::createDeleteJob(const QList<QUrl> &urls, bool moveToTrash)
{
    const Type type = moveToTrash ? Trash : Delete;
    const bool oneFile = urls.length() == 1;
    const QString description = moveToTrash
        ? (oneFile ? i18n("Move %1 to trash", urls.first().toDisplayString()) : i18np("Move %1 file to trash", "Move %1 files to trash", urls.length()))
        : (oneFile ? i18n("Delete %1", urls.first().toDisplayString()) : i18np("Delete %1 file", "Delete %1 files", urls.length()));

    return new KrJob(type, urls, QUrl(), KIO::DefaultFlags /*dummy*/, description);
}

KrJob *KrJob::createDropJob(KIO::DropJob *dropJob, KIO::CopyJob *job)
{
    // NOTE: DrobJobs are internally recorded
    // KIO::FileUndoManager::self()->recordCopyJob(job);
    return createKrCopyJob(job->operationMode(), job->srcUrls(), job->destUrl(), KIO::DefaultFlags /*dummy*/, job, dropJob);
}

KrJob *KrJob::createKrCopyJob(KIO::CopyJob::CopyMode mode,
                              const QList<QUrl> &src,
                              const QUrl &destination,
                              KIO::JobFlags flags,
                              KIO::CopyJob *job,
                              KIO::DropJob *dropJob)
{
    Type type(Copy);
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
        type = Link;
        description = i18n("Link to %1", destination.toDisplayString());
        break;
    }

    return new KrJob(type, src, destination, flags, description, job, dropJob);
}

KrJob::KrJob(Type type,
             const QList<QUrl> &urls,
             const QUrl &dest,
             KIO::JobFlags flags,
             const QString &description,
             KIO::CopyJob *copyJob,
             KIO::DropJob *dropJob)
    : m_type(type)
    , m_urls(urls)
    , m_dest(dest)
    , m_flags(flags)
    , m_description(description)
    , m_job(copyJob)
    , m_dropJob(dropJob)
{
    if (m_job) {
        connectStartedJob();
    }
}

void KrJob::start()
{
    if (m_job) {
        // job was already started
        m_job->resume();
        return;
    }

    switch (m_type) {
    case Copy: {
        KIO::CopyJob *job = KIO::copy(m_urls, m_dest, m_flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        m_job = job;
        break;
    }
    case Move: {
        KIO::CopyJob *job = KIO::move(m_urls, m_dest, m_flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        m_job = job;
        break;
    }
    case Link: {
        KIO::CopyJob *job = KIO::link(m_urls, m_dest, m_flags);
        KIO::FileUndoManager::self()->recordCopyJob(job);
        m_job = job;
        break;
    }
    case Trash: {
        m_job = KIO::trash(m_urls);
        KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, m_urls, QUrl("trash:/"), m_job);
        break;
    }
    case Delete:
        m_job = KIO::del(m_urls);
    }

    connectStartedJob();

    emit started(m_job);
}

void KrJob::connectStartedJob()
{
    connect(m_job, &KIO::Job::finished, this, [=]() {
        emit terminated(this);
        deleteLater();
    });
}

void KrJob::cancel()
{
    if (m_dropJob) { // kill the parent; killing the copy subjob does not kill the parent dropjob
        m_dropJob->kill();
    } else if (m_job) {
        m_job->kill();
    } else {
        emit terminated(this);
        deleteLater();
    }
}

void KrJob::pause()
{
    if (m_job)
        m_job->suspend();
}
