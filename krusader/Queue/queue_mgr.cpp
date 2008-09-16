#include "queue_mgr.h"
#include <QList>

const QString QueueManager::defaultName="default";
QMap<QString, Queue*> QueueManager::_queues;
QString QueueManager::_current="default";

QueueManager::QueueManager()
{
	Queue *defaultQ = new Queue(defaultName);
	_queues.insert(defaultQ->name(), defaultQ);
	_current = defaultName;
}

QueueManager::~QueueManager() 
{
	QMap<QString, Queue*>::iterator it;
 	for (it = _queues.begin(); it != _queues.end(); ++it )
 		delete it.value();
	_queues.clear();
}

Queue* QueueManager::queue(const QString& queueName)
{
	if (!_queues.contains(queueName))
		return 0;
	return _queues[queueName];
}

QList<QString> QueueManager::queues() const
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
