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

KrViewer::KrViewer(QWidget *parent, const char *name ) :
  KParts::MainWindow(parent,name), manager(this,this){

  //setWFlags(WType_TopLevel | WDestructiveClose);
  setXMLFile( "krviewer.rc" ); // kpart-related xml file
  setHelpMenuEnabled(false);
  
  // KDE HACK START: Viewer fails to resize at maximized mode
  int scnum = QApplication::desktop()->screenNumber(parentWidget());
  QRect desk = QApplication::desktop()->screenGeometry(scnum);
  KGlobal::config()->setGroup( "KrViewerWindow" );
  QSize size( KGlobal::config()->readNumEntry( QString::fromLatin1("Width %1").arg(desk.width()), 0 ),
              KGlobal::config()->readNumEntry( QString::fromLatin1("Height %1").arg(desk.height()), 0 ) );
  if( size.width() > desk.width() )
    KGlobal::config()->writeEntry( QString::fromLatin1("Width %1").arg(desk.width()), desk.width() );
  if( size.height() > desk.height() )
    KGlobal::config()->writeEntry( QString::fromLatin1("Height %1").arg(desk.height()), desk.height() );
  // KDE HACK END
    
  setAutoSaveSettings("KrViewerWindow",true);
  tmpFile.setAutoDelete(true);
  hex_part=generic_part=text_part=editor_part=0L;

  connect(&manager,SIGNAL(activePartChanged(KParts::Part*)),
          this,SLOT(createGUI(KParts::Part*)));
  

  viewerMenu = new QPopupMenu( this );
  viewerMenu->insertItem( i18n("&Generic viewer"), this, SLOT(viewGeneric()), CTRL+Key_G,1 );
  viewerMenu->insertItem( i18n("&Text viewer"),    this, SLOT(viewText()),    CTRL+Key_T,2 );
  viewerMenu->insertItem( i18n("&Hex viewer"),     this, SLOT(viewHex()),     CTRL+Key_H,3 );
  viewerMenu->insertSeparator();
  viewerMenu->insertItem( i18n("Text &editor"),    this, SLOT(editText()), CTRL+Key_E,4 );
  viewerMenu->insertSeparator();
  viewerMenu->insertItem( i18n("&Close"), this, SLOT( close() ), Key_Escape );

  statusBar()->show();
}

KrViewer::~KrViewer(){
  
  disconnect(&manager,SIGNAL(activePartChanged(KParts::Part*)),
              this,SLOT(createGUI(KParts::Part*)));
          
  if( editor_part )
  {
    manager.removePart( editor_part );
    delete editor_part;
    editor_part = 0;
  }

  if( text_part )
  {
    manager.removePart( text_part );
    delete text_part;
    text_part = 0;
  }

  if( generic_part )
  {
    manager.removePart( generic_part );
    delete generic_part;
    generic_part = 0;
  }

  if( hex_part )
  {
    manager.removePart( hex_part );
    delete hex_part;
    hex_part = 0;
  }
}

void KrViewer::slotStatResult( KIO::Job* job ) {
  if( !job || job->error() ) entry = KIO::UDSEntry();
  else entry = static_cast<KIO::StatJob*>(job)->statResult();
  busy = false;
}

KParts::Part* KrViewer::getPart(KURL url, QString mimetype ,bool readOnly, bool create){
  KParts::Part *part = 0L;
  KLibFactory  *factory = 0;
  KTrader::OfferList offers = KTrader::self()->query(mimetype);
  QString typeIn = (readOnly ? "KParts::ReadOnlyPart" : "KParts::ReadWritePart");
  
  if( create )
  {
    KIO::StatJob* statJob = KIO::stat( url, false );
    connect( statJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotStatResult( KIO::Job* ) ) );
    busy = true;
    while ( busy ) qApp->processEvents();
    if( !entry.isEmpty() )
    {
      KFileItem file( entry, url );
      if( file.isReadable() )
        create = false;
    }
  }

  // in theory, we only care about the first one.. but let's try all
  // offers just in case the first can't be loaded for some reason
  KTrader::OfferList::Iterator it(offers.begin());
  for( ; it != offers.end(); ++it) {
    KService::Ptr ptr = (*it);
    // we now know that our offer can handle mimetype and is a part.
    // since it is a part, it must also have a library... let's try to
    // load that now

    QString type = typeIn;
    
    QStringList args;
    QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
    if ( argsProp.isValid() )
    {
      QString argStr = argsProp.toString();
      args = QStringList::split( " ", argStr );
    
      if( ptr->serviceTypes().contains( "Browser/View" ) )
      {
        args << QString::fromLatin1( "Browser/View" );
        type = "Browser/View";
      }
    }
    
    QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );

    if ( !prop.isValid() || prop.toBool() ) // defaults to true
    {
      factory = KLibLoader::self()->factory( ptr->library().latin1() );
      if (factory) {
        part = static_cast<KParts::Part *>(factory->create(this,
                             ptr->name().latin1(), type.latin1(), args ));
        if( part )
        {
          if( !create )
          {
            if( ((KParts::ReadOnlyPart*)part)->openURL( url ) ) break;
            else {
              delete part;
              part = 0L;
            }
          }
          else
          {
            ((KParts::ReadWritePart*)part)->saveAs( url );
            break;
          }
        }
      }
    }
  }
  return part;
}

void KrViewer::createGUI(KParts::Part* part){
  // make sure all the other parts are hidden
  if(generic_part) generic_part->widget()->hide();
  if(text_part)    text_part->widget()->hide();
  if(hex_part)     hex_part->widget()->hide();
  if(editor_part)  editor_part->widget()->hide();

  // and show the new part widget
 	connect(part,SIGNAL(setStatusBarText(const QString&)),
          this,SLOT(slotSetStatusBarText(const QString&)));
	KParts::MainWindow::createGUI(part);
  setCentralWidget(part->widget());
  part->widget()->show();

  // and "fix" the menubar
  menuBar()->removeItem(70);
  menuBar()->insertItem( i18n("&KrViewer"), viewerMenu,70 );
  menuBar()->show();
}

void KrViewer::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
    case Key_F10:
    case Key_Escape:
      close();
      break;
		default:
			e->ignore();
			break;
  }
}

void KrViewer::view(KURL url){
  KrViewer* viewer = new KrViewer(krApp);

  viewer->url = url;
  viewer->setCaption("KrViewer: "+url.url());
  viewer->show();

  if( !viewer->viewGeneric() ){
    if( !viewer->viewText() )
    {
      viewer->destroy();
      return;
    }
    viewer->viewerMenu->setItemEnabled(1,false);
  }
}

void KrViewer::edit(KURL url, bool create){
  krConfig->setGroup( "General" );
  QString edit = krConfig->readEntry( "Editor", _Editor );

  if ( edit != "internal editor" ) {
    KProcess proc;
    // if the file is local, pass a normal path and not a url. this solves
	 // the problem for editors that aren't url-aware
	 if (url.isLocalFile())
	 	proc << QStringList::split(' ', edit) << url.path();
	 else proc << QStringList::split(' ', edit) << url.prettyURL();
	 if ( !proc.start( KProcess::DontCare ) )
      KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
    return;
  }

  KrViewer* viewer = new KrViewer(krApp);

  viewer->url = url;
  viewer->setCaption("KrEdit: "+url.url());
  viewer->show();

  if( !viewer->editText( create ) )
  {
      viewer->destroy();
      return;
  }
}

bool KrViewer::editGeneric(QString mimetype, KURL _url){
  KParts::ReadWritePart *kedit_part = 0L;
  KLibFactory *factory = 0;
  KTrader::OfferList offers = KTrader::self()->query(mimetype);

  // in theory, we only care about the first one.. but let's try all
  // offers just in case the first can't be loaded for some reason
  KTrader::OfferList::Iterator it(offers.begin());
  for( ; it != offers.end(); ++it) {
    KService::Ptr ptr = (*it);
    // we now know that our offer can handle mimetype and is a part.
    // since it is a part, it must also have a library... let's try to
    // load that now
    factory = KLibLoader::self()->factory( ptr->library().latin1() );
    if (factory) {
      kedit_part = static_cast<KParts::ReadWritePart *>(factory->create(this,
                           ptr->name().latin1(), "KParts::ReadWritePart"));
      if( kedit_part )
        if(kedit_part->openURL(_url) ) break;
        else {
          delete kedit_part;
          kedit_part = 0L;
        }
    }
  }

  if (!kedit_part){
    KMessageBox::error(this,i18n("Sorry, can't find internal editor"));
    return false;
  }

  setCentralWidget(kedit_part->widget());
  createGUI(kedit_part);
  kedit_part->widget()->show();
  return true;
}

bool KrViewer::editText( bool create ){
  if( !editor_part ){
    editor_part = static_cast<KParts::ReadWritePart*>(getPart(url,"text/plain",false,create));
    if( !editor_part ) return false;
    manager.addPart(editor_part,this);
  }
  manager.setActivePart(editor_part);
  return true;
}

bool KrViewer::viewGeneric(){
  QString mimetype = KMimeType::findByURL( url )->name();
  // ugly hack: don't try to get a part for an XML file, it usually don't work
  if( mimetype == "text/xml" ) return false;
  if( url.prettyURL().startsWith("man:") ) mimetype = "text/html";
  if( mimetype == "text/plain" )
    viewerMenu->setItemEnabled(1,false);

  if( !generic_part ){
    if( mimetype.contains("html") ){
      KHTMLPart* p = new KHTMLPart(this,0,0,0,KHTMLPart::BrowserViewGUI);
      connect(p->browserExtension(), SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
              this,SLOT(handleOpenURLRequest(const KURL &,const KParts::URLArgs & )));

      p-> openURL(url);
      generic_part = p;
    } else {
      generic_part = static_cast<KParts::ReadOnlyPart*>(getPart(url,mimetype,true));
    }
    if( generic_part ) manager.addPart(generic_part,this);
    else return false;
  }

  manager.setActivePart(generic_part);
  return true;
}

bool KrViewer::viewText(){
  if( !text_part ){
    text_part = static_cast<KParts::ReadOnlyPart*>(getPart(url,"text/plain",true));
    if( !text_part ) return false;    
    manager.addPart(text_part,this);
  }
  manager.setActivePart(text_part);
  return true;
}

void KrViewer::viewHex(){
  if( !hex_part ){
    QString file;
    // files that are not local must first be downloaded
    if( !url.isLocalFile() ){
      if( !KIO::NetAccess::download( url, file ) ){
        KMessageBox::sorry(this,i18n("KrViewer is unable to download: ")+url.url());
        return;
      }
    } else file = url.path();


    // create a hex file
    QFile f_in( file );
    f_in.open( IO_ReadOnly );
    QDataStream in( &f_in );

    FILE *out = KDE_fopen(tmpFile.name().local8Bit(),"w");

    KIO::filesize_t fileSize = f_in.size();
    KIO::filesize_t address = 0;
    char buf[16];
		unsigned int* pBuff = (unsigned int*)buf;

    while( address<fileSize ){
			memset(buf,0,16);
			int bufSize = ((fileSize-address) > 16)? 16 : (fileSize-address);
      in.readRawBytes(buf,bufSize);
			fprintf(out,"0x%8.8llx: ",address);
			for(int i=0; i<4; ++i){
				if(i<(bufSize/4)) fprintf(out,"%8.8x ",pBuff[i]);
				else fprintf(out,"         ");
			}
			fprintf(out,"| ");

      for(int i=0; i<bufSize; ++i){
        if(buf[i]>' ' && buf[i]<'~' ) fputc(buf[i],out);
        else fputc('.',out);
      }
      fputc('\n',out);

      address += 16;
    }
    // clean up
    f_in.close();
		fclose(out);
    if( !url.isLocalFile() )
      KIO::NetAccess::removeTempFile( file );

    hex_part = static_cast<KParts::ReadOnlyPart*>(getPart(tmpFile.name(),"text/plain",true));
    if( !hex_part ) return;
    manager.addPart(hex_part,this);
  }
  manager.setActivePart(hex_part);
}

void KrViewer::handleOpenURLRequest( const KURL &url, const KParts::URLArgs & ){
  if(generic_part) generic_part->openURL(url);

}

bool KrViewer::queryClose()
{
  if( editor_part && editor_part->queryClose() == false )
    return false;
    
  return true;
}

bool KrViewer::queryExit()
{
  kapp->ref(); // FIX: krusader exits at closing the viewer when minimized to tray
  return true; // don't let the reference counter reach zero
}


#include "krviewer.moc"
