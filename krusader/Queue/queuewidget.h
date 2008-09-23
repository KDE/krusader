#ifndef QUEUE_WIDGET_H
#define QUEUE_WIDGET_H

#include "queue.h"
#include <ktabwidget.h>
#include <qlistwidget.h>
#include <qpointer.h>

class QueueWidget: public KTabWidget
{
	Q_OBJECT
public:
	QueueWidget( QWidget * parent = 0 );
	~QueueWidget();
};

class KrQueueListWidget : public QListWidget
{
	Q_OBJECT
public:
	KrQueueListWidget( Queue * queue, QWidget * parent );

public slots:
	void slotChanged();
	
private:
	QPointer<Queue> _queue;
};


#endif // QUEUE_WIDGET_H
