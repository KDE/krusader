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
#include <kstatusbar.h>
#include <kdebug.h>
// Krusader includes
#include "krviewer.h"
#include "../krusader.h"


KrViewer::KrViewer(QWidget *parent, const char *name ) :
  KParts::MainWindow(parent,name), manager(this,this){
  //setWFlags(WType_TopLevel | WDestructiveClose);
  setXMLFile("krviewerui.rc");
  setHelpMenuEnabled(false);
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
	//kdDebug() << "KrViewer killed" << endl;
}

KParts::Part* KrViewer::getPart(KURL url, QString mimetype ,bool readOnly){
  KParts::Part *part = 0L;
  KLibFactory  *factory = 0;
  KTrader::OfferList offers = KTrader::self()->query(mimetype);
  QString type = (readOnly ? "KParts::ReadOnlyPart" : "KParts::ReadWritePart");

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
      part = static_cast<KParts::Part *>(factory->create(this,
                           ptr->name().latin1(), type.latin1() ));
      if( part )
        if( ((KParts::ReadOnlyPart*)part)->openURL(url) ) break;
        else {
          delete part;
          part = 0L;
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
  QString mimetype = KMimeType::findByURL( url )->name();
  KrViewer* viewer = new KrViewer(krApp);

  viewer->url = url;
  viewer->setCaption("KrViewer: "+url.url());
  viewer->show();

  if( !viewer->viewGeneric() ){
    viewer->viewText();
    viewer->viewerMenu->setItemEnabled(1,false);
  }
}

void KrViewer::edit(KURL url){
  KrViewer* viewer = new KrViewer(krApp);

  viewer->url = url;
  viewer->setCaption("KrEdit: "+url.url());
  viewer->show();

  viewer->editText();
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

void KrViewer::editText(){
  if( !editor_part ){
    editor_part = static_cast<KParts::ReadWritePart*>(getPart(url,"text/plain",false));
    manager.addPart(editor_part,this);
  }
  manager.setActivePart(editor_part);
}

bool KrViewer::viewGeneric(){
  QString mimetype = KMimeType::findByURL( url )->name();
  if( mimetype == "text/plain" )
    viewerMenu->setItemEnabled(1,false);

  if( !generic_part ){
     generic_part = static_cast<KParts::ReadOnlyPart*>(getPart(url,mimetype,true));
     if( generic_part ) manager.addPart(generic_part,this);
     else return false;
  }

  manager.setActivePart(generic_part);
  return true;
}

void KrViewer::viewText(){
  if( !text_part ){
    text_part = static_cast<KParts::ReadOnlyPart*>(getPart(url,"text/plain",true));
    manager.addPart(text_part,this);
  }
  manager.setActivePart(text_part);
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
    } else file = url.url().mid(url.url().find("/"));


    // create a hex file
    QFile f_in( file );
    f_in.open( IO_ReadOnly );
    QDataStream in( &f_in );

    FILE *out = fopen(tmpFile.name().mid(tmpFile.name().find("/")).latin1(),"w");

    KIO::filesize_t fileSize = f_in.size();
    long address = 0;
    char buf[16];
		unsigned int* pBuff = (unsigned int*)buf;

    while( address<fileSize ){
			memset(buf,0,16);
			int bufSize = ((fileSize-address) > 16)? 16 : (fileSize-address);
      in.readRawBytes(buf,bufSize);
			fprintf(out,"0x%8.8lx: ",address);
			for(int i=0; i<4; ++i){
				if(i<(bufSize/4)) fprintf(out,"%8.8x ",pBuff[i]);
				else fprintf(out,"         ");
			}
			fprintf(out,"| ");
      /*fprintf(out,"0x%8.8lx: %8.8x %8.8x %8.8x %8.8x | ",address,pBuff[0],pBuff[1],pBuff[2],pBuff[3]); */

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
    manager.addPart(hex_part,this);
  }
  manager.setActivePart(hex_part);
}
