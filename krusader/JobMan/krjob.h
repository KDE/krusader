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
#ifndef KRJOB_H
#define KRJOB_H

#include <KIO/Job>
#include <KIO/CopyJob>


/** Wrapper for KIO::Job cause it has limitations.
 *
 * KIO::Jobs cannot be started in pause mode (and pausing direct after creation is buggy). Instead
 * a KrJob can be created which creates the KIO::Job on first start() call.
 *
 * Started jobs are recorded by KIO/FileUndoManager.
 *
 * KrJob is deleted after KIO::Job was deleted (which is after finished() call). Do not use
 * KIO::Job::finished() but KrJob::terminated() to be prepared for job deletion.
 */
class KrJob : public QObject
{
    Q_OBJECT
public:
    /** Type of this job. Add other types if needed.*/
    enum Type { Copy, Move, Link, Trash, Delete };

    /** Create a new copy, move, or link job. */
    static KrJob *createCopyJob(KIO::CopyJob::CopyMode mode, const QList<QUrl> &src,
                         const QUrl &destination, KIO::JobFlags flags);
    /** Create a new trash or delete job. */
    static KrJob *createDeleteJob(const QList<QUrl> &urls, bool moveToTrash);

    /** Start or resume this job. If job was started started() is emitted. */
    void start();
    /** Cancel this job and mark for deletion. */
    void cancel();
    /** Suspend job (if started). */
    void pause();

    /** Return true if job was started and is not suspended(). */
    bool isRunning() const { return _job && !_job->isSuspended(); }
    /** Return percent progress of job. */
    int percent() const { return _job ? _job->percent() : 0; }

    /** Return (initial) job description.
     * The KIO::Job emits a more detailed description after start.
     */
    QString description() const { return _description; }

  signals:
    /** Emitted if job was started. Parameter is the KIO::Job that was created. */
    void started(KIO::Job *job);
    /** Emitted if job is finished or was canceled. Job will be deleted afterwards. */
    void terminated(KrJob *krJob);

private:
    KrJob(Type type, const QList<QUrl> &urls, const QUrl &dest, KIO::JobFlags flags,
          const QString &description);

    const Type _type;
    const QList<QUrl> _urls;
    const QUrl _dest;
    const KIO::JobFlags _flags;
    const QString _description;

    KIO::Job *_job;
};

#endif // KRJOB_H
