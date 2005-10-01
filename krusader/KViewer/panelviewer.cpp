#include <kurl.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <qapplication.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kmessagebox.h>
#include <qdict.h>
#include <qlabel.h>
#include <kmimetype.h>
#include <ktempfile.h>
#include <klocale.h>
#include <klibloader.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <klargefile.h>
#include "panelviewer.h"

#define DICTSIZE 211

/* ----==={ PanelViewerBase }===---- */

PanelViewerBase::PanelViewerBase( QWidget *parent ) :
QWidgetStack( parent ) {
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored ) );

	mimes = new QDict<KParts::ReadOnlyPart>( DICTSIZE, false );
	mimes->setAutoDelete( true );
	cpart = 0;
	fallback = new QLabel( i18n( "No file selected or selected file can't be displayed." ), this );
	fallback->setAlignment( AlignCenter | ExpandTabs | WordBreak );
	addWidget( fallback );
	raiseWidget( fallback );
}

PanelViewerBase::~PanelViewerBase() {
//	cpart->queryClose();
	closeURL();
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

KParts::ReadOnlyPart* PanelViewer::openURL( const KURL &url, KrViewer::Mode mode ) {
	closeURL();
	curl = url;

	if( mode == KrViewer::Generic ){
		cmimetype = KMimeType::findByURL( curl ) ->name();
		cpart = ( *mimes ) [ cmimetype ];
		if ( !cpart ){
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
	}

	KTempFile tmpFile;

	if( mode == KrViewer::Hex ){
		if ( !cpart ) cpart = getHexPart();
		if ( !cpart ) oldHexViewer(tmpFile);
	}
	
	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	if ( cpart && cpart->openURL( curl ) ){
		curl = url; /* needed because of the oldHexViewer */
		return cpart;
	}
	else {
		raiseWidget( fallback );
		return 0;
	}
}

bool PanelViewer::closeURL() {
	raiseWidget( fallback );
	if ( cpart && cpart->closeURL() ) return true;
	return false;
}

KParts::ReadOnlyPart* PanelViewer::getPart( QString mimetype ) {
	KParts::ReadOnlyPart * part = 0L;
	KLibFactory *factory = 0;
	KService::Ptr ptr = KServiceTypeProfile::preferredService( mimetype, "KParts/ReadOnlyPart" );
	if ( ptr ) {
		QStringList args;
		QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			QString argStr = argsProp.toString();
			args = QStringList::split( " ", argStr );
		}
		QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )   // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().latin1() );
			if ( factory ) {
				part = static_cast<KParts::ReadOnlyPart *>( factory->create( this,
				        ptr->name().latin1(), QString( "KParts::ReadOnlyPart" ).latin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, SLOT( openURL( const KURL & ) ) );
			connect( ext, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, SIGNAL( openURLRequest( const KURL & ) ) );
		}
	}
	return part;
}

KParts::ReadOnlyPart* PanelViewer::getHexPart(){
	KParts::ReadOnlyPart * part = 0L;

	KLibFactory * factory = KLibLoader::self() ->factory( "libkhexedit2part" );
	if ( factory ) {
		// Create the part
		part = ( KParts::ReadOnlyPart * ) factory->create( this, "hexedit2part","KParts::ReadOnlyPart" );
	}

	return part;
}

void PanelViewer::oldHexViewer(KTempFile& tmpFile) {
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
	f_in.open( IO_ReadOnly );
	QDataStream in( &f_in );

	FILE *out = KDE_fopen( tmpFile.name().local8Bit(), "w" );

	KIO::filesize_t fileSize = f_in.size();
	KIO::filesize_t address = 0;
	char buf[ 16 ];
	unsigned int* pBuff = ( unsigned int* ) buf;

	while ( address < fileSize ) {
		memset( buf, 0, 16 );
		int bufSize = ( ( fileSize - address ) > 16 ) ? 16 : ( fileSize - address );
		in.readRawBytes( buf, bufSize );
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

	curl = tmpFile.name();
}

/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor( QWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelEditor::~PanelEditor() {
	static_cast<KParts::ReadWritePart *>(cpart)->queryClose();
}

KParts::ReadOnlyPart* PanelEditor::openURL( const KURL &url, KrViewer::Mode mode ) {
	closeURL();
	curl = url;

	if( mode == KrViewer::Generic ){
		cmimetype = KMimeType::findByURL( curl ) ->name();
		cpart = ( *mimes ) [ cmimetype ];
		if ( !cpart ){ 
			cpart = getPart( cmimetype );
			mimes->insert( cmimetype, cpart );
		}
	}

	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );

	if ( cpart ) {
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	else {
		raiseWidget( fallback );
		return 0;
	}

	bool create = true;
	KIO::StatJob* statJob = KIO::stat( url, false );
	connect( statJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
	busy = true;
	while ( busy ) qApp->processEvents();
	if( !entry.isEmpty() ) {
		KFileItem file( entry, url );
		if( file.isReadable() ) create = false;
	}
	
	if( create ){
		if( static_cast<KParts::ReadWritePart *>(cpart)->saveAs( curl ) ) return cpart;
	}
	else {
		if ( cpart->openURL( curl ) ) return cpart;
	}
	return 0;
}

bool PanelEditor::closeURL() {
	raiseWidget( fallback );

	if ( !cpart ) return false;

	static_cast<KParts::ReadWritePart *>(cpart)->queryClose();
	static_cast<KParts::ReadWritePart *>(cpart)->closeURL(true);
	return true;
}

KParts::ReadWritePart* PanelEditor::getPart( QString mimetype ) {
	KParts::ReadWritePart * part = 0L;
	KLibFactory *factory = 0;
	KService::Ptr ptr = KServiceTypeProfile::preferredService( mimetype, "KParts/ReadWritePart" );
	if ( ptr ) {
		QStringList args;
		QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
		if ( argsProp.isValid() ) {
			QString argStr = argsProp.toString();
			args = QStringList::split( " ", argStr );
		}
		QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );
		if ( !prop.isValid() || prop.toBool() )  // defaults to true
		{
			factory = KLibLoader::self() ->factory( ptr->library().latin1() );
			if ( factory ) {
				part = static_cast<KParts::ReadWritePart *>( factory->create( this,
				        ptr->name().latin1(), QString( "KParts::ReadWritePart" ).latin1(), args ) );
			}
		}
	}
	if ( part ) {
		KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( part );
		if ( ext ) {
			connect( ext, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, SLOT( openURL( const KURL & ) ) );
			connect( ext, SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ), this, SIGNAL( openURLRequest( const KURL & ) ) );
		}
	}
	return part;
}

void PanelEditor::slotStatResult( KIO::Job* job ) {
  if( !job || job->error() ) entry = KIO::UDSEntry();
  else entry = static_cast<KIO::StatJob*>(job)->statResult();
  busy = false;
}

#include "panelviewer.moc"
