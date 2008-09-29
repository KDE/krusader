#ifndef QUEUE_WIDGET_H
#define QUEUE_WIDGET_H

#include "queue.h"
#include <ktabwidget.h>
#include <qlistwidget.h>
#include <qpointer.h>

class KrQueueListWidget;

class QueueWidget: public KTabWidget
{
	Q_OBJECT
public:
	QueueWidget( QWidget * parent = 0 );
	~QueueWidget();
	
protected slots:
	void slotQueueAdded( Queue * );
	void slotQueueDeleted( Queue * );
	void slotCurrentChanged( Queue * );
	void slotCurrentChanged( int );

signals:
	void currentChanged();

private:
	QMap<QString, KrQueueListWidget*> _queueWidgets;
};

class KrQueueListWidget : public QListWidget
{
	Q_OBJECT
public:
	KrQueueListWidget( Queue * queue, QWidget * parent );
	Queue * queue() { return _queue; }

public slots:
	void slotChanged();
	
signals:
	void stateChanged();
	
private:
	QPointer<Queue> _queue;
};


#endif // QUEUE_WIDGET_H
