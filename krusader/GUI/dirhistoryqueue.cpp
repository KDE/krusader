/***************************************************************************
                         dirhistoryqueue.cpp  -  description
                            -------------------
   begin                : Thu Jan 1 2004
   copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
   email                : 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dirhistoryqueue.h"
#include "../Panel/listpanel.h"

#include <kdebug.h>

DirHistoryQueue::DirHistoryQueue( ListPanel* p ) {
	panel = p;

	connect( panel, SIGNAL( pathChanged( ListPanel* ) ), this, SLOT( slotPathChanged( ListPanel* ) ) );
}
DirHistoryQueue::~DirHistoryQueue() {}

/** No descriptions */
void DirHistoryQueue::slotPathChanged( ListPanel* p ) {
	KUrl url = p->virtualPath();
	// already in the queue ?
	if(  urlQueue.indexOf( url ) >= 0 ){
		// remove it !
		urlQueue.removeAll( url );
	}
	// do we have room for another ?
	if ( urlQueue.size() > 12 ) {
		// no room - remove the oldest entry
		urlQueue.pop_back();	
	}
	
	urlQueue.push_front( url );
}

#if 0
void DirHistoryQueue::addUrl(const KUrl& url){
	if ( pathQueue.indexOf( path ) == -1 ) {
		if ( pathQueue.size() > 12 ) {
			// remove the oldest entry
			pathQueue.pop_back();
		}
	} else {
		pathQueue.remove( path );
	}

	pathQueue.push_front( path );
}

void DirHistoryQueue::RemovePath( const QString& path ) {
	QStringList::iterator it;
	it = pathQueue.find( path );
	if ( it != pathQueue.end() ) {
		pathQueue.remove( it );
	}
}

bool DirHistoryQueue::checkPath( const QString& path ) {
	KUrl url( path );

	QString p = url.path();
	//  kDebug() << "url:" << p <<  ", file: " << url.fileName() << ", dir: " << url.directory() <<  endl;
	if ( url.protocol() == "file" ) {
		QDir dir( path );
		if ( !dir.exists() ) {
			RemovePath( path );
			return false;
		}
	}

	return true;

}
#endif

#include "dirhistoryqueue.moc"
