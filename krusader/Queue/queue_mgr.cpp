#include "queue_mgr.h"
#include "queuedialog.h"
#include <QList>
#include <klocale.h>

const QString QueueManager::defaultName=i18n( "default" );
QMap<QString, Queue*> QueueManager::_queues;
QString QueueManager::_current=QueueManager::defaultName;
QueueManager * QueueManager::_self = 0;

QueueManager::QueueManager()
{
	Queue *defaultQ = new Queue(defaultName);
	connect( defaultQ, SIGNAL( showQueueDialog() ), this, SLOT( slotShowQueueDialog() ) );
	connect( defaultQ, SIGNAL( emptied() ), this, SLOT( slotQueueEmptied() ) );
	_queues.insert(defaultQ->name(), defaultQ);
	_current = defaultName;
	_self = this;
}

QueueManager::~QueueManager() 
{
	QMap<QString, Queue*>::iterator it;
 	for (it = _queues.begin(); it != _queues.end(); ++it )
 		delete it.value();
	_queues.clear();
	QueueDialog::deleteDialog();
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
	return queue( _current );
}

void QueueManager::setCurrentQueue(const QString& queueName)
{
	if (_queues.contains(queueName))
		_current = queueName;
}

void QueueManager::slotShowQueueDialog()
{
	QueueDialog::showDialog();
}

void QueueManager::slotQueueEmptied()
{
	bool empty = true;
	QList<Queue *> queues = _queues.values();
	foreach( Queue * queue, queues ) {
		if( queue->count() != 0 ) {
			empty = false;
			break;
		}
	}
	QueueDialog::everyQueueIsEmpty();
}
