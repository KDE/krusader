#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "queue.h"
#include <qmap.h>

/**
 * QueueManager holds multiple queues and has a static
 * method that fetches a queue by name. calling it with
 * no arguments will fetch the default queue
 */
class QueueManager
{
	static const QString defaultName;
public:
	QueueManager();
	~QueueManager();
	
	static Queue* queue(const QString& queueName=defaultName);
	QValueList<QString> queues() const;

protected:
	static QMap<QString, Queue*> _queues;
};

#endif // QUEUE_MGR_H
