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

KrPreviewPopup::KrPreviewPopup(): id(1){}

void KrPreviewPopup::setUrls(const KURL::List* urls){
	KIO::PreviewJob* pjob;
	QStringList plugins = KIO::PreviewJob::availablePlugins();

	for( unsigned int i=0; i< urls->count(); ++i){
		KFileItem* kfi = new KFileItem(KFileItem::Unknown,KFileItem::Unknown,*(urls->at(i)));
		files.append(kfi);
	}

	pjob = new KIO::PreviewJob(files,200,200,200,1,true,true,0);
	connect(pjob,SIGNAL(gotPreview(const KFileItem*,const QPixmap&)),
          this,SLOT(addPreview(const KFileItem*,const QPixmap&)));
}

KrPreviewPopup::~KrPreviewPopup(){}

void KrPreviewPopup::addPreview(const KFileItem* file,const QPixmap& preview){
  insertItem(file->text());
	insertItem(preview,id);
	insertSeparator();
}
