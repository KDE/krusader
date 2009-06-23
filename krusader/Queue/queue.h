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

#ifndef QUEUE_H
#define QUEUE_H

#include <QtCore/QObject>
#include "../VFS/kiojobwrapper.h"
#include <QList>
#include <QTimer>
#include <QTime>

class KJob;

#define PERCENT_UNUSED  -2
#define PERCENT_UNKNOWN -1

/**
 * Queue can hold anything which inherits KIO::Job, and schedule it, start it, stop etc...
 * the main reason to hold the Job itself (at least for phase 1) is to keep the code
 * in krusader relatively unchaged, and allow to create the job as usual and choose if
 * to start it, or queue it.
 *
 */
class Queue: public QObject
{
    Q_OBJECT
public:
    Queue(const QString& name);
    virtual ~Queue();

    inline const QString& name() const {
        return _name;
    }
    void enqueue(KIOJobWrapper *job);
    int  count() {
        return _jobs.size();
    }
    bool isSuspended() {
        return _suspended;
    }
    QTime scheduleTime();
    int getPercent() {
        return _percent;
    }
    void remove(KIOJobWrapper *);

    QList<QString> itemDescriptions();
    QList<KIOJobWrapper *> items();

public slots:
    void suspend();
    void resume();
    void schedule(const QTime &);

protected slots:
    void slotJobDestroyed(QObject *);
    void slotResult(KJob *);
    void slotPercent(KJob *, unsigned long percent);

protected:
    QString _name;
    QList<KIOJobWrapper *> _jobs;
    bool _suspended;
    QTimer _scheduleTimer;
    QTime _scheduleTime;
    int _percent;

signals:
    void showQueueDialog();
    void changed();
    void emptied();
    void stateChanged();
    void percent(Queue *, int value);
};

#endif // QUEUE_H
