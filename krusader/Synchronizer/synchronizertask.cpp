/***************************************************************************
                     synchronizertask.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#include "synchronizer.h"
#include "synchronizertask.h"
#include "synchronizerfileitem.h"
#include "synchronizerdirlist.h"
#include <qtimer.h>
#include <qfile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "../VFS/vfs.h"

CompareTask::CompareTask( SynchronizerFileItem *parentIn, const QString &leftURL,
                          const QString &rightURL, const QString &leftDir,
                          const QString &rightDir, bool hidden ) : SynchronizerTask (),  m_parent( parentIn ),
                          m_url( leftURL ), m_dir( leftDir ), m_otherUrl( rightURL ),
                          m_otherDir( rightDir ), m_duplicate( true ),
                          m_dirList( 0 ), m_otherDirList( 0 ) {
  ignoreHidden = hidden;
}

CompareTask::CompareTask( SynchronizerFileItem *parentIn, const QString &urlIn,
                          const QString &dirIn, bool isLeftIn, bool hidden ) : SynchronizerTask (),
                          m_parent( parentIn ), m_url( urlIn ), m_dir( dirIn ),
                          m_isLeft( isLeftIn ), m_duplicate( false ),
                          m_dirList( 0 ), m_otherDirList( 0 ) {
  ignoreHidden = hidden;
}

CompareTask::~CompareTask() {
  if( m_dirList ) {
    delete m_dirList;
    m_dirList = 0;
  }
  if( m_otherDirList ) {
    delete m_otherDirList;
    m_otherDirList = 0;
  }
}

void CompareTask::start() {
  if( m_state == ST_STATE_NEW ) {
    m_state = ST_STATE_PENDING;
    m_loadFinished = m_otherLoadFinished = false;

    m_dirList = new SynchronizerDirList( parentWidget, ignoreHidden );
    connect( m_dirList, SIGNAL( finished( bool ) ), this, SLOT( slotFinished( bool ) ));
    m_dirList->load( m_url, false );

    if( m_duplicate ) {
      m_otherDirList = new SynchronizerDirList( parentWidget, ignoreHidden );
      connect( m_otherDirList, SIGNAL( finished( bool ) ), this, SLOT( slotOtherFinished( bool ) ));
      m_otherDirList->load( m_otherUrl, false );
    }
  }
}

void CompareTask::slotFinished( bool result ) {
  if( !result ) {
    m_state = ST_STATE_ERROR;
    return;
  }
  m_loadFinished = true;

  if( m_otherLoadFinished || !m_duplicate ) 
    m_state = ST_STATE_READY;
}


void CompareTask::slotOtherFinished( bool result ) {
  if( !result ) {
    m_state = ST_STATE_ERROR;
    return;
  }
  m_otherLoadFinished = true;

  if( m_loadFinished )
    m_state = ST_STATE_READY;
}

CompareContentTask::CompareContentTask( Synchronizer *syn, SynchronizerFileItem *itemIn, const KUrl &leftURLIn,
                                        const KUrl &rightURLIn, KIO::filesize_t sizeIn ) : SynchronizerTask(), 
                                        leftURL( leftURLIn ), rightURL( rightURLIn ),
                                        size( sizeIn ), errorPrinted(false), leftReadJob( 0 ),
                                        rightReadJob( 0 ), compareArray(), item( itemIn ), timer( 0 ),
                                        leftFile( 0 ), rightFile( 0 ), received( 0 ), sync( syn ) {
}

CompareContentTask::~CompareContentTask() {
  abortContentComparing();

  if( timer )
    delete timer;
  if( leftFile )
    delete leftFile;
  if( rightFile )
    delete rightFile;
}

void CompareContentTask::start() {
  m_state = ST_STATE_PENDING;

  if( leftURL.isLocalFile() && rightURL.isLocalFile() ) {
    leftFile = new QFile( leftURL.path() );
    if( !leftFile->open( QIODevice::ReadOnly ) ) {
      KMessageBox::error(parentWidget, i18n("Error at opening %1!", leftURL.path() ));
      m_state = ST_STATE_ERROR;
      return;
    }

    rightFile = new QFile( rightURL.path() );
    if( !rightFile->open( QIODevice::ReadOnly ) ) {
      KMessageBox::error(parentWidget, i18n("Error at opening %1!", rightURL.path() ));
      m_state = ST_STATE_ERROR;
      return;
    }

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(sendStatusMessage()) );
    timer->setSingleShot( true );
    timer->start( 1000 );

    localFileCompareCycle();
  } else {
    leftReadJob = KIO::get( leftURL, KIO::NoReload, KIO::HideProgressInfo );
    rightReadJob = KIO::get( rightURL, KIO::NoReload, KIO::HideProgressInfo );

    connect(leftReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotDataReceived(KIO::Job *, const QByteArray &)));
    connect(rightReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(slotDataReceived(KIO::Job *, const QByteArray &)));
    connect(leftReadJob, SIGNAL(result(KJob*)),
            this, SLOT(slotFinished(KJob *)));
    connect(rightReadJob, SIGNAL(result(KJob*)),
            this, SLOT(slotFinished(KJob *)));

    rightReadJob->suspend();

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(sendStatusMessage()) );
    timer->setSingleShot( true );
    timer->start( 1000 );
  }
}

void CompareContentTask::localFileCompareCycle() {

  bool different = false;

  char leftBuffer[ 1440 ]; 
  char rightBuffer[ 1440 ];

  QTime timer;
  timer.start();

  int cnt = 0;

  while ( !leftFile->atEnd() && !rightFile->atEnd() )
  {
    int leftBytes = leftFile->read( leftBuffer, sizeof( leftBuffer ) );
    int rightBytes = rightFile->read( rightBuffer, sizeof( rightBuffer ) );
    
    if( leftBytes != rightBytes ) {
      different = true;
      break;
    }

    if( leftBytes <= 0 )
      break;

    received += leftBytes;

    if( memcmp( leftBuffer, rightBuffer, leftBytes ) ) {
      different = true;
      break;
    }

    if( (++cnt % 16) == 0 && timer.elapsed() >= 250 )
      break;
  }

  if( different ) {
    sync->compareContentResult( item, false );
    m_state = ST_STATE_READY;
    return;
  }

  if( leftFile->atEnd() && rightFile->atEnd() ) {
    sync->compareContentResult( item, true );
    m_state = ST_STATE_READY;
    return;
  }

  QTimer::singleShot( 0, this, SLOT( localFileCompareCycle() ) );
}


void CompareContentTask::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
  int bufferLen = compareArray.size();
  int dataLen   = data.size();

  if( job == leftReadJob )
    received += dataLen;

  do
  {
    if( bufferLen == 0 )
    {
      compareArray.duplicate( data.data(), dataLen );
      break;
    }

    int minSize   = ( dataLen < bufferLen ) ? dataLen : bufferLen;

    for( int i = 0; i != minSize; i++ )
      if( data[i] != compareArray[i] )
      {
        abortContentComparing();
        return;
      }

    if( minSize == bufferLen )
    {
      compareArray.duplicate( data.data() + bufferLen, dataLen - bufferLen );
      if( dataLen == bufferLen )
        return;
      break;
    }
    else
    {
      compareArray.duplicate( compareArray.data() + dataLen, bufferLen - dataLen );
      return;
    }

  }while ( false );

  KIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;

  if( otherJob == 0 )
  {
    if( compareArray.size() )
      abortContentComparing();
  }
  else
  {
    if( !((KIO::TransferJob *)job)->isSuspended() )
    {
      ((KIO::TransferJob *)job)->suspend();
      otherJob->resume();
    }
  }
}

void CompareContentTask::slotFinished(KJob *job)
{
  KIO::TransferJob *otherJob = ( job == leftReadJob ) ? rightReadJob : leftReadJob;

  if( job == leftReadJob )
    leftReadJob = 0;
  else
    rightReadJob = 0;

  if( otherJob )
    otherJob->resume();

  if( job->error() )
  {
    timer->stop();
    abortContentComparing();
  }

  if( job->error() && job->error() != KIO::ERR_USER_CANCELED && !errorPrinted )
  {
    errorPrinted = true;
    KMessageBox::error(parentWidget, i18n("IO error at comparing file %1 with %2!",
                       leftURL.pathOrUrl(), rightURL.pathOrUrl() ) );
  }

  if( leftReadJob == 0 && rightReadJob == 0 )
  {
    if( !compareArray.size() )
      sync->compareContentResult( item, true );
    else
      sync->compareContentResult( item, false );

    m_state = ST_STATE_READY;
  }
}

void CompareContentTask::abortContentComparing()
{
  if( timer )
    timer->stop();

  if( leftReadJob )
    leftReadJob->kill();
  if( rightReadJob )
    rightReadJob->kill();

  if( item->task() >= TT_UNKNOWN )
    sync->compareContentResult( item, false );

  m_state = ST_STATE_READY;
}

void CompareContentTask::sendStatusMessage()
{
  double perc = (size == 0) ? 1. : (double)received / (double)size;
  int percent = (int)(perc * 10000. + 0.5);
  QString statstr = QString( "%1.%2%3" ).arg( percent / 100 ).arg( ( percent / 10 )%10 ).arg( percent % 10 ) + "%";
  setStatusMessage( i18n( "Comparing file %1 (%2)...", leftURL.fileName(), statstr ) );
  timer->setSingleShot( true );
  timer->start( 500 );
}

#include "synchronizertask.moc"
