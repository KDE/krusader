/*
* This file is part of the KDE project
* Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
*               2001 Carsten Pfeiffer <pfeiffer@kde.org>
*
* You can Freely distribute this program under the GNU Library General Public
* License. See the file "COPYING" for the exact licensing terms.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <q3whatsthis.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>
#include <QResizeEvent>
#include <Q3VBoxLayout>
#include <Q3Frame>

#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kio/previewjob.h>

#include "kimagefilepreview.h"

/**** KrusaderImageFilePreview ****/

KrusaderImageFilePreview::KrusaderImageFilePreview( QWidget *parent )
		: KPreviewWidgetBase( parent ),
m_job( 0L ) {
	Q3VBoxLayout *vb = new Q3VBoxLayout( this, KDialog::marginHint() );

	imageLabel = new QLabel( this );
	imageLabel->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
	imageLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
	imageLabel->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored ) );
	vb->addWidget( imageLabel, 1 );

	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), SLOT( showPreview() ) );

	setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
}

KrusaderImageFilePreview::~KrusaderImageFilePreview() {
	if ( m_job )
		m_job->kill();
}

void KrusaderImageFilePreview::showPreview() {
	// Pass a copy since clearPreview() will clear currentURL
	KUrl url = currentURL;
	showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KrusaderImageFilePreview::showPreview( const KUrl& url ) {
	showPreview( url, false );
}

void KrusaderImageFilePreview::showPreview( const KUrl &url, bool force ) {
	if ( !url.isValid() ) {
		clearPreview();
		return ;
	}

	if ( url != currentURL || force ) {
		clearPreview();
		currentURL = url;

		int w = imageLabel->contentsRect().width() - 4;
		int h = imageLabel->contentsRect().height() - 4;

		m_job = createJob( url, w, h );
		connect( m_job, SIGNAL( result( KIO::Job * ) ),
		         this, SLOT( slotResult( KIO::Job * ) ) );
		connect( m_job, SIGNAL( gotPreview( const KFileItem*,
		                                    const QPixmap& ) ),
		         SLOT( gotPreview( const KFileItem*, const QPixmap& ) ) );

		connect( m_job, SIGNAL( failed( const KFileItem* ) ),
		         this, SLOT( slotFailed( const KFileItem* ) ) );
	}
}

void KrusaderImageFilePreview::resizeEvent( QResizeEvent * ) {
	timer->start( 100, true ); // forces a new preview
}

QSize KrusaderImageFilePreview::sizeHint() const {
	return QSize( 20, 200 ); // otherwise it ends up huge???
}

KIO::PreviewJob * KrusaderImageFilePreview::createJob( const KUrl& url, int w, int h ) {
	KUrl::List urls;
	urls.append( url );
	return KIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KrusaderImageFilePreview::gotPreview( const KFileItem* item, const QPixmap& pm ) {
	if ( item->url() == currentURL )   // should always be the case
		imageLabel->setPixmap( pm );
}

void KrusaderImageFilePreview::slotFailed( const KFileItem* item ) {
	if ( item->isDir() )
		imageLabel->clear();
	else if ( item->url() == currentURL )   // should always be the case
		imageLabel->setPixmap( SmallIcon( "file_broken", KIcon::SizeLarge,
		                                  KIcon::DisabledState ) );
}

void KrusaderImageFilePreview::slotResult( KIO::Job *job ) {
	if ( job == m_job )
		m_job = 0L;
}

void KrusaderImageFilePreview::clearPreview() {
	if ( m_job ) {
		m_job->kill();
		m_job = 0L;
	}

	imageLabel->clear();
	currentURL = KUrl();
}

void KrusaderImageFilePreview::virtual_hook( int id, void* data ) {
	KPreviewWidgetBase::virtual_hook( id, data );
}

#include "kimagefilepreview.moc"
