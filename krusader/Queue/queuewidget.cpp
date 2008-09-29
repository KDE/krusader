#include "queuewidget.h"
#include "queue_mgr.h"

QueueWidget::QueueWidget( QWidget * parent ): KTabWidget( parent )
{
  QList<QString> queueList = QueueManager::queues();
  foreach( QString name, queueList ) {
    KrQueueListWidget * jobWidget = new KrQueueListWidget( QueueManager::queue( name ), this );
    connect( jobWidget, SIGNAL( stateChanged() ), this, SIGNAL( currentChanged() ) );
    addTab( jobWidget, name );
    _queueWidgets[ name ] = jobWidget;
  }

  connect( QueueManager::instance(), SIGNAL( queueInserted( Queue * ) ), this, SLOT( slotQueueAdded( Queue * ) ) );
  connect( QueueManager::instance(), SIGNAL( queueDeleted( Queue * ) ), this, SLOT( slotQueueDeleted( Queue * ) ) );
  connect( QueueManager::instance(), SIGNAL( currentChanged( Queue * ) ), this, SLOT( slotCurrentChanged( Queue * ) ) );

  Queue * current = QueueManager::currentQueue();
  if( current )
  {
    QString name = current->name();
    setCurrentWidget( _queueWidgets[ name ] );
  }

  connect( this, SIGNAL( currentChanged ( int ) ), this, SLOT( slotCurrentChanged ( int ) ) );
}


QueueWidget::~QueueWidget()
{
}

void QueueWidget::slotQueueAdded( Queue * queue )
{
  KrQueueListWidget * jobWidget = new KrQueueListWidget( queue, this );
  connect( jobWidget, SIGNAL( stateChanged() ), this, SIGNAL( currentChanged() ) );
  addTab( jobWidget, queue->name() );
  _queueWidgets[ queue->name() ] = jobWidget;
  setCurrentWidget( jobWidget );
}

void QueueWidget::slotQueueDeleted( Queue * queue )
{
  KrQueueListWidget * queueWidget = _queueWidgets[ queue->name() ];
  _queueWidgets.remove( queue->name() );
  int index = indexOf( queueWidget );
  removeTab( index );
  delete queueWidget;
}

void QueueWidget::slotCurrentChanged( int index )
{
  KrQueueListWidget * queueWidget = dynamic_cast<KrQueueListWidget *>( widget( index ) );
  if( queueWidget->queue() )
    QueueManager::setCurrentQueue( queueWidget->queue() );

  emit currentChanged();
}

void QueueWidget::slotCurrentChanged( Queue * queue )
{
  KrQueueListWidget * queueWidget = _queueWidgets[ queue->name() ];
  setCurrentWidget( queueWidget );
}

KrQueueListWidget::KrQueueListWidget( Queue * queue, QWidget * parent ) : QListWidget( parent ), 
                                      _queue( queue )
{
  connect( queue, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
  connect( queue, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
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
