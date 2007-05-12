#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "queue.h"
#include <qmap.h>
//Added by qt3to4:
#include <Q3ValueList>

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
	Q3ValueList<QString> queues() const;

protected:
	static QMap<QString, Queue*> _queues;
};

#endif // QUEUE_MGR_H
