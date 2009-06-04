#ifndef QUEUEWIDGET_H
#define QUEUEWIDGET_H

#include <QtCore/QPointer>

#include <ktabwidget.h>

#include "queue.h"
#include "../GUI/krlistwidget.h"

class KrQueueListWidget;

class QueueWidget: public KTabWidget
{
	Q_OBJECT
public:
	QueueWidget( QWidget * parent = 0 );
	~QueueWidget();
	
	void deleteCurrent();
	
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

class KrQueueListWidget : public KrListWidget
{
	Q_OBJECT
public:
	KrQueueListWidget( Queue * queue, QWidget * parent );
	Queue * queue() { return _queue; }
	void deleteItem( QListWidgetItem * item );

public slots:
	void slotChanged();
	void slotItemRightClicked( QListWidgetItem * );
	
signals:
	void stateChanged();
	
private:
	QPointer<Queue> _queue;
};


#endif // QUEUE_WIDGET_H
