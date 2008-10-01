#include "queue_mgr.h"
#include "queuedialog.h"
#include "queue.h"
#include <QList>
#include <klocale.h>

const QString QueueManager::defaultName=i18n( "default" );
QMap<QString, Queue*> QueueManager::_queues;
Queue * QueueManager::_current=0;
QueueManager * QueueManager::_self = 0;

QueueManager::QueueManager()
{
	_self = this;
	createQueue( defaultName );
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
	return _current;
}

void QueueManager::setCurrentQueue(Queue * queue)
{
	if (_queues.contains(queue->name())) {
		_current = queue;
		_self->emit currentChanged( queue );
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
	foreach( Queue * queue, queues ) {
		if( queue->count() != 0 ) {
			empty = false;
			break;
		}
	}
	QueueDialog::everyQueueIsEmpty();
}

Queue * QueueManager::createQueue(const QString& queueName)
{
	if( _queues.contains( queueName ) )
		return 0;
	Queue *queue = new Queue(queueName);
	connect( queue, SIGNAL( showQueueDialog() ), _self, SLOT( slotShowQueueDialog() ) );
	connect( queue, SIGNAL( emptied() ), _self, SLOT( slotQueueEmptied() ) );
	connect( queue, SIGNAL( percent( Queue *, int ) ), _self, SIGNAL( percent( Queue *, int ) ) );
	_queues.insert(queue->name(), queue);
	_current = queue;
	
	_self->emit queueInserted( queue );
	_self->emit currentChanged( queue );
	return queue;
}

void QueueManager::removeQueue( Queue * queue )
{
	if( _queues.count() < 2 && _queues.contains( queue->name() ) )
		return;
	
	_self->emit queueDeleted( queue );
	_queues.remove( queue->name() );
	
	QMap<QString, Queue*>::iterator it;
 	_current = _queues.begin().value();
	_self->emit currentChanged( queue );
	
	delete queue;
}
