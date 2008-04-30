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
#include <qtimer.h>
#include <QKeyEvent>
#include <QEvent>
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
#include <kde_file.h>
#include <khtml_part.h>
#include <kprocess.h>
#include <kfileitem.h> 
#include <ktoolbar.h>
// Krusader includes
#include "krviewer.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"

#include "panelviewer.h"

#define VIEW_ICON     "viewmag"
#define EDIT_ICON     "edit"
#define MODIFIED_ICON "filesaveas"


QList<KrViewer *> KrViewer::viewers;

KrViewer::KrViewer( QWidget *parent ) :
KParts::MainWindow( parent, (Qt::WindowFlags)KDE_DEFAULT_WINDOWFLAGS ), manager( this, this ), tabBar( this ), returnFocusTo( 0 ), returnFocusTab( 0 ),
                                    reservedKeys(), reservedKeyActions() {

	//setWFlags(Qt::WType_TopLevel | WDestructiveClose);
	setXMLFile( "krviewer.rc" ); // kpart-related xml file
	setHelpMenuEnabled( false );

	setAutoSaveSettings( "KrViewerWindow", true );

	connect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( createGUI( KParts::Part* ) ) );
	connect( &tabBar, SIGNAL( currentChanged( QWidget *) ),
	         this, SLOT( tabChanged(QWidget*) ) );
	connect( &tabBar, SIGNAL( closeRequest( QWidget *) ),
	         this, SLOT( tabCloseRequest(QWidget*) ) );

	tabBar.setTabReorderingEnabled(false);
	tabBar.setAutomaticResizeTabs(true);
//	"edit"
//	"filesaveas"
	setCentralWidget( &tabBar );

	printAction = KStandardAction::print( this, SLOT( print() ), 0 );
	copyAction = KStandardAction::copy( this, SLOT( copy() ), 0 );

	viewerMenu = new QMenu( this );
	viewerMenu->addAction( i18n( "&Generic viewer" ), this, SLOT( viewGeneric() ))->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_G );
	viewerMenu->addAction( i18n( "&Text viewer" ), this, SLOT( viewText() ))->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_T );
	viewerMenu->addAction( i18n( "&Hex viewer" ), this, SLOT( viewHex() ))->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_H );
	viewerMenu->addSeparator();
	viewerMenu->addAction( i18n( "Text &editor" ), this, SLOT( editText() ))->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_E );
	viewerMenu->addSeparator();
	viewerMenu->addAction( i18n( "&Next tab" ), this, SLOT( nextTab() ))->setShortcut( Qt::ALT+Qt::Key_Right );
	viewerMenu->addAction( i18n( "&Previous tab" ), this, SLOT( prevTab() ))->setShortcut( Qt::ALT+Qt::Key_Left );

	detachAction = viewerMenu->addAction( i18n( "&Detach tab" ), this, SLOT( detachTab() ));
	detachAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_D );
	//no point in detaching only one tab..
	detachAction->setEnabled(false);	
	viewerMenu->addSeparator();
	viewerMenu->addAction( printAction->icon(), printAction->text(), this, SLOT( print() ))->setShortcut( printAction->shortcut().primary() );
	toolBar()->addAction( printAction->icon(), printAction->text(), this, SLOT( print() ));
	viewerMenu->addAction( copyAction->icon(), copyAction->text(), this, SLOT( copy() ))->setShortcut( copyAction->shortcut().primary() );
	toolBar()->addAction( copyAction->icon(), copyAction->text(), this, SLOT( copy() ) );
	viewerMenu->addSeparator();
	( tabClose = viewerMenu->addAction( i18n( "&Close current tab" ), this, SLOT( tabCloseRequest() )))->setShortcut( Qt::Key_Escape );
	( closeAct = viewerMenu->addAction( i18n( "&Quit" ), this, SLOT( close() )))->setShortcut( Qt::CTRL + Qt::Key_Q );

	tabBar.setHoverCloseButton(true);

	checkModified();
}

KrViewer::~KrViewer() {

	disconnect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	            this, SLOT( createGUI( KParts::Part* ) ) );

	viewers.removeAll( this );
	delete printAction;
	delete copyAction;
}

void KrViewer::createGUI( KParts::Part* part ) {
	/* TODO: fix the toolbar bugs */
	if ( part == 0 )   /*     KHTMLPart calls this function with 0 at destruction.    */
		return ;        /*   Can cause crash after JavaScript self.close() if removed  */

	
	// and show the new part widget
	connect( part, SIGNAL( setStatusBarText( const QString& ) ),
	         this, SLOT( slotSetStatusBarText( const QString& ) ) );

	KParts::MainWindow::createGUI( part );
	toolBar()->addSeparator();

	PanelViewerBase *pvb = getPanelViewerBase( part );
	if( pvb )
		updateActions( pvb );

	toolBar() ->show();
	statusBar() ->show();

	// the KParts part may override the viewer shortcuts. We prevent it
	// by installing an event filter on the menuBar() and the part
	reservedKeys.clear();
	reservedKeyActions.clear();
	
	QList<QAction *> list = viewerMenu->actions();
	// getting the key sequences of the viewer menu
	for( int w=0; w != list.count(); w++ )
	{
		QAction *act = list[ w ];
		QList<QKeySequence> sequences = act->shortcuts();
		if( !sequences.isEmpty() )
		{
			for( int i=0; i != sequences.count(); i++ )
			{
				reservedKeys.push_back( sequences[ i ] );
				reservedKeyActions.push_back( act );
			}
		}
	}
	
	// and "fix" the menubar
	QList<QAction *> actList = menuBar()->actions();
	foreach( QAction * a, actList ) {
		if( a->data().canConvert<int>() && a->data().toInt() == 70 )
			menuBar()->removeAction( a );
	}
	viewerMenu->setTitle( i18n( "&KrViewer" ) );
	QAction * act = menuBar() ->addMenu( viewerMenu );
	act->setData( QVariant( 70 ) );
	menuBar() ->show();
	
	// filtering out the key events
	menuBar() ->installEventFilter( this );
	part->installEventFilter( this );
}

bool KrViewer::eventFilter (  QObject * /* watched */, QEvent * e )
{
	if( e->type() == QEvent::ShortcutOverride )
	{
		QKeyEvent* ke = (QKeyEvent*) e;
		if( reservedKeys.contains( ke->key() ) )
		{
			ke->accept();
			
			QAction *act = reservedKeyActions[ reservedKeys.indexOf( ke->key() ) ];
			if( act != 0 )
			{
				// don't activate the close functions immediately!
				// it can cause crash
				if( act == tabClose )
					QTimer::singleShot( 0, this, SLOT( tabCloseRequest() ) );
				else if( act == closeAct )
					QTimer::singleShot( 0, this, SLOT( close() ) );
				else {
					act->activate( QAction::Trigger );
				}
			}
			return true;
		}
	}
	else if( e->type() == QEvent::KeyPress )
	{
		QKeyEvent* ke = (QKeyEvent*) e;
		if( reservedKeys.contains( ke->key() ) )
		{
			ke->accept();
			return true;
		}
	}
	return false;
}
void KrViewer::keyPressEvent( QKeyEvent *e ) {
	switch ( e->key() ) {
		case Qt::Key_F10:
			close();
			break;
		case Qt::Key_Escape:
			tabCloseRequest();
			break;
		default:
			e->ignore();
			break;
	}
}

KrViewer* KrViewer::getViewer(bool new_window){
	if( !new_window ){
		if( viewers.isEmpty() ){	
			viewers.prepend( new KrViewer() ); // add to first (active)
		}
		else {
			if( viewers.first()->isMinimized() ) // minimized? -> show it again
				viewers.first()->showNormal();
			viewers.first()->raise();
			viewers.first()->setActiveWindow();
		}
		return viewers.first();
	}
	else {
		KrViewer *newViewer = new KrViewer();
		viewers.prepend( newViewer );
		return newViewer;
	}
}	

void KrViewer::view( KUrl url, QWidget * parent ) {
	Mode defaultMode = Generic;
	bool defaultWindow = false;

	KConfigGroup group( krConfig, "General" );
	defaultWindow = group.readEntry( "View In Separate Window",_ViewInSeparateWindow );

	QString modeString = group.readEntry( "Default Viewer Mode", QString( "generic" ) );

	if( modeString == "generic" ) defaultMode = Generic;
	else if( modeString == "text" ) defaultMode = Text;
	else if( modeString == "hex" ) defaultMode = Hex;

	view(url,defaultMode,defaultWindow, parent );
}

void KrViewer::view( KUrl url, Mode mode,  bool new_window, QWidget * parent ) {
	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* viewWidget = new PanelViewer(&viewer->tabBar);
	KParts::Part* part = viewWidget->openUrl(url,mode);
	viewer->addTab(viewWidget,i18n( "Viewing" ),VIEW_ICON,part);

	viewer->returnFocusTo = parent;
	viewer->returnFocusTab = viewWidget;
}

void KrViewer::edit( KUrl url, QWidget * parent ) {
	edit( url, Text, -1, parent );
}

void KrViewer::edit( KUrl url, Mode mode, int new_window, QWidget * parent ) {
	KConfigGroup group( krConfig, "General" );
	QString edit = group.readEntry( "Editor", _Editor );
	
	if( new_window == -1 )
		new_window = group.readEntry( "View In Separate Window",_ViewInSeparateWindow );

	if ( edit != "internal editor" ) {
		KProcess proc;
		// if the file is local, pass a normal path and not a url. this solves
		// the problem for editors that aren't url-aware
		if ( url.isLocalFile() )
			proc << edit.split( ' ' ) << url.path();
		else
			proc << edit.split( ' ' ) << url.prettyUrl();
		if ( !proc.startDetached() )
			KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
		return ;
	}

	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* editWidget = new PanelEditor(&viewer->tabBar);
	KParts::Part* part = editWidget->openUrl(url,mode);
	viewer->addTab(editWidget,i18n("Editing"),EDIT_ICON,part);
	
	viewer->returnFocusTo = parent;
	viewer->returnFocusTab = editWidget;
}

void KrViewer::addTab(PanelViewerBase* pvb, QString msg, QString iconName ,KParts::Part* part){
	if( !part ) return;

	KUrl url = pvb->url();
	setWindowTitle( msg+": " + url.prettyUrl() );

	QIcon icon = QIcon(krLoader->loadIcon(iconName,KIconLoader::Small));

	manager.addPart( part, this );
	manager.setActivePart( part );
	tabBar.addTab(pvb,icon,url.fileName()+"("+msg+")");	
	tabBar.setCurrentIndex(tabBar.indexOf(pvb));
	tabBar.setTabToolTip(pvb,msg+": " + url.prettyUrl());

	updateActions( pvb );

	// now we can offer the option to detach tabs (we have more than one)
	if( tabBar.count() > 1 ){
		detachAction->setEnabled(true);	
	}

	show();
	tabBar.show();
	
	connect( pvb, SIGNAL( urlChanged( PanelViewerBase *, const KUrl & ) ), 
	         this,  SLOT( tabURLChanged(PanelViewerBase *, const KUrl & ) ) );
	
	if( part->widget() )
		part->widget()->setFocus();
}

void KrViewer::tabURLChanged( PanelViewerBase *pvb, const KUrl & url ) {
	QString msg = pvb->isEditor() ? i18n( "Editing" ) : i18n( "Viewing" );
	tabBar.setTabText( tabBar.indexOf( pvb ), url.fileName()+"("+msg+")" );
	tabBar.setTabToolTip(pvb,msg+": " + url.prettyUrl());
}

void KrViewer::tabChanged(QWidget* w){
	manager.setActivePart( static_cast<PanelViewerBase*>(w)->part() );
	
	if( static_cast<PanelViewerBase*>(w) != returnFocusTab ) {
		returnFocusTo = 0;
		returnFocusTab = 0;
	}
	
	// set this viewer to be the main viewer
	if( viewers.removeAll( this ) ) viewers.prepend( this ); // move to first
}

void KrViewer::tabCloseRequest(QWidget *w){
	if( !w ) return;
	
	// important to save as returnFocusTo will be cleared at removePart
	QWidget * returnFocusToThisWidget = returnFocusTo;
	
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>(w);
	
	if( !pvb->queryClose() )
		return;
		
	manager.removePart(pvb->part());
	
	pvb->closeUrl();
	
	tabBar.removePage(w);

	if( tabBar.count() <= 0 ){
		if( returnFocusToThisWidget ){ 
			returnFocusToThisWidget->raise();
			returnFocusToThisWidget->setActiveWindow();
		}
		else {
			krApp->raise();
			krApp->setActiveWindow();
		}
		delete this;
		return;
	} else if( tabBar.count() == 1 ){
		//no point in detaching only one tab..
		detachAction->setEnabled(false);
	}

	if( returnFocusToThisWidget ){ 
		returnFocusToThisWidget->raise();
		returnFocusToThisWidget->setActiveWindow();
	}
}

void KrViewer::tabCloseRequest(){
	tabCloseRequest( tabBar.currentPage() ); 
}

bool KrViewer::queryClose() {
	for( int i=0; i != tabBar.count(); i++ ) {
		PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.page( i ) );
		if( !pvb )
			continue;
		
		tabBar.setCurrentIndex( i );
		
		if( !pvb->queryClose() )
			return false;
	}
	return true;
}

bool KrViewer::queryExit() {
	return true; // don't let the reference counter reach zero
}

void KrViewer::viewGeneric(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openUrl(pvb->url(),Generic);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::viewText(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openUrl(pvb->url(),Text);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::viewHex(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openUrl(pvb->url(),Hex);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::editText(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* editWidget = new PanelEditor(&tabBar);
	KParts::Part* part = editWidget->openUrl(pvb->url(),Text);
	addTab(editWidget,i18n("Editing"),EDIT_ICON,part);
}

void KrViewer::checkModified(){
	QTimer::singleShot( 1000, this, SLOT(checkModified()) );

	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	if( !pvb->part()->url().equals( pvb->url(), KUrl::CompareWithoutTrailingSlash ) ) {
		pvb->setUrl( pvb->part()->url() );
	}

	bool lastModified = false;
	QVariant lastModProp = pvb->property( "LastModified" );
	if( lastModProp.type() == QVariant::Bool )
		lastModified = lastModProp.toBool();
	pvb->setProperty( "LastModified", QVariant( pvb->isModified() ) );

	// add a * to modified files.
	if( pvb->isModified() && !lastModified ){
		int ndx = tabBar.indexOf( pvb );
		QString label = tabBar.tabText( ndx );
		label.prepend("*");
		QIcon icon = QIcon(krLoader->loadIcon(MODIFIED_ICON,KIconLoader::Small));
		tabBar.setTabText( ndx,label );
		tabBar.setTabIcon( ndx,icon );
	}
	// remove the * from previously modified files.
	else if( !pvb->isModified() && lastModified ) {
		int ndx = tabBar.indexOf( pvb );
		QString label = tabBar.tabText( tabBar.indexOf( pvb ) );
		label = label.mid( 1 );
		QIcon icon = QIcon(krLoader->loadIcon(EDIT_ICON,KIconLoader::Small));
		tabBar.setTabText( ndx,label );
		tabBar.setTabIcon( ndx,icon );
	}
}

void KrViewer::nextTab(){
	int index = (tabBar.currentPageIndex()+1)%tabBar.count();
	tabBar.setCurrentIndex( index );
}

void KrViewer::prevTab(){
	int index = (tabBar.currentPageIndex()-1)%tabBar.count();
	while( index < 0 ) index+=tabBar.count();
	tabBar.setCurrentIndex( index );
}

void KrViewer::detachTab(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	KrViewer* viewer = getViewer(true);

	manager.removePart(pvb->part());
	tabBar.removePage(pvb);
	
	if( tabBar.count() == 1 ) {
		//no point in detaching only one tab..
		detachAction->setEnabled(false);
	}
	
	pvb->reparent(&viewer->tabBar,QPoint(0,0));

	if( pvb->isEditor() )
		viewer->addTab(pvb,i18n( "Editing" ),EDIT_ICON,pvb->part());
	else
		viewer->addTab(pvb,i18n( "Viewing" ),VIEW_ICON,pvb->part());
}

void KrViewer::windowActivationChange ( bool /* oldActive */ ) {
	if( isActiveWindow() )
		if( viewers.removeAll( this ) ) viewers.prepend( this ); // move to first
}

void KrViewer::print() {
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;
	
	KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( pvb->part() );
	if( ext && ext->isActionEnabled( "print" ) )
		Invoker( ext, SLOT( print() ) ).invoke();
}

void KrViewer::copy() {
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;
	
	KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( pvb->part() );
	if( ext && ext->isActionEnabled( "copy" ) )
		Invoker( ext, SLOT( copy() ) ).invoke();
}

PanelViewerBase * KrViewer::getPanelViewerBase( KParts::Part * part ) {
	for( int i=0; i != tabBar.count(); i++ ) {
		PanelViewerBase *pvb = static_cast<PanelViewerBase*>( tabBar.page( i ) );
		if( pvb && pvb->part() == part )
			return pvb;
	}
	return 0;
}

void KrViewer::updateActions( PanelViewerBase * pvb ) {
	if( pvb->isEditor() ) {
		printAction->setVisible(false);
		copyAction->setVisible(false);
	}
	else {
		if( !printAction->isVisible() )
			printAction->setVisible( true );
		if( !copyAction->isVisible() )
			copyAction->setVisible( true );
	}
}

#if 0
bool KrViewer::editGeneric( QString mimetype, KUrl _url ) {
	KParts::ReadWritePart * kedit_part = 0L;
	KPluginFactory *factory = 0;
	KTrader::OfferList offers = KTrader::self() ->query( mimetype );

	// in theory, we only care about the first one.. but let's try all
	// offers just in case the first can't be loaded for some reason
	KTrader::OfferList::Iterator it( offers.begin() );
	for ( ; it != offers.end(); ++it ) {
		KService::Ptr ptr = ( *it );
		// we now know that our offer can handle mimetype and is a part.
		// since it is a part, it must also have a library... let's try to
		// load that now
		factory = KLibLoader::self() ->factory( ptr->library().toLatin1() );
		if ( factory ) {
			kedit_part = static_cast<KParts::ReadWritePart *>( factory->create( this,
			             ptr->name().toLatin1(), "KParts::ReadWritePart" ) );
			if ( kedit_part )
				if ( kedit_part->openUrl( _url ) ) break;
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
	QString mimetype = KMimeType::findByUrl( url ) ->name();
	// ugly hack: don't try to get a part for an XML file, it usually don't work
	if ( mimetype == "text/xml" ) return false;
	if ( url.prettyUrl().startsWith( "man:" ) ) mimetype = "text/html";
	if ( mimetype == "text/plain" )
		viewerMenu->setItemEnabled( 1, false );

	if ( !generic_part ) {
		if ( mimetype.contains( "html" ) ) {
			KHTMLPart * p = new KHTMLPart( this, 0, 0, 0, KHTMLPart::BrowserViewGUI );
			connect( p->browserExtension(), SIGNAL( openUrlRequest( const KUrl &, const KParts::URLArgs & ) ),
			         this, SLOT( handleOpenUrlRequest( const KUrl &, const KParts::URLArgs & ) ) );
			/* At JavaScript self.close() the KHTMLPart destroys itself.  */
			/* After destruction, just close the window */
			connect( p, SIGNAL( destroyed() ), this, SLOT( close() ) );

			p-> openUrl( url );
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
		f_in.open( QIODevice::ReadOnly );
		QDataStream in( &f_in );

		FILE *out = KDE_fopen( tmpFile.name().toLocal8Bit(), "w" );

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
