/***************************************************************************
                                 kviewer.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site         		 : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// QT includes
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qtextbrowser.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlayout.h>
#include <qvaluelist.h>
// KDE includes
#include <kmimetype.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kparts/part.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kcursor.h>
#include <kmessagebox.h>
// Krusader includes
#include "kviewer.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../resources.h"
//#include "../resources.h"
#include "kviewerwidget.h"


void KViewer::view(QString fileName, QString mimetype) {
  krApp->setCursor(KCursor::waitCursor());

  KURL url = fileName;
  QString tmpFile = QString::null;

  // the following part is for local files
  if (url.isLocalFile()) {
     // translate symlink to real names
     for( QFileInfo qf(fileName); qf.isSymLink(); qf.setFile(fileName) ){
       fileName = qf.dir().absFilePath(qf.readLink());
     }
     if( !QDir().exists(fileName) ) return;  // safty
     tmpFile = fileName;
  } else {
     // files that are not local must first be downloaded
     // unless for html files that is : )
     if( mimetype.left(9)!="text/html" ){
       if( !KIO::NetAccess::download( url, tmpFile ) ){
         KMessageBox::sorry(krApp,i18n("KViewer is unable to download: ")+fileName);
         return;
       }
     } else tmpFile = fileName;
  }
  url.setPath(tmpFile);
  mimetype = KMimeType::findByURL(url,0,true,false)->name();

  short Type=0;                            // the default is a KPART viewer
  if ( mimetype.left(5)=="image") Type=1;  // image
  if ( mimetype.contains("text") &&
      !mimetype.contains("html")) Type=2;  // text
  if ( mimetype.contains("executable") ||
       mimetype.contains("object-file")||
       mimetype.contains("octet-stream") ) Type =3; // binary format

  switch (Type) {
    case 0 : viewGeneric(tmpFile,mimetype);           break;
    case 1 : viewImage(tmpFile);                      break;
    case 2 : viewText(tmpFile);                       break;
    case 3 : viewHex(tmpFile);                        break;
  }
  // delete temp file ( if there is one )
  if( tmpFile != fileName ) KIO::NetAccess::removeTempFile( tmpFile );
  krApp->setCursor(KCursor::arrowCursor());
}

void KViewer::viewImage(QString fileName) {
	// first, open and load the image
  QPixmap image;
  if (!image.load(fileName)) {
    KMessageBox::sorry(krApp,i18n("KViewer is unable to load the image."));
    return;
  }
  // now, to the widget...	
  QWidget *dock=new KViewerWidget();
	// check if the picture is bigger than the screen
	short sw=QApplication::desktop()->width();
	short sh=QApplication::desktop()->height();
	if (image.width() > sw || image.height() > sh-90) {
    // perspectively-scale down the image to a viewable size
    short newHeight=sh-90; // allow space for a large-panel
    short newWidth=image.width()*newHeight/image.height(); // perspective
	  QImage temp=image.convertToImage();
	  temp=temp.smoothScale(newWidth,newHeight);
	  if (!image.convertFromImage(temp,QPixmap::AutoColor)) {
	    KMessageBox::sorry(krApp,i18n("KViewer is unable to load the image."));
      return;
    }
	}
	dock->resize(image.width(), image.height());
	dock->setBackgroundPixmap(image);
	dock->setCaption("(image) KViewer: "+fileName);
	
	// let's go.
	dock->show();	
}

void KViewer::viewGeneric(QString fileName, QString mimetype, QString caption) {
  KParts::ReadOnlyPart *kviewer_part = 0L;
  KLibFactory *factory = 0;
  KTrader::OfferList offers = KTrader::self()->query(mimetype.latin1());

  QWidget *viewer = new KViewerWidget();
  QVBoxLayout* layout=new QVBoxLayout(viewer);

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
      kviewer_part = static_cast<KParts::ReadOnlyPart *>(factory->create(viewer,
                           ptr->name().latin1(), "KParts::ReadOnlyPart"));
      if( kviewer_part && kviewer_part->openURL(fileName) ) break;
      else {
        delete kviewer_part;
        kviewer_part = 0L;
      }
    }
  }

  if (!kviewer_part){
    delete viewer;  // also delete the layout
    viewSpecial(fileName);
    return;
  }

  layout->addWidget(kviewer_part->widget());
  // let's go.
  if (caption.isEmpty()) caption = fileName;
	viewer->setCaption("KViewer: "+caption);
  viewer->resize(krApp->width()*2/3,krApp->height()*2/3);	
  viewer->show();
}

// this function is called if the file isn't an html or an image, so
// a simple editor is used
void KViewer::viewText(QString fileName) {
	// first, create a stream to the file
	QFile file(fileName);
	file.open(IO_ReadOnly);
  // now, to the widget...	
	KViewerWidget *dock = new KViewerWidget();
	KREdit *viewer = new KREdit(dock);
  connect(dock,SIGNAL(search()),viewer,SLOT(krsearch()));
  QVBoxLayout* layout=new QVBoxLayout(dock); // <patch> thanks to Aurelien Gateau
	viewer->setReadOnly(true);

  // since we want a QUICK view - we read only the first 5000 lines.
  // we also don't read more than 200 chars per line - lines longer than that
  // are probably part of a binary file...
	QString line;
  for(int i = 0; i < 5000 ; ++i){
    if ( file.readLine(line,200)== -1 ) break;
		else{
		  if(line.right(1) == "\n" ) line.truncate(line.length()-1);
		  viewer->insertLine(line);
	  }
	}

  //if we cut the file short - notify the user.
	if(file.readLine(line,10) != -1){
		viewer->insertLine("\n********* KRUSADER MESSAGE ********");
		viewer->insertLine("ONLY THE FIRST 5000 LINES ARE SHOWN:");
		viewer->insertLine("USE F4 TO SEE THE WHOLE FILE.");
  }

  file.close();

  // let's go.
	layout->addWidget(viewer);
	dock->setCaption("KViewer: "+fileName);
  dock->resize(krApp->width()*2/3,krApp->height()*2/3);	
	dock->show();
}

void KViewer::viewSpecial(QString fileName){
  // get the file info from the 'file' command.
  QString tmpFile = krApp->getTempFile();
  KShellProcess proc;
  proc << "file"<< "\""+fileName+"\"" <<" > "+tmpFile ;
  proc.start(KProcess::Block);

  // create a stream to the file
	QFile file(tmpFile);
	file.open(IO_ReadOnly);
  QString line;
  QString fileInfo;
  while( file.readLine(line,200) != -1 ){
    if(line.right(1) == "\n" ) line.truncate(line.length()-1);
    fileInfo += line + '\n';
  }
  QDir().remove(tmpFile);
  kdWarning() << fileInfo << endl;

  int res = KMessageBox::warningYesNoCancel(0, i18n(
     "Krusader can't find a viewer for %1 How do you want to view it? Warning: Krusader may freeze or crash if you'll try to view a special file !").
     arg(fileInfo), i18n("Unknown viewer"), i18n("&Text view"), i18n("&Hex dump"));
  if (res == KMessageBox::Yes) viewText(fileName);
  if (res == KMessageBox::No) viewHex(fileName);
}

void KViewer::viewHex(QString fileName){
  QString tmpFile = krApp->getTempFile();
  KShellProcess proc;
  proc << "hexdump -v "<< "\""+fileName+"\"" <<" > "+tmpFile ;
  proc.start(KProcess::Block);

  viewGeneric(tmpFile,"text/plain",fileName);
  QDir().remove(tmpFile);
}
