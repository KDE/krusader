#include "queue.h"
#include "../krusader.h"
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

Queue::Queue(const QString& name): _name(name)
{
}

Queue::~Queue() 
{
	// TODO: save queue on delete? or just delete jobs
}

void Queue::enqueue(KIOJobWrapper *job)
{
	bool isRunning = _jobs.count();
	_jobs.append(job);
	connect( job, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotJobDestroyed( QObject * ) ) );
	job->connectTo( SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
	
	if( !isRunning )
		job->start();
	
	emit showQueueDialog();
	dumpQueue();
}

void Queue::dumpQueue()
{
	kDebug() << "Queue: " << name().toLatin1();
}

void Queue::slotJobDestroyed( QObject * obj )
{
	KIOJobWrapper * jw = (KIOJobWrapper*) obj;
	bool current = false;
	if( _jobs.count() > 0 && _jobs[ 0 ] == jw )
		current = true;
	_jobs.removeAll( jw );
	if( current && _jobs.count() > 0 )
		_jobs[ 0 ]->start();
}

void Queue::slotResult( KJob * job )
{
	if( _jobs.count() > 0 ) {
		KIOJobWrapper * jw = _jobs[ 0 ];
		KIO::Job * kjob = jw->job();
		if( kjob && kjob == job ) {
			_jobs.removeAll( jw );
			if( job->error() && _jobs.count() > 0 ) {
				// what to do with the other elements in the queue
				QString message = i18n( "An error occurred during processing the queue.\n" );
				if( job->error() == KIO::ERR_USER_CANCELED )
					message = i18n( "The last processed element in the queue was aborted.\n" );
				message += i18n( "Do you wish to continue with the next element, delete the queue or suspend the queue?" );
				int res = KMessageBox::questionYesNoCancel(krApp, message,
				          i18n( "Krusader::Queue" ), KStandardGuiItem::cont(),
				          KStandardGuiItem::del(), KGuiItem( i18n( "Suspend" ) ) );
				switch( res )
				{
				case KMessageBox::Yes: // continue
					break;
				case KMessageBox::No: // delete the queue
					_jobs.clear();
					return;
				default: // suspend
					/* TODO */
					return;
				}
			}
			
			if( _jobs.count() > 0 )
				_jobs[ 0 ]->start();
		}
	}
}
