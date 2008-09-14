/***************************************************************************
                              kiojobwrapper.cpp
                             -------------------
    copyright            : (C) 2008+ by Csaba Karai
    email                : krusader@users.sourceforge.net
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

#include "kiojobwrapper.h"
#include <qevent.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/directorysizejob.h>
#include <kio/job.h>
#include <qapplication.h>
#include <iostream>

class JobStartEvent : public QEvent {
public:
	JobStartEvent( KIOJobWrapper * wrapperIn ) : QEvent( QEvent::User ),
		m_wrapper( wrapperIn ) {}
	virtual ~JobStartEvent() { delete m_wrapper; }
	
	KIOJobWrapper * wrapper() { return m_wrapper; }
private:
	KIOJobWrapper * m_wrapper;
};

KrJobStarter * KrJobStarter::m_self = 0;

bool KrJobStarter::event( QEvent * e ) {
	if( e->type() == QEvent::User ) {
		JobStartEvent *je = (JobStartEvent *)e;
		je->wrapper()->createJob();
		return true;
	}
	return QObject::event( e );
}

void KIOJobWrapper::createJob() {
	KIO::Job *job = 0;
	switch( m_type ) {
	case Stat:
		job = KIO::stat( m_url );
		break;
	case DirectorySize:
		job = KIO::directorySize( m_url );
		break;
	default:
		fprintf( stderr, "Internal error: invalid job!\n" );
		break;
	}
	if( job ) {
		for( int i=0; i != m_signals.count(); i++ )
			if( !m_receivers[ i ].isNull() )
				connect( job, m_signals[ i ], m_receivers[ i ], m_methods[ i ] );
	}
}

KIOJobWrapper * KIOJobWrapper::stat( KUrl &url ) {
	return new KIOJobWrapper( Stat, url );
}

KIOJobWrapper * KIOJobWrapper::directorySize( KUrl &url ) {
	return new KIOJobWrapper( DirectorySize, url );
}

void KIOJobWrapper::start() {
	KrJobStarter *self = KrJobStarter::self();
	QApplication::postEvent( self, new JobStartEvent( this ) );
}

void KIOJobWrapper::connectTo( const char * signal, const QObject * receiver, const char * method ) {
	m_signals.append( signal );
	m_receivers.append( (QObject *)receiver );
	m_methods.append( method );
}
