#include <kurl.h>
#include <qstring.h>
#include <qstackedwidget.h>
#include <qapplication.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kmessagebox.h>
#include <qhash.h>
#include <qlabel.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <klibloader.h>
#include <kservicetypeprofile.h>
#include <kmimetypetrader.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <kde_file.h>
#include "panelviewer.h"

#define DICTSIZE 211

/* ----==={ PanelViewerBase }===---- */

PanelViewerBase::PanelViewerBase( QWidget *parent ) :
QStackedWidget( parent ), mimes( 0 ), cpart( 0 ) {
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored ) );
	
	mimes = new QHash<QString, QPointer<KParts::ReadOnlyPart> >();
	cpart = 0;
	fallback = new QLabel( i18n( "No file selected or selected file can't be displayed." ), this );
	fallback->setAlignment( Qt::Alignment( QFlag( Qt::AlignCenter | Qt::TextExpandTabs ) ) );
	fallback->setWordWrap( true );
	addWidget( fallback );
	setCurrentWidget( fallback );
}

PanelViewerBase::~PanelViewerBase() {
//	cpart->queryClose();
	closeUrl();
	QHashIterator< QString, QPointer<KParts::ReadOnlyPart> > lit( *mimes );
	while( lit.hasNext() ) {
		QPointer<KParts::ReadOnlyPart> p = lit.next().value();
		if( p )
			delete p;
	}
	mimes->clear();
	delete mimes;
	delete fallback;
}

/* ----==={ PanelViewer }===---- */

PanelViewer::PanelViewer( QWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelViewer::~PanelViewer() {
}

KParts::ReadOnlyPart* PanelViewer::openUrl( const KUrl &url, KrViewer::Mode mode ) {
	emit urlChanged( this, url );
	closeUrl();
	curl = url;

	if( mode == KrViewer::Generic ){
		KMimeType::Ptr mt = KMimeType::findByUrl( curl );
		cmimetype = mt ? mt->name() : QString();
		if( mimes->find( cmimetype ) == mimes->end() ) {
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
		else
			cpart = ( *mimes ) [ cmimetype ];
	}

	KTemporaryFile tmpFile;

	if( mode == KrViewer::Hex ){
		if ( !cpart ) cpart = getHexPart();
		if ( !cpart ) oldHexViewer(tmpFile);
	}
	
	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		setCurrentWidget( cpart->widget() );
	}
	if ( cpart && cpart->openUrl( curl ) ){
		curl = url; /* needed because of the oldHexViewer */
		connect( cpart, SIGNAL( destroyed() ), this, SLOT( slotCPartDestroyed() ) );
		return cpart;
	}
	else {
		setCurrentWidget( fallback );
		return 0;
	}
}

bool PanelViewer::closeUrl() {
	setCurrentWidget( fallback );
	if ( cpart && cpart->closeUrl() ) {
		cpart = 0;
		return true;
	}
	return false;
}

KParts::ReadOnlyPart* PanelViewer::getPart( QString mimetype ) {
	KParts::ReadOnlyPart * part = 0L;
	KPluginFactory *factory = 0;
	KService::Ptr ptr = KMimeTypeTrader::self()->preferredService( mimetype, "KParts/ReadOnlyPart" );
	if ( ptr ) {
		QStringList args;
		QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			QString argStr = argsProp.toString();
			args = argStr.split( " " );
		}
		QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )   // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().toLatin1() );
			if ( factory ) {
				if( ptr->serviceTypes().contains( "Browser/View" ) )
					part = static_cast<KParts::ReadOnlyPart *>( factory->create( this,
					        QString( "Browser/View" ).toLatin1(), args ) );
				if( !part )
					part = static_cast<KParts::ReadOnlyPart *>( factory->create( this,
					        QString( "KParts::ReadOnlyPart" ).toLatin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, SIGNAL( openUrlRequestDelayed( const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments& ) ), this, SLOT( openUrl( const KUrl & ) ) );
			connect( ext, SIGNAL( openUrlRequestDelayed( const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments& ) ), this, SIGNAL( openUrlRequest( const KUrl & ) ) );
		}
	}
	return part;
}

KParts::ReadOnlyPart* PanelViewer::getHexPart(){
	KParts::ReadOnlyPart * part = 0L;

	KPluginFactory * factory = KLibLoader::self() ->factory( "libkhexedit2part" );
	if ( factory ) {
		// Create the part
		part = ( KParts::ReadOnlyPart * ) factory->create( this, "KParts::ReadOnlyPart" );
	}

	return part;
}

void PanelViewer::oldHexViewer(KTemporaryFile& tmpFile) {
	QString file;
	// files that are not local must first be downloaded
	if ( !curl.isLocalFile() ) {
		if ( !KIO::NetAccess::download( curl, file,this ) ) {
			KMessageBox::sorry( this, i18n( "KrViewer is unable to download: " ) + curl.url() );
			return ;
		}
	} else file = curl.path();


	// create a hex file
	QFile f_in( file );
	f_in.open( QIODevice::ReadOnly );
	QDataStream in( &f_in );

  tmpFile.open(); // else there is no filename
	FILE *out = KDE_fopen( tmpFile.fileName().toLocal8Bit(), "w" ); // TODO get rid of FILE*

	KIO::filesize_t fileSize = f_in.size();
	KIO::filesize_t address = 0;
	char buf[ 16 ];
	unsigned int* pBuff = ( unsigned int* ) buf;

	while ( address < fileSize ) {
		memset( buf, 0, 16 );
		int bufSize = ( ( fileSize - address ) > 16 ) ? 16 : ( fileSize - address );
		in.readRawData( buf, bufSize );
		fprintf( out, "0x%8.8llx: ", address );
		for ( int i = 0; i < 4; ++i ) {
			if ( i < ( bufSize / 4 ) ) fprintf( out, "%8.8x ", pBuff[ i ] );
			else fprintf( out, "         " );
		}
		fprintf( out, "| " );

		for ( int i = 0; i < bufSize; ++i ) {
			if ( buf[ i ] > ' ' && buf[ i ] < '~' ) fputc( buf[ i ], out );
			else fputc( '.', out );
		}
		fputc( '\n', out );

		address += 16;
	}
	// clean up
	f_in.close();
	fclose( out );
	if ( !curl.isLocalFile() )
	KIO::NetAccess::removeTempFile( file );

	curl = tmpFile.fileName();
}

/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor( QWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelEditor::~PanelEditor() {
}

KParts::ReadOnlyPart* PanelEditor::openUrl( const KUrl &url, KrViewer::Mode mode ) {
	emit urlChanged( this, url );
	closeUrl();
	curl = url;

	if( mode == KrViewer::Generic ){
		KMimeType::Ptr mt = KMimeType::findByUrl( curl );
		cmimetype = mt ? mt->name() : QString();
		if( mimes->find( cmimetype ) == mimes->end() ) {
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
		else
			cpart = ( *mimes ) [ cmimetype ];
	}

	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		setCurrentWidget( cpart->widget() );
	}
	else {
		setCurrentWidget( fallback );
		return 0;
	}

	bool create = true;
	KIO::StatJob* statJob = KIO::stat( url, false );
	connect( statJob, SIGNAL( result( KJob* ) ), this, SLOT( slotStatResult( KJob* ) ) );
	busy = true;
	while ( busy ) qApp->processEvents();
	if( entry.count() != 0 ) {
		KFileItem file( entry, url );
		if( file.isReadable() ) create = false;
	}
	
	if( create ){
		if( static_cast<KParts::ReadWritePart *>(cpart.data())->saveAs( curl ) ) {
			connect( cpart, SIGNAL( destroyed() ), this, SLOT( slotCPartDestroyed() ) );
			return cpart;
		}
	}
	else {
		if ( cpart->openUrl( curl ) ) {
			connect( cpart, SIGNAL( destroyed() ), this, SLOT( slotCPartDestroyed() ) );
			return cpart;
		}
	}
	return 0;
}

bool PanelEditor::queryClose() {
	if ( !cpart ) return true;
	return static_cast<KParts::ReadWritePart *>(cpart.data())->queryClose();
}

bool PanelEditor::closeUrl() {
	if ( !cpart ) return false;
	
	static_cast<KParts::ReadWritePart *>(cpart.data())->closeUrl( false );
	
	setCurrentWidget( fallback );
	cpart = 0;
	return true;
}

KParts::ReadWritePart* PanelEditor::getPart( QString mimetype ) {
	KParts::ReadWritePart * part = 0L;
	KPluginFactory *factory = 0;
	KService::Ptr ptr = KMimeTypeTrader::self()->preferredService( mimetype, "KParts/ReadWritePart" );
	if ( ptr ) {
		QStringList args;
		QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			QString argStr = argsProp.toString();
			args = argStr.split( " " );
		}
		QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )  // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().toLatin1() );
			if ( factory ) {
				part = static_cast<KParts::ReadWritePart *>( factory->create( this,
				        QString( "KParts::ReadWritePart" ).toLatin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, SIGNAL( openUrlRequestDelayed( const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments& ) ), this, SLOT( openUrl( const KUrl & ) ) );
			connect( ext, SIGNAL( openUrlRequestDelayed( const KUrl &, const KParts::OpenUrlArguments&, const KParts::BrowserArguments& ) ), this, SIGNAL( openUrlRequest( const KUrl & ) ) );
		}
	}
	return part;
}

void PanelEditor::slotStatResult( KJob* job ) {
  if( !job || job->error() ) entry = KIO::UDSEntry();
  else entry = static_cast<KIO::StatJob*>(job)->statResult();
  busy = false;
}

bool PanelEditor::isModified(){
	return static_cast<KParts::ReadWritePart *>(cpart.data())->isModified();
}

#include "panelviewer.moc"
