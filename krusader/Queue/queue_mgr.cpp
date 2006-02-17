#include "queue_mgr.h"

const QString QueueManager::defaultName="default";
QMap<QString, Queue*> QueueManager::_queues;

QueueManager::QueueManager()
{
	Queue *defaultQ = new Queue();
	_queues.insert(defaultName, defaultQ);
}

QueueManager::~QueueManager() 
{
	// TODO: delete all queues
}

Queue* QueueManager::queue(const QString& queueName)
{
	if (!_queues.contains(queueName))
		return 0;
	return _queues[queueName];
}

