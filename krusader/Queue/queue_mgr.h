#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "queue.h"
#include <qmap.h>

class QueueManager
{
	static const QString defaultName;
public:
	QueueManager();
	~QueueManager();
	
	static Queue* queue(const QString& queueName=defaultName);

protected:
	static QMap<QString, Queue*> _queues;
};

#endif // QUEUE_MGR_H
