/*****************************************************************************
 * Copyright (C) 2008-2009 Csaba Karai <cskarai@freemail.hu>                 *
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

#include "queue.h"
#include "../krglobal.h"
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <QTime>

Queue::Queue(const QString& name): _name(name), _suspended(false), _percent(PERCENT_UNUSED)
{
    connect(&_scheduleTimer, SIGNAL(timeout()), this, SLOT(resume()));
}

Queue::~Queue()
{
    foreach(KIOJobWrapper * job, _jobs) {
        job->abort();
        delete job;
    }
    _jobs.clear();
    // TODO: save queue on delete? or just delete jobs
}

void Queue::enqueue(KIOJobWrapper *job)
{
    bool isRunning = _jobs.count();
    _jobs.append(job);
    connect(job, SIGNAL(destroyed(QObject *)), this, SLOT(slotJobDestroyed(QObject *)));
    job->connectTo(SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
    job->connectTo(SIGNAL(percent(KJob*, unsigned long)), this, SLOT(slotPercent(KJob*, unsigned long)));

    if (!_suspended && !isRunning) {
        job->start();
        slotPercent(0, PERCENT_UNKNOWN);
    }

    emit changed();
    emit showQueueDialog();
}

void Queue::slotJobDestroyed(QObject * obj)
{
    KIOJobWrapper * jw = (KIOJobWrapper*) obj;
    bool current = false;
    if (_jobs.count() > 0 && _jobs[ 0 ] == jw)
        current = true;
    _jobs.removeAll(jw);
    if (current)
        slotPercent(0, PERCENT_UNUSED);
    if (!_suspended && current && _jobs.count() > 0) {
        _jobs[ 0 ]->start();
        slotPercent(0, PERCENT_UNKNOWN);
    }
    emit changed();
}

void Queue::slotResult(KJob * job)
{
    if (_jobs.count() > 0) {
        KIOJobWrapper * jw = _jobs[ 0 ];
        KIO::Job * kjob = jw->job();
        if (kjob && kjob == job) {
            slotPercent(0, PERCENT_UNUSED);
            _jobs.removeAll(jw);
            if (job->error() && _jobs.count() > 0) {
                // what to do with the other elements in the queue
                QString message = i18n("An error occurred during processing the queue.\n");
                if (job->error() == KIO::ERR_USER_CANCELED)
                    message = i18n("The last processed element in the queue was aborted.\n");
                message += i18n("Do you wish to continue with the next element, delete the queue or suspend the queue?");
                int res = KMessageBox::questionYesNoCancel(krMainWindow, message,
                          i18n("Krusader::Queue"), KStandardGuiItem::cont(),
                          KStandardGuiItem::del(), KGuiItem(i18n("Suspend")));
                switch (res) {
                case KMessageBox::Yes: // continue
                    break;
                case KMessageBox::No: // delete the queue
                    foreach(KIOJobWrapper * job, _jobs) {
                        job->abort();
                        delete job;
                    }
                    _jobs.clear();
                    emit changed();
                    emit emptied();
                    return;
                default: // suspend
                    emit changed();
                    suspend();
                    return;
                }
            }

            emit changed();
            if (!_suspended) {
                if (_jobs.count() > 0) {
                    _jobs[ 0 ]->start();
                    slotPercent(0, PERCENT_UNKNOWN);
                } else
                    emit emptied();
            }
        }
    }
}

QList<KIOJobWrapper *> Queue::items()
{
    return _jobs;
}

QList<QString> Queue::itemDescriptions()
{
    QList<QString> ret;
    foreach(KIOJobWrapper *job, _jobs) {
        ret.append(job->typeStr() + " : " + job->url().prettyUrl());
    }
    return ret;
}

void Queue::suspend()
{
    _suspended = true;
    if ((_jobs.count() > 0) && _jobs[ 0 ]->isStarted())
        _jobs[ 0 ]->suspend();

    emit stateChanged();
}

void Queue::resume()
{
    _suspended = false;
    _scheduleTimer.stop();
    if (_jobs.count() > 0) {
        if (_jobs[ 0 ]->isSuspended())
            _jobs[ 0 ]->resume();
        else if (!_jobs[ 0 ]->isStarted()) {
            _jobs[ 0 ]->start();
            slotPercent(0, PERCENT_UNKNOWN);
        }
    }

    emit stateChanged();
}

void Queue::schedule(const QTime &time)
{
    QTime nowTime = QTime::currentTime();
    _suspended = true;
    if ((_jobs.count() > 0) && _jobs[ 0 ]->isStarted())
        _jobs[ 0 ]->suspend();

    int now = ((nowTime.hour() * 60) + nowTime.minute()) * 60 + nowTime.second();
    int schedule = ((time.hour() * 60) + time.minute()) * 60 + time.second();

    int diff = schedule - now;
    if (diff < 0)
        diff += 24 * 60 * 60; // 1 day plus

    diff *= 1000; // milliseconds
    _scheduleTime = time;

    _scheduleTimer.stop();
    _scheduleTimer.setSingleShot(true);
    _scheduleTimer.start(diff);

    emit stateChanged();
}

QTime Queue::scheduleTime()
{
    if (_suspended && _scheduleTimer.isActive())
        return _scheduleTime;
    else
        return QTime();
}

void Queue::slotPercent(KJob *, unsigned long percent_)
{
    _percent = percent_;
    emit percent(this, percent_);
}

void Queue::remove(KIOJobWrapper * jw)
{
    bool current = false;
    if (_jobs.count() > 0 && _jobs[ 0 ] == jw)
        current = true;
    _jobs.removeAll(jw);
    jw->abort();
    delete jw;
    if (current)
        slotPercent(0, PERCENT_UNUSED);
    if (!_suspended && current && _jobs.count() > 0) {
        _jobs[ 0 ]->start();
        slotPercent(0, PERCENT_UNKNOWN);
    }
    emit changed();
}
