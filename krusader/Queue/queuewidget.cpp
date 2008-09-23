#include "queuewidget.h"
#include "queue_mgr.h"

QueueWidget::QueueWidget( QWidget * parent ): KTabWidget( parent )
{
  QList<QString> queueList = QueueManager::queues();
  foreach( QString name, queueList ) {
    KrQueueListWidget * jobWidget = new KrQueueListWidget( QueueManager::queue( name ), this );
    addTab( jobWidget, name );
  }
}


QueueWidget::~QueueWidget()
{
}


KrQueueListWidget::KrQueueListWidget( Queue * queue, QWidget * parent ) : QListWidget( parent ), 
                                      _queue( queue )
{
  connect( queue, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
  slotChanged();
}

void KrQueueListWidget::slotChanged()
{
  clear();
  if( _queue )
  {
    QList<QString> itdescs = _queue->itemDescriptions();
    foreach( QString desc, itdescs )
      addItem( desc );
  }
}
