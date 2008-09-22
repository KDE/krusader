#ifndef QUEUE_WIDGET_H
#define QUEUE_WIDGET_H

#include <ktabwidget.h>

class QueueWidget: public KTabWidget
{
	Q_OBJECT
public:
	QueueWidget( QWidget * parent = 0 );
	~QueueWidget();
};

#endif // QUEUE_WIDGET_H
