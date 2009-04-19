/***************************************************************************
                     krtrashhandler.cpp  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krtrashhandler.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QByteArray>
#include <QDataStream>
#include <kuiserverjobtracker.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <knotification.h>
#include <kio/jobuidelegate.h>
#include "krusader.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdirwatch.h>
#include "krusader.h"

KrTrashWatcher * KrTrashHandler::_trashWatcher = 0;

bool KrTrashHandler::isTrashEmpty()
{
  KConfig trashConfig( "trashrc" );
  KConfigGroup cfg( &trashConfig, "Status" );
  return cfg.readEntry( "Empty", false );
}

QString KrTrashHandler::trashIcon()
{
  return isTrashEmpty() ? "user-trash" : "user-trash-full";
}

void KrTrashHandler::emptyTrash()
{
  QByteArray packedArgs;
  QDataStream stream( &packedArgs, QIODevice::WriteOnly );
  stream << (int)1;
  KIO::Job *job = KIO::special( KUrl("trash:/"), packedArgs );
  KNotification::event("Trash: emptied", QString() , QPixmap() , 0l, KNotification::DefaultEvent );
  job->ui()->setWindow( krApp );
  QObject::connect( job, SIGNAL( result( KJob * ) ), ACTIVE_PANEL->func, SLOT( refresh() ) );
}

void KrTrashHandler::restoreTrashedFiles( const KUrl::List &urls )
{
    KonqMultiRestoreJob* job = new KonqMultiRestoreJob( urls );
    job->ui()->setWindow( krApp );
    KIO::getJobTracker()->registerJob(job);
    QObject::connect( job, SIGNAL( result( KJob * ) ), ACTIVE_PANEL->func, SLOT( refresh() ) );
}

void KrTrashHandler::startWatcher()
{
    if( !_trashWatcher )
        _trashWatcher = new KrTrashWatcher();
}

void KrTrashHandler::stopWatcher()
{
    delete _trashWatcher;
    _trashWatcher = 0;
}

KonqMultiRestoreJob::KonqMultiRestoreJob( const KUrl::List& urls )
    : KIO::Job(),
      m_urls( urls ), m_urlsIterator( m_urls.begin() ),
      m_progress( 0 )
{
    QTimer::singleShot(0, this, SLOT(slotStart()));
    setUiDelegate(new KIO::JobUiDelegate);
}

void KonqMultiRestoreJob::slotStart()
{
    if ( m_urlsIterator == m_urls.begin() ) // first time: emit total
        setTotalAmount( KJob::Files, m_urls.count() );

    if ( m_urlsIterator != m_urls.end() )
    {
        const KUrl& url = *m_urlsIterator;

        KUrl new_url = url;
        if ( new_url.protocol()=="system"
          && new_url.path().startsWith("/trash") )
        {
            QString path = new_url.path();
	    path.remove(0, 6);
	    new_url.setProtocol("trash");
	    new_url.setPath(path);
        }

        Q_ASSERT( new_url.protocol() == "trash" );
        QByteArray packedArgs;
        QDataStream stream( &packedArgs, QIODevice::WriteOnly );
        stream << (int)3 << new_url;
        KIO::Job* job = KIO::special( new_url, packedArgs, KIO::HideProgressInfo );
        addSubjob( job );
        setProcessedAmount(KJob::Files, processedAmount(KJob::Files) + 1);
    }
    else // done!
    {
        emitResult();
    }
}

void KonqMultiRestoreJob::slotResult( KJob *job )
{
    if ( job->error() )
    {
        KIO::Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }
    removeSubjob(job);
    // Move on to next one
    ++m_urlsIterator;
    ++m_progress;
    //emit processedSize( this, m_progress );
    emitPercent( m_progress, m_urls.count() );
    slotStart();
}


KrTrashWatcher::KrTrashWatcher()
{
    QString trashrcFile = KGlobal::mainComponent().dirs()->saveLocation("config") +
                            QString::fromLatin1("trashrc");
    _watcher = new KDirWatch();
    // connect the watcher
    connect(_watcher,SIGNAL(dirty(const QString&)),this,SLOT(slotDirty(const QString&)));
    connect(_watcher,SIGNAL(created(const QString&)),this, SLOT(slotCreated(const QString&)));
    _watcher->addFile( trashrcFile ); //start trashrc watcher
    _watcher->startScan(true);
}

KrTrashWatcher::~KrTrashWatcher()
{
    delete _watcher;
    _watcher = 0;
}

void KrTrashWatcher::slotDirty(const QString& ) {
    Krusader::actTrashBin->setIcon( KIcon( KrTrashHandler::trashIcon() ) );
}

void KrTrashWatcher::slotCreated(const QString& ) {
    Krusader::actTrashBin->setIcon( KIcon( KrTrashHandler::trashIcon() ) );
}
