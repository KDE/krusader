/***************************************************************************
                      virtualcopyjob.cpp  -  description
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

#include "virtualcopyjob.h"
#include "vfs.h"
#include "vfile.h"
#include "../krusader.h"
#include <kio/observer.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/directorysizejob.h>

#define REPORT_TIMEOUT 200

VirtualCopyJob::VirtualCopyJob( const QStringList *names, vfs * vfs, const KUrl& dest, const KUrl& baseURL, 
	PreserveMode pmode, KIO::CopyJob::CopyMode mode, bool asMethod, bool showProgressInfo ) :
		KIO::Job( showProgressInfo ), m_totalSize( 0 ), m_totalFiles( 0 ), m_totalSubdirs( 0 ),
		m_processedSize( 0 ), m_processedFiles( 0 ), m_processedSubdirs( 0 ), m_tempSize( 0 ), m_tempFiles( 0 ),
		m_tempSubdirs( 0 ), m_dirsToGetSize(), m_filesToCopy(), m_size(), m_filenum(), m_subdirs(), m_baseURL( baseURL ),
		m_dest( dest ), m_pmode( pmode ), m_mode( mode ), m_asMethod( asMethod ), m_showProgressInfo( showProgressInfo ),
		m_state( ST_STARTING ), m_reportTimer(), m_current(), m_currentDir(), m_dirStack() {
	
	m_filesToCopy.setAutoDelete( true );
	m_dest.adjustPath( KUrl::AddTrailingSlash );
	
	vfile * file = vfs->vfs_getFirstFile();
	while( file ) {
		if( names->contains( file->vfile_getName() ) ) {
			QString relativeDir = KUrl::relativeUrl( baseURL, file->vfile_getUrl().upUrl() );
			
			KUrl::List *list = m_filesToCopy.find( relativeDir );
			if( list == 0 ) {
				list = new KUrl::List();
				m_filesToCopy.insert( relativeDir, list );
				// initialize the dir content
				m_size[ relativeDir ] = 0;
				m_filenum[ relativeDir ] = 0;
				m_subdirs[ relativeDir ] = 0;
			}
			
			if( !list->contains( file->vfile_getUrl() ) ) {
				if( file->vfile_isDir() ) {
					m_dirsToGetSize.append( file->vfile_getUrl() );
					m_totalSubdirs++;
					m_subdirs[ relativeDir ]++;
				} else {
					m_totalFiles++;
					m_filenum[ relativeDir ]++;
					m_totalSize += file->vfile_getSize();
					m_size[ relativeDir ] += file->vfile_getSize();
				}
				list->append( file->vfile_getUrl() );
			}
		}
		file = vfs->vfs_getNextFile();
	}
	
	if ( showProgressInfo ) {
		connect( this, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
			Observer::self(), SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
		connect( this, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
			Observer::self(), SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );
		connect( this, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
			Observer::self(), SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
		connect( this, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
			Observer::self(), SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );
		connect( this, SIGNAL( percent( KIO::Job*, unsigned long ) ),
			Observer::self(), SLOT( slotPercent( KIO::Job*, unsigned long ) ) );
	}
	
	QTimer::singleShot( 0, this, SLOT( slotStart() ) );
}

void VirtualCopyJob::slotStart() {
	if( m_showProgressInfo ) {
		if( m_mode == KIO::CopyJob::Move )
			Observer::self()->slotMoving( this, m_baseURL, m_dest );
		else
			Observer::self()->slotCopying( this, m_baseURL, m_dest );
	}
	
	connect(&m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
	m_reportTimer.start(REPORT_TIMEOUT,false);
	
	statNextDir();
}

void VirtualCopyJob::slotReport() {
	switch( m_state ) {
	case ST_CREATING_DIRECTORY:
		if( m_showProgressInfo ) {
			Observer::self()->slotCreatingDir( this, m_current );
			Observer::self()->slotProcessedDirs( this, m_processedSubdirs );
			emit processedDirs( this, m_processedSubdirs );
		}
		break;
	case ST_CALCULATING_TOTAL_SIZE:
		emit totalSize( this, m_totalSize );
		emit totalDirs( this, m_totalSubdirs );
		emit totalFiles( this, m_totalFiles );
		break;
	case ST_COPYING:
		{
			emit processedDirs( this, m_processedSubdirs + m_tempSubdirs );
			emit processedFiles( this, m_processedFiles + m_tempFiles );
			setProcessedSize( m_processedSize + m_tempSize );
			emit processedSize( this, m_processedSize + m_tempSize );
			double percDbl = ((double)( m_processedSize + m_tempSize )/(double)m_totalSize) * 100. + 0.5;
			unsigned long perc = (long)percDbl;
			if( perc > 100 )
				perc = 100;
			emit percent( this, perc );
			break;
		}
	default:
		break;
	}
}

void VirtualCopyJob::statNextDir() {
	m_state = ST_CALCULATING_TOTAL_SIZE;
	
	if( m_dirsToGetSize.count() == 0 ) {
		slotReport(); // report the total size values
		createNextDir();
		return;
	}
	KUrl dirToCheck = m_dirsToGetSize.first();
	m_dirsToGetSize.pop_front();
	
	m_currentDir = KUrl::relativeUrl( m_baseURL, dirToCheck.upUrl() );
	
	KIO::DirectorySizeJob* kds  = KIO::directorySize( dirToCheck );
	connect( kds, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotKdsResult( KIO::Job* ) ) );
}

void VirtualCopyJob::slotKdsResult( KIO::Job * job ) {
	KIO::DirectorySizeJob* kds = static_cast<DirectorySizeJob*>(job);
	m_totalSize += kds->totalSize();
	m_totalFiles += kds->totalFiles();
	m_totalSubdirs += kds->totalSubdirs();
	
	m_size[ m_currentDir ] += kds->totalSize();
	m_filenum[ m_currentDir ] += kds->totalFiles();
	m_subdirs[ m_currentDir ] += kds->totalSubdirs();
	statNextDir();
}

void VirtualCopyJob::createNextDir() {
	m_state = ST_CREATING_DIRECTORY;
	
	if( m_filesToCopy.count() == 0 ) {
		emitResult();
		return;
	}
	
	Q3DictIterator<KUrl::List> diter( m_filesToCopy );
	
	m_currentDir = diter.currentKey();
	m_current = m_dest;
	if( m_currentDir != "./" && !m_currentDir.isEmpty() )
		m_current.addPath( m_currentDir );
	
	KIO::Job *job = KIO::stat( m_current );
	connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
}

void VirtualCopyJob::slotStatResult( KIO::Job *job ) {
	KUrl url = (static_cast<KIO::SimpleJob*>(job) )->url();
	
	if ( job && job->error() ) {
		if( job->error() == KIO::ERR_DOES_NOT_EXIST && !url.equals( url.upUrl(),KUrl::CompareWithoutTrailingSlash ) ) {
			m_dirStack.push_back( url.fileName() );
			KIO::Job *job = KIO::stat( url.upUrl() );
			connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
			return;
		}
		job->showErrorDialog( krApp );
		directoryFinished( m_currentDir );
		createNextDir();
		return;
	}
	
	if( m_dirStack.count() ) {
		url.adjustPath( KUrl::AddTrailingSlash );
		url.addPath( m_dirStack.last() );
		m_dirStack.pop_back();
		
		KIO::Job *mkdir_job = KIO::mkdir( url );
		connect( mkdir_job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotMkdirResult( KIO::Job* ) ) );
	}
	else
		copyCurrentDir();
}

void VirtualCopyJob::slotMkdirResult( KIO::Job *job ) {
	KUrl url = (static_cast<KIO::SimpleJob*>(job) )->url();
	
	if ( job && job->error() ) {
		job->showErrorDialog( krApp );
		directoryFinished( m_currentDir );
		createNextDir();
		return;
	}
	
	if( m_dirStack.count() ) {
		url.adjustPath( KUrl::AddTrailingSlash );
		url.addPath( m_dirStack.last() );
		m_dirStack.pop_back();
	
		KIO::Job *mkdir_job = KIO::mkdir( url );
		connect( mkdir_job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotMkdirResult( KIO::Job* ) ) );
	}
	else
		copyCurrentDir();
}

void VirtualCopyJob::copyCurrentDir() {
	m_state = ST_COPYING;
	
	KIO::CopyJob * copy_job = PreservingCopyJob::createCopyJob( m_pmode, *m_filesToCopy[ m_currentDir ], m_current,
		m_mode, m_asMethod, false );
	
	connect( copy_job, SIGNAL( copying(KIO::Job *, const KUrl &, const KUrl &) ),
		this, SLOT( slotCopying(KIO::Job *, const KUrl &, const KUrl &) ) );
	connect( copy_job, SIGNAL( moving(KIO::Job *, const KUrl &, const KUrl &) ),
		this, SLOT( slotMoving(KIO::Job *, const KUrl &, const KUrl &) ) );
	connect( copy_job, SIGNAL( creatingDir(KIO::Job *, const KUrl &) ),
		this, SLOT( slotCreatingDir(KIO::Job *, const KUrl &) ) );
	connect( copy_job, SIGNAL( processedFiles (KIO::Job *, unsigned long) ),
		this, SLOT( slotProcessedFiles (KIO::Job *, unsigned long) ) );
	connect( copy_job, SIGNAL( processedDirs (KIO::Job *, unsigned long) ),
		this, SLOT( slotProcessedDirs (KIO::Job *, unsigned long) ) );
	connect( copy_job, SIGNAL( processedSize (KIO::Job *, KIO::filesize_t) ),
		this, SLOT( slotProcessedSize (KIO::Job *, KIO::filesize_t) ) );
	connect( copy_job, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotCopyResult( KIO::Job* ) ) );
}

void VirtualCopyJob::slotCopyResult( KIO::Job *job ) {
	if ( job && job->error() ) {
		job->showErrorDialog( krApp );
	}
	
	directoryFinished( m_currentDir );
	createNextDir();
}

void VirtualCopyJob::directoryFinished( const QString &name ) {
	m_filesToCopy.remove( name );
	
	m_processedSize += m_size[ name ];
	m_processedFiles += m_filenum[ name ];
	m_processedSubdirs += m_subdirs[ name ];
	
	m_tempSize = m_tempFiles = m_tempSubdirs = 0;
	
	m_size.remove( name );
	m_filenum.remove( name );
	m_subdirs.remove( name );
}

void VirtualCopyJob::slotCopying(KIO::Job *, const KUrl &from, const KUrl &to) {
	if( m_showProgressInfo )
		Observer::self()->slotCopying( this, from, to );
}

void VirtualCopyJob::slotMoving(KIO::Job *, const KUrl &from, const KUrl &to) {
	if( m_showProgressInfo )
		Observer::self()->slotMoving( this, from, to );
}

void VirtualCopyJob::slotCreatingDir(KIO::Job *, const KUrl &to) {
	if( m_showProgressInfo )
		Observer::self()->slotCreatingDir( this, to );
}
  
void VirtualCopyJob::slotProcessedFiles (KIO::Job *, unsigned long filenum) {
	m_tempFiles = filenum;
}

void VirtualCopyJob::slotProcessedDirs (KIO::Job *, unsigned long subdirs) {
	m_tempSubdirs = subdirs;
}

void VirtualCopyJob::slotProcessedSize (KIO::Job *, KIO::filesize_t size) {
	m_tempSize = size;
}

#include "virtualcopyjob.moc"
