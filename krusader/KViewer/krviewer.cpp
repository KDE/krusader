/***************************************************************************
                         krviewer.cpp  -  description
                            -------------------
   begin                : Thu Apr 18 2002
   copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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
// Qt includes
#include <qdatastream.h>
#include <qfile.h>
#include <qpopupmenu.h> 
#include <qtimer.h>
// KDE includes
#include <kmenubar.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kio/netaccess.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <klargefile.h>
#include <khtml_part.h>
#include <kprocess.h>
#include <kfileitem.h> 
// Krusader includes
#include "krviewer.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"

#include "panelviewer.h"

KrViewer* KrViewer::viewer = NULL;

KrViewer::KrViewer( QWidget *parent, const char *name ) :
KParts::MainWindow( parent, name ), manager( this, this ), tabBar( this ) {

	//setWFlags(WType_TopLevel | WDestructiveClose);
	setXMLFile( "krviewer.rc" ); // kpart-related xml file
	setHelpMenuEnabled( false );

	// KDE HACK START: Viewer fails to resize at maximized mode
	int scnum = QApplication::desktop() ->screenNumber( parentWidget() );
	QRect desk = QApplication::desktop() ->screenGeometry( scnum );
	KGlobal::config() ->setGroup( "KrViewerWindow" );
	QSize size( KGlobal::config() ->readNumEntry( QString::fromLatin1( "Width %1" ).arg( desk.width() ), 0 ),
	            KGlobal::config() ->readNumEntry( QString::fromLatin1( "Height %1" ).arg( desk.height() ), 0 ) );
	if ( size.width() > desk.width() )
		KGlobal::config() ->writeEntry( QString::fromLatin1( "Width %1" ).arg( desk.width() ), desk.width() );
	if ( size.height() > desk.height() )
		KGlobal::config() ->writeEntry( QString::fromLatin1( "Height %1" ).arg( desk.height() ), desk.height() );
	// KDE HACK END

	setAutoSaveSettings( "KrViewerWindow", true );
	tmpFile.setAutoDelete( true );

	connect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( createGUI( KParts::Part* ) ) );
	connect( &tabBar, SIGNAL( currentChanged( QWidget *) ),
	         this, SLOT( tabChanged(QWidget*) ) );
	connect( &tabBar, SIGNAL( closeRequest( QWidget *) ),
	         this, SLOT( tabCloseRequest(QWidget*) ) );

	icon = QIconSet(krLoader->loadIcon("view_detailed",KIcon::Small));
	viewerDict.setAutoDelete( true );
	setCentralWidget( &tabBar );

	viewerMenu = new QPopupMenu( this );
	viewerMenu->insertItem( i18n( "&Generic viewer" ), this, SLOT( viewGeneric() ), CTRL + Key_G, 1 );
	viewerMenu->insertItem( i18n( "&Text viewer" ), this, SLOT( viewText() ), CTRL + Key_T, 2 );
	viewerMenu->insertItem( i18n( "&Hex viewer" ), this, SLOT( viewHex() ), CTRL + Key_H, 3 );
	viewerMenu->insertSeparator();
	viewerMenu->insertItem( i18n( "Text &editor" ), this, SLOT( editText() ), CTRL + Key_E, 4 );
	viewerMenu->insertSeparator();
	viewerMenu->insertItem( i18n( "&Close current tab" ), this, SLOT( tabCloseRequest() ), Key_Escape );
	viewerMenu->insertItem( i18n( "&Quit" ), this, SLOT( close() ), Key_F10 );

	//toolBar() ->insertLined("Edit:",1,"",this,"",true ,i18n("Enter an URL to edit and press enter"));
	
	tabBar.setHoverCloseButton(true);

	checkModified();
}

KrViewer::~KrViewer() {

	disconnect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	            this, SLOT( createGUI( KParts::Part* ) ) );

	if( this == viewer ) viewer = NULL;
}

void KrViewer::createGUI( KParts::Part* part ) {
	if ( part == 0 )   /*     KHTMLPart calls this function with 0 at destruction.    */
		return ;        /*   Can cause crash after JavaScript self.close() if removed  */

	// and show the new part widget
	connect( part, SIGNAL( setStatusBarText( const QString& ) ),
	         this, SLOT( slotSetStatusBarText( const QString& ) ) );

	KParts::MainWindow::createGUI( part );

	toolBar() ->insertSeparator(1,1);
	toolBar() ->show();
	statusBar() ->show();

	// and "fix" the menubar
	menuBar() ->removeItem( 70 );
	menuBar() ->insertItem( i18n( "&KrViewer" ), viewerMenu, 70 );
	menuBar() ->show();
}

void KrViewer::keyPressEvent( QKeyEvent *e ) {
	switch ( e->key() ) {
		case Key_F10:
			close();
			break;
		case Key_Escape:
			tabCloseRequest();
			break;
		default:
			e->ignore();
			break;
	}
}

KrViewer* KrViewer::getViewer(bool new_window){
	if( !new_window ){
		if( !viewer ){	
			viewer = new KrViewer( krApp );
		}
		else {
			viewer->raise();
			viewer->setActiveWindow();
		}
		return viewer;
	}
	else return new KrViewer( krApp );
}	


void KrViewer::view( KURL url, Mode mode,  bool new_window ) {
	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* viewWidget = new PanelViewer(&viewer->tabBar);
	KParts::Part* part = viewWidget->openURL(url,mode);
	viewer->addTab(viewWidget,"Viewing",part);

}

void KrViewer::edit( KURL url, Mode mode, bool new_window ) {
	krConfig->setGroup( "General" );
	QString edit = krConfig->readEntry( "Editor", _Editor );

	if ( edit != "internal editor" ) {
		KProcess proc;
		// if the file is local, pass a normal path and not a url. this solves
		// the problem for editors that aren't url-aware
		if ( url.isLocalFile() )
			proc << QStringList::split( ' ', edit ) << url.path();
		else proc << QStringList::split( ' ', edit ) << url.prettyURL();
		if ( !proc.start( KProcess::DontCare ) )
			KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
		return ;
	}

	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* editWidget = new PanelEditor(&viewer->tabBar);
	KParts::Part* part = editWidget->openURL(url,mode);
	viewer->addTab(editWidget,QString("Editing"),part);
}

void KrViewer::addTab(PanelViewerBase* pvb, QString msg ,KParts::Part* part){
	if( !part ) return;

	KURL url = pvb->url();
	setCaption( msg+": " + url.prettyURL() );

	manager.addPart( part, this );
	manager.setActivePart( part );
	tabBar.insertTab(pvb,icon,url.fileName()+"("+msg+")");	
	tabBar.setCurrentPage(tabBar.indexOf(pvb));
	tabBar.setTabToolTip(pvb,msg+": " + url.prettyURL());

	viewerDict.insert( tabBar.currentPageIndex(),pvb );

	show();
	tabBar.show();
}

void KrViewer::tabChanged(QWidget* w){
	manager.setActivePart( static_cast<PanelViewerBase*>(w)->part() );
}

void KrViewer::tabCloseRequest(QWidget *w){
	if( !w ) return;

	PanelViewerBase* pvb = static_cast<PanelViewerBase*>(w);
	manager.removePart(pvb->part());
	tabBar.removePage(w);
	viewerDict.remove(tabBar.indexOf(w));

	if( tabBar.count() <= 0 ){
		delete this;
	}
}

void KrViewer::tabCloseRequest(){
	tabCloseRequest( tabBar.currentPage() ); 
}

bool KrViewer::queryClose() {
	return true;
}

bool KrViewer::queryExit() {
	kapp->ref(); // FIX: krusader exits at closing the viewer when minimized to tray
	return true; // don't let the reference counter reach zero
}

void KrViewer::viewGeneric(){
	PanelViewerBase* pvb = viewerDict[tabBar.currentPageIndex()];
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Generic);
	addTab(viewerWidget,QString("Viewing"),part);
}

void KrViewer::viewText(){
	PanelViewerBase* pvb = viewerDict[tabBar.currentPageIndex()];
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Text);
	addTab(viewerWidget,QString("Viewing"),part);
}

void KrViewer::viewHex(){
	PanelViewerBase* pvb = viewerDict[tabBar.currentPageIndex()];
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Hex);
	addTab(viewerWidget,QString("Viewing"),part);
}

void KrViewer::editText(){
	PanelViewerBase* pvb = viewerDict[tabBar.currentPageIndex()];
	if( !pvb ) return;

	PanelViewerBase* editWidget = new PanelEditor(&tabBar);
	KParts::Part* part = editWidget->openURL(pvb->url(),Text);
	addTab(editWidget,QString("Editing"),part);
}

void KrViewer::checkModified(){
	QTimer::singleShot( 1000, this, SLOT(checkModified()) );

	PanelViewerBase* pvb = viewerDict[tabBar.currentPageIndex()];
	if( !pvb ) return;

	// add a * to modified files.
	if( pvb->isModified() ){
		QString label = tabBar.tabLabel(pvb);
		if( !label.endsWith("*)") ){
			label.truncate(label.length()-1);
			label.append("*)");
			tabBar.setTabLabel(pvb,label);
		}
	}
	// remove the * from previously modified files.
	else {
		QString label = tabBar.tabLabel(pvb);
		if( label.endsWith("*)") ){
			label.truncate(label.length()-2);
			label.append(")");
			tabBar.setTabLabel(pvb,label);
		}		
	}
}

#if 0
bool KrViewer::editGeneric( QString mimetype, KURL _url ) {
	KParts::ReadWritePart * kedit_part = 0L;
	KLibFactory *factory = 0;
	KTrader::OfferList offers = KTrader::self() ->query( mimetype );

	// in theory, we only care about the first one.. but let's try all
	// offers just in case the first can't be loaded for some reason
	KTrader::OfferList::Iterator it( offers.begin() );
	for ( ; it != offers.end(); ++it ) {
		KService::Ptr ptr = ( *it );
		// we now know that our offer can handle mimetype and is a part.
		// since it is a part, it must also have a library... let's try to
		// load that now
		factory = KLibLoader::self() ->factory( ptr->library().latin1() );
		if ( factory ) {
			kedit_part = static_cast<KParts::ReadWritePart *>( factory->create( this,
			             ptr->name().latin1(), "KParts::ReadWritePart" ) );
			if ( kedit_part )
				if ( kedit_part->openURL( _url ) ) break;
				else {
					delete kedit_part;
					kedit_part = 0L;
				}
		}
	}

	if ( !kedit_part ) {
		KMessageBox::error( this, i18n( "Sorry, can't find internal editor" ) );
		return false;
	}

	setCentralWidget( kedit_part->widget() );
	createGUI( kedit_part );
	kedit_part->widget() ->show();
	return true;
}

bool KrViewer::editText( bool create ) {
	if ( !editor_part ) {
		editor_part = static_cast<KParts::ReadWritePart*>( getPart( url, "text/plain", false, create ) );
		if ( !editor_part ) return false;
		manager.addPart( editor_part, this );
	}
	manager.setActivePart( editor_part );
	tabBar.addTab(editor_part->widget(),url.fileName());
	return true;
}

bool KrViewer::viewGeneric() {
	QString mimetype = KMimeType::findByURL( url ) ->name();
	// ugly hack: don't try to get a part for an XML file, it usually don't work
	if ( mimetype == "text/xml" ) return false;
	if ( url.prettyURL().startsWith( "man:" ) ) mimetype = "text/html";
	if ( mimetype == "text/plain" )
		viewerMenu->setItemEnabled( 1, false );

	if ( !generic_part ) {
		if ( mimetype.contains( "html" ) ) {
			KHTMLPart * p = new KHTMLPart( this, 0, 0, 0, KHTMLPart::BrowserViewGUI );
			connect( p->browserExtension(), SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
			         this, SLOT( handleOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );
			/* At JavaScript self.close() the KHTMLPart destroys itself.  */
			/* After destruction, just close the window */
			connect( p, SIGNAL( destroyed() ), this, SLOT( close() ) );

			p-> openURL( url );
			generic_part = p;
		} else {
			generic_part = static_cast<KParts::ReadOnlyPart*>( getPart( url, mimetype, true ) );
		}
		if ( generic_part ) manager.addPart( generic_part, this );
		
		else return false;
	}

	manager.setActivePart( generic_part );
	tabBar.addTab(generic_part->widget(),url.fileName());
	return true;
}

bool KrViewer::viewText() {
	if ( !text_part ) {
		text_part = static_cast<KParts::ReadOnlyPart*>( getPart( url, "text/plain", true ) );
		if ( !text_part ) return false;
		manager.addPart( text_part, this );
	}
	manager.setActivePart( text_part );
	tabBar.addTab(text_part->widget(),url.fileName());
	return true;
}

void KrViewer::viewHex() {
	if ( !hex_part ) {
		QString file;
		// files that are not local must first be downloaded
		if ( !url.isLocalFile() ) {
			if ( !KIO::NetAccess::download( url, file ) ) {
				KMessageBox::sorry( this, i18n( "KrViewer is unable to download: " ) + url.url() );
				return ;
			}
		} else file = url.path();


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
		if ( !url.isLocalFile() )
			KIO::NetAccess::removeTempFile( file );

		hex_part = static_cast<KParts::ReadOnlyPart*>( getPart( tmpFile.name(), "text/plain", true ) );
		if ( !hex_part ) return ;
		manager.addPart( hex_part, this );
	}
	manager.setActivePart( hex_part );
	tabBar.addTab(hex_part->widget(),url.fileName());
}
#endif


#include "krviewer.moc"
