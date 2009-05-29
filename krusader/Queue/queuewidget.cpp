#include "queuewidget.h"
#include "queue_mgr.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <kmenu.h>
#include <qcursor.h>

QueueWidget::QueueWidget( QWidget * parent ): KTabWidget( parent )
{
  QList<QString> queueList = QueueManager::queues();
  foreach( const QString &name, queueList ) {
    KrQueueListWidget * jobWidget = new KrQueueListWidget( QueueManager::queue( name ), this );
    connect( jobWidget, SIGNAL( stateChanged() ), this, SIGNAL( currentChanged() ) );
    addTab( jobWidget, name );
    _queueWidgets[ name ] = jobWidget;
  }

  connect( QueueManager::instance(), SIGNAL( queueInserted( Queue * ) ), this, SLOT( slotQueueAdded( Queue * ) ) );
  connect( QueueManager::instance(), SIGNAL( queueDeleted( Queue * ) ), this, SLOT( slotQueueDeleted( Queue * ) ) );
  connect( QueueManager::instance(), SIGNAL( currentChanged( Queue * ) ), this, SLOT( slotCurrentChanged( Queue * ) ) );
  connect( this, SIGNAL( currentChanged ( int ) ), this, SLOT( slotCurrentChanged ( int ) ) );

  Queue * current = QueueManager::currentQueue();
  if( current )
  {
    QString name = current->name();
    setCurrentWidget( _queueWidgets[ name ] );
  }
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

void QueueWidget::deleteCurrent()
{
  QWidget * current = currentWidget();
  if( current )
  {
    KrQueueListWidget * lw = (KrQueueListWidget *)current;
    QListWidgetItem * lvitem = lw->currentItem();
    if( lvitem == 0 )
      lvitem = lw->item( 0 );
    if( lvitem )
      lw->deleteItem( lvitem );
  }
}

class KrQueueListWidgetItem : public QListWidgetItem
{
public:
  KrQueueListWidgetItem( const QString & label_, KIOJobWrapper * job_ ) : 
    QListWidgetItem( label_ ), _job( job_ )
  {
    setToolTip( job_->toolTip() );
  }

  KIOJobWrapper * job() { return _job; }

private:
  QPointer<KIOJobWrapper> _job;
};

KrQueueListWidget::KrQueueListWidget( Queue * queue, QWidget * parent ) : KrListWidget( parent ), 
                                      _queue( queue )
{
  connect( queue, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
  connect( queue, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
  connect( this, SIGNAL( itemRightClicked( QListWidgetItem *, const QPoint & ) ),
           this, SLOT( slotItemRightClicked( QListWidgetItem * ) ) );
  slotChanged();
}

void KrQueueListWidget::slotChanged()
{
  clear();
  if( _queue )
  {
    QList<QString> itdescs = _queue->itemDescriptions();
    QList<KIOJobWrapper *> items = _queue->items();
    for( int i=0; i != items.count(); i++ )
      addItem( new KrQueueListWidgetItem( itdescs[ i ], items[ i ] ) );
  }
}

void KrQueueListWidget::slotItemRightClicked( QListWidgetItem * item )
{
  if( item )
  {
    KrQueueListWidgetItem * kitem = (KrQueueListWidgetItem * )item;
    if( kitem->job() )
    {
      KMenu popup( this );
      popup.setTitle( i18n("Queue Manager"));
      QAction * actDelete = popup.addAction( i18n("Delete") );
      QAction * res = popup.exec( QCursor::pos() );

      if( res == actDelete )
        deleteItem( kitem );
    }
  }
}

void KrQueueListWidget::deleteItem( QListWidgetItem * item )
{
  KrQueueListWidgetItem * kitem = (KrQueueListWidgetItem * )item;
  if( kitem->job() && kitem->job()->isStarted() )
  {
    if( KMessageBox::warningContinueCancel(this,
       i18n("Deleting this job will abort the pending job. Do you wish to continue?"),
       i18n("Warning") ) != KMessageBox::Continue )
       return;
  }
  if( kitem->job() )
    _queue->remove( kitem->job() );
}
