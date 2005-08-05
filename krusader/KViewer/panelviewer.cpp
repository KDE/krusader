#include <kurl.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <qdict.h>
#include <qlabel.h>
#include <kmimetype.h>
#include <klocale.h>
#include <klibloader.h>
#include <kuserprofile.h>
#include <kdebug.h>
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

KParts::ReadOnlyPart* PanelViewer::openURL( const KURL &url ) {
	closeURL();
	curl = url;
	cmimetype = KMimeType::findByURL( curl ) ->name();
	cpart = ( *mimes ) [ cmimetype ];
	if ( !cpart ) cpart = getPart( cmimetype );
	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );
	if ( cpart ) {
		mimes->insert( cmimetype, cpart );
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	if ( cpart && cpart->openURL( curl ) ) return cpart;
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

KParts::ReadOnlyPart *PanelViewer::getPart( QString mimetype ) {
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

/* ----==={ PanelEditor }===---- */

PanelEditor::PanelEditor( QWidget *parent ) :
PanelViewerBase( parent ) {
}

PanelEditor::~PanelEditor() {
	static_cast<KParts::ReadWritePart *>(cpart)->queryClose();
}

KParts::ReadOnlyPart* PanelEditor::openURL( const KURL &url ) {
	closeURL();
	curl = url;
	cmimetype = KMimeType::findByURL( curl ) ->name();
	cpart = ( *mimes ) [ cmimetype ];
	if ( !cpart ) cpart = getPart( cmimetype );
	if ( !cpart ) cpart = getPart( "text/plain" );
	if ( !cpart ) cpart = getPart( "all/allfiles" );
	if ( cpart ) {
		mimes->insert( cmimetype, cpart );
		addWidget( cpart->widget() );
		raiseWidget( cpart->widget() );
	}
	if ( cpart && cpart->openURL( curl ) ) return cpart;
	else {
		raiseWidget( fallback );
		return 0;
	}
}

bool PanelEditor::closeURL() {
	raiseWidget( fallback );

	if ( !cpart ) return false;

	static_cast<KParts::ReadWritePart *>(cpart)->queryClose();
	static_cast<KParts::ReadWritePart *>(cpart)->closeURL(true);
	return true;
}

KParts::ReadWritePart *PanelEditor::getPart( QString mimetype ) {
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


#include "panelviewer.moc"
