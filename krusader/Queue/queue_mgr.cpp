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

#include "queue_mgr.h"
#include "queuedialog.h"
#include "queue.h"
#include <QList>
#include <QVector>
#include <QVariant>
#include <klocale.h>
#include "../krglobal.h"

QMap<QString, Queue*> QueueManager::_queues;
Queue * QueueManager::_current = 0;
QueueManager * QueueManager::_self = 0;
int QueueManager::_nextId = 1;

QueueManager::QueueManager()
{
    _self = this;
    QStringList queues;
    queues << defaultName();
    int current = 0;
    KConfigGroup group(krConfig, "QueueManager");
    queues = group.readEntry("Queues", queues);
    current = group.readEntry("Active", current);

    const int queuesSize = queues.size();
    if (queuesSize == 0)
        queues << defaultName();

    QVector<Queue*> queueArray;

    foreach(const QString &queueName, queues)
        queueArray.append(createQueue(queueName));

    if (current < queuesSize)
        setCurrentQueue(queueArray[ current ]);
    else
        setCurrentQueue(queueArray[ 0 ]);
}

QueueManager::~QueueManager()
{
    QList<int>     idList;
    QStringList    nameList;
    int currentId = 0;
    QMap<QString, Queue*>::iterator it;
    for (it = _queues.begin(); it != _queues.end(); ++it) {
        QString name = it.key();
        int id = (*it)->property("QueueID").toInt();
        if ((*it) == currentQueue())
            currentId = id;

        int pos = 0;
        while (pos < idList.count() && idList[ pos ] < id)
            pos++;
        idList.insert(pos, id);
        nameList.insert(pos, name);

        delete it.value();
    }
    _queues.clear();
    QueueDialog::deleteDialog();

    KConfigGroup group(krConfig, "QueueManager");
    group.writeEntry("Queues", nameList);
    int active = 0;
    for (int p = 0; p < idList.count(); p++)
        if (idList[ p ] == currentId) {
            active = p;
            break;
        }
    group.writeEntry("Active", active);
}

QString QueueManager::defaultName()
{
    return QString(i18n("default"));
}

Queue* QueueManager::queue(const QString& queueName)
{
    if (!_queues.contains(queueName))
        return 0;
    return _queues[queueName];
}

QList<QString> QueueManager::queues()
{
    return _queues.keys();
}

Queue* QueueManager::currentQueue()
{
    return _current;
}

void QueueManager::setCurrentQueue(Queue * queue)
{
    if (queue == 0)
        return;

    if (_queues.contains(queue->name())) {
        _current = queue;
        _self->emit currentChanged(queue);
    }
}

void QueueManager::slotShowQueueDialog()
{
    QueueDialog::showDialog();
}

void QueueManager::slotQueueEmptied()
{
    bool empty = true;
    QList<Queue *> queues = _queues.values();
    foreach(Queue * queue, queues) {
        if (queue->count() != 0) {
            empty = false;
            break;
        }
    }
    QueueDialog::everyQueueIsEmpty();
}

Queue * QueueManager::createQueue(const QString& queueName)
{
    if (_queues.contains(queueName))
        return 0;
    Queue *queue = new Queue(queueName);
    int id = _nextId++;
    queue->setProperty("QueueID", QVariant(id));
    connect(queue, SIGNAL(showQueueDialog()), _self, SLOT(slotShowQueueDialog()));
    connect(queue, SIGNAL(emptied()), _self, SLOT(slotQueueEmptied()));
    connect(queue, SIGNAL(percent(Queue *, int)), _self, SIGNAL(percent(Queue *, int)));
    _queues.insert(queue->name(), queue);
    _current = queue;

    _self->emit queueInserted(queue);
    _self->emit currentChanged(queue);
    return queue;
}

void QueueManager::removeQueue(Queue * queue)
{
    if (_queues.count() < 2 && _queues.contains(queue->name()))
        return;

    _self->emit queueDeleted(queue);
    _queues.remove(queue->name());

    QMap<QString, Queue*>::iterator it;
    _current = _queues.begin().value();
    _self->emit currentChanged(queue);

    delete queue;
}
