#ifndef QUEUE_H
#define QUEUE_H

#include <qobject.h>
#include <kio/jobclasses.h>

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
	Queue();
	virtual ~Queue();

protected:
};

#endif // QUEUE_H
