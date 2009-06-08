/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krpreviewpopup.h"

#include <QPixmap>
#include <QPainter>
#include <QStyle>

#include <kio/previewjob.h>
#include <kdebug.h>
#include <klocale.h>

#include "../KViewer/krviewer.h"
#include "../krusader.h"

KrPreviewPopup::KrPreviewPopup(): prevNotAvailAction( 0 ), id(1),noPreview(true){
	connect(this,SIGNAL(triggered(QAction *)),this,SLOT(view(QAction *)));
	
	maxYSize = QFontMetrics(font()).height() * 12;
	if( maxYSize < 50 )
	  maxYSize = 50;
	
	maxXSize = (int)(( 1.5 * maxYSize ) + 0.5 );
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
	
	double sizeX = preview.width();
	double sizeY = preview.height();
	QFont f = font();
	QString data;
	
	if( sizeX != 0. && sizeY != 0. ) {
		if( sizeY > maxYSize ) {
			sizeX /= sizeY / maxYSize;
			sizeY = maxYSize;
		}
		if( sizeX > maxXSize ) {
			sizeY /= sizeY / maxYSize;
			sizeX = maxXSize;
		}
		
		f.setPixelSize( (int)sizeY );
		
		do {
			data += ' ';
		}while( QFontMetrics( f ).width( data ) < sizeX );
	}
	
	QAction *act = addAction(data);
	act->setProperty( "preview", QVariant( preview.scaled( (int)sizeX, (int)sizeY ) ) );
	act->setData( QVariant(id) );
	act->setFont( f );
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

void KrPreviewPopup::paintEvent(QPaintEvent *e)
{
	QMenu::paintEvent( e );
	QPainter p(this);
	QRegion emptyArea = QRegion(rect());

	//draw the items that need updating..
	//draw the items that need updating..
	foreach ( QAction* action, actions() )
	{
		QRect adjustedActionRect = actionGeometry(action);
		if ( !e->rect().intersects(adjustedActionRect) )
			continue;
		//set the clip region to be extra safe (and adjust for the scrollers)
		QRegion adjustedActionReg(adjustedActionRect);
		emptyArea -= adjustedActionReg;
		p.setClipRegion(adjustedActionReg);
		
		QVariant prop = action->property( "preview" );
		if( !prop.isNull() && prop.canConvert<QPixmap> () ) {
			QPixmap pm = prop.value<QPixmap>();
			style()->drawItemPixmap( &p, adjustedActionRect, Qt::AlignCenter, pm );
		}
	}
}

#include "krpreviewpopup.moc"
