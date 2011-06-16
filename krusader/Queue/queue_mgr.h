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

#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "queue.h"
#include <qmap.h>
#include <QList>

/**
 * QueueManager holds multiple queues and has a static
 * method that fetches a queue by name. calling it with
 * no arguments will fetch the default queue
 */
class QueueManager : public QObject
{
    Q_OBJECT

    static QString defaultName();
public:
    QueueManager();
    ~QueueManager();

    static Queue* queue(const QString& queueName = defaultName());
    static QList<QString> queues();

    static Queue* currentQueue();
    static void setCurrentQueue(Queue * queue);
    static Queue* createQueue(const QString& queueName);
    static void   removeQueue(Queue * queue);

    static QueueManager * instance() {
        return _self;
    }

protected slots:
    void slotShowQueueDialog();
    void slotQueueEmptied();

signals:
    void queueInserted(Queue *);
    void queueDeleted(Queue *);
    void currentChanged(Queue *);
    void percent(Queue *, int);

protected:
    static QMap<QString, Queue*> _queues;
    static Queue *               _current;
    static QueueManager *        _self;
    static int                   _nextId;
};

#endif // QUEUE_MGR_H
