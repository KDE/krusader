#ifndef QUEUE_H
#define QUEUE_H

#include <qobject.h>
#include "../VFS/kiojobwrapper.h"
#include <QList>

class KJob;

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
	Queue(const QString& name);
	virtual ~Queue();
	
	inline const QString& name() const { return _name; }
	void enqueue(KIOJobWrapper *job);
	int  count() { return _jobs.size(); }
	bool isSuspended() { return _suspended; }
	
	QList<QString> itemDescriptions();
	QList<KIOJobWrapper *> items();
	
public slots:
	void suspend();
	void resume();

protected slots:
	void slotJobDestroyed( QObject * );
	void slotResult( KJob * );
	
protected:
	void dumpQueue();

	QString _name;
	QList<KIOJobWrapper *> _jobs;
	bool _suspended;

signals:
	void showQueueDialog();
	void changed();
	void emptied();
};

#endif // QUEUE_H
