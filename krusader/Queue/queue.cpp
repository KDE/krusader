#include "queue.h"
#include <kdebug.h>

Queue::Queue(const QString& name): _name(name)
{
}

Queue::~Queue() 
{
	// TODO: save queue on delete? or just delete jobs
}

void Queue::enqueue(KIO::Job *job)
{
	_jobs.append(job);
	
	dumpQueue();
}

void Queue::dumpQueue()
{
	kDebug() << "Queue: " << name().toLatin1();
}
