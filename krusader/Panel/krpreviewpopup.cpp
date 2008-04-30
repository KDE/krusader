/***************************************************************************
                          krpreviewpopup.cpp  -  description
                             -------------------
    begin                : Sun Dec 29 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
    web site             : http://krusader.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kio/previewjob.h>
#include "krpreviewpopup.h"
#include <kdebug.h>
#include <klocale.h>
#include "../KViewer/krviewer.h"
#include "../krusader.h"
#include <QPixmap>

KrPreviewPopup::KrPreviewPopup(): prevNotAvailAction( 0 ), id(1),noPreview(true){
	connect(this,SIGNAL(triggered(QAction *)),this,SLOT(view(QAction *)));
}

void KrPreviewPopup::setUrls(const KUrl::List* urls){
	if( prevNotAvailAction ) {
		removeAction( prevNotAvailAction );
		delete prevNotAvailAction;
	}
	prevNotAvailAction = addAction(i18n("Preview not available"));

	KIO::PreviewJob* pjob;
	QStringList plugins = KIO::PreviewJob::availablePlugins();

	for( int i=0; i< urls->count(); ++i){
		files.push_back( KFileItem(KFileItem::Unknown,KFileItem::Unknown,(*urls)[ i ] ) );
	}

	pjob = new KIO::PreviewJob(files,200,200,200,1,true,true,0);
	connect(pjob,SIGNAL(gotPreview(const KFileItem&,const QPixmap&)),
          this,SLOT(addPreview(const KFileItem&,const QPixmap&)));
}

KrPreviewPopup::~KrPreviewPopup() {
	if( prevNotAvailAction )
		delete prevNotAvailAction;
	prevNotAvailAction = 0;
}

void KrPreviewPopup::addPreview(const KFileItem& file,const QPixmap& preview){
	if(noPreview){
		if( prevNotAvailAction ) {
			removeAction( prevNotAvailAction );
			delete prevNotAvailAction;
			prevNotAvailAction = 0;
		}
		noPreview = false;
	}
	addAction(preview, QString())->setData( QVariant(id) );
	addAction(file.text())->setData( QVariant(id++) );
	addSeparator();
	availablePreviews.push_back(file.url());
}

void KrPreviewPopup::view(QAction *clicked){
	if( clicked && clicked->data().canConvert<int>() ) {
		int id = clicked->data().toInt();
		KUrl url = availablePreviews[ id-1 ];
		KrViewer::view(url);
	}
}

#include "krpreviewpopup.moc"
