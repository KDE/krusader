/***************************************************************************
                                quickviewpanel.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
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
#include <qtextstream.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>
// KDE includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
// Krusader includes
#include "kvfspanel.h"
#include "../krusader.h"
#include "panelfunc.h"
#include "../defaults.h"
#include "kfilelist.h"
#include "../VFS/vfs.h"
#include "../VFS/vfile.h"

QuickViewPanel::QuickViewPanel(QWidget *parent,const char *name) :
    KVFSPanel(parent,name){
  type = "quickview";
	func = new QuickViewPanelFunc();
	
  setAcceptDrops(false);
	layout=new QGridLayout(this,2,1);
  // totals will show how many dirs are marked
  totals = new KSqueezedTextLabel(this);
  status = new KSqueezedTextLabel(this);
  fileList = new KFileList(this); // the same KFilelist can do trees!


  origin = new QPushButton("/",this);
 	krConfig->setGroup("Look&Feel");
 	origin->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
	origin->font().setBold(true);
	origin->font().setPointSize(origin->font().pointSize()+2);		
	origin->setText("Quick View Panel");
	connect(origin,SIGNAL(clicked()),this,SLOT(slotFocusOnMe()));

  about = KGlobal::dirs()->findResource("appdata","about.png");
	
	textViewer 	= 0;
  htmlViewer 	= 0;
  imageViewer = 0;

	inWork = false;
	defaultIsOn = false;
	workspace = new QWidget(this);
	workspace->setBackgroundColor(QColor(0,0,0));

	wsLayout = new QGridLayout(workspace,1,1);
	wsLayout->setRowStretch(1,10);
  wsLayout->setColStretch(1,10);
 	
	// connections
	connect(this,SIGNAL(activePanelChanged(KVFSPanel *)),this->parent()->parent()->parent(),
					SLOT(slotSetActivePanel(KVFSPanel *)) );
	 	// make sure that a focus/path change reflects in the command line and activePanel
	connect(this,SIGNAL(cmdLineUpdate(QString)),this->parent()->parent()->parent(),
					SLOT(slotCurrentChanged(QString)) );
	// finish the layout
	layout->addWidget(origin,0,0);
	layout->addWidget(workspace,1,0);
}

void QuickViewPanel::slotFocusOnMe(){		 // give this VFS the focus (the path bar)
  // we start by calling the KVFS function
  KVFSPanel::slotFocusOnMe();
  // then we set up actions
  krProperties->setEnabled(false);      // file properties
  krUnpack->setEnabled(false);          // unpack archive
  krTest->setEnabled(false);            // test archive
  krSelect->setEnabled(false);          // select a group by filter
  krSelectAll->setEnabled(false);       // select all files
  krUnselect->setEnabled(false);        // unselect by filter
  krUnselectAll->setEnabled(false);     // remove all selections
  krInvert->setEnabled(false);          // invert the selection
  krFTPConnect->setEnabled(false);      // connect to an ftp
  krFTPNew->setEnabled(false);          // create a new connection
  krFTPDiss->setEnabled(false);         // disconnect an FTP session
  krFullPanel->setEnabled(true);        // switch to list panel
  krTreePanel->setEnabled(true);        // switch to tree panel
  krQuickPanel->setEnabled(false);      // switch to quick view panel
  krAllFiles->setEnabled(false); 				// show all files in list
  krExecFiles->setEnabled(false);      	// show only executables
  krCustomFiles->setEnabled(false);    	// show a custom set of files
  krBack->setEnabled(false);            // go back
  krRoot->setEnabled(false);            // go all the way up
  krAddBookmark->setEnabled(false);     // add a bookmark
	krCompare->setEnabled(false);         // compare by content
	krCompareDirs->setEnabled(false); compareMode=false; otherPanel->compareMode=false;
	emit cmdLineUpdate(getPath());
}

void QuickViewPanel::start(bool){
	// connect the other panel current-changed  signal to our view slot,
	connect(otherPanel->fileList,SIGNAL(currentChanged(QListViewItem *)),
          this,SLOT(view(QListViewItem *)));
}

void QuickViewPanel::view(QListViewItem * item){
	if(inWork) return; // don't allow 2 view functions in the same time
	inWork = true;
  short Type=0;
  QString fileName = "";

	vfile* vf = otherPanel->files->vfs_search(fileList->getFilename(item));
	if(vf == 0 ) Type = 3;
	else {
	  // first get the file
	  fileName = otherPanel->files->vfs_getFile(fileList->getFilename(item));

	  // then determine the type
	  QString mime = vf->vfile_getMime();
	  // the default is a text/binary unknown file
    if (mime.contains("html"))      Type=1;  // html
    if (mime.contains("image")) 		Type=2;      // image
	  if ( vf->vfile_isDir() )        Type=3;
	}
	// save time and flicker  - don't load the default if it's loaded
  if( Type==3 && defaultIsOn ){
		inWork = false; // finished working
		return;
	}
	else defaultIsOn = false;
	
	// if we have an active viewer - kill it
	if(textViewer!=0) { delete textViewer;  textViewer = 0; }
	if(htmlViewer!=0) { delete htmlViewer;  htmlViewer = 0; }
	if(imageViewer!=0){ delete imageViewer; imageViewer= 0; }
	
  switch (Type) {
    case 0 : viewGeneric (fileName);break;
    case 1 : viewHtml (fileName);   break;
    case 2 : viewImage(fileName);   break;
		case 3 : viewDefault();   			break;
  }
	inWork = false; // finished working
}

void QuickViewPanel::viewImage(QString fileName) {
  // first, open and load the image
  QPixmap temp;
  QImage image;
	imageViewer = new QWidget(workspace);

	if( fileName == QString::null ) fileName=about;
	
  if (!image.load(fileName)) {
    KMessageBox::sorry(krApp,i18n("KViewer is unable to load the image."));
    return;
  }
  // now, to the widget...	
  int sw=workspace->width();
	int sh=workspace->height();
	// perspectively-scale down the image to a viewable size
  int newHeight=image.height() ,newWidth=image.width() ;
	bool scale = false;
	
	if( newHeight > sh ){
  	newHeight = sh;
		newWidth = newHeight*image.width()/image.height();
		scale = true;
	}	
  if( newWidth > sw ){
		newWidth = sw;
  	newHeight = newWidth*image.height()/image.width();
		scale = true;
	}
	
	if( scale ) image=image.smoothScale(newWidth,newHeight);
	temp.convertFromImage(image,QPixmap::AutoColor);
	
	imageViewer = new QWidget(workspace);
	imageViewer->setBackgroundPixmap(temp);
  imageViewer->resize(newWidth,newHeight );
	imageViewer->setFixedSize(QSize(newWidth,newHeight));
	imageViewer->show();
	wsLayout->addWidget(imageViewer,1,1);
	workspace->show();
}

void QuickViewPanel::viewHtml(QString fileName) {
	// first, create a stream to the file
	QFile file(fileName);
	file.open(IO_ReadOnly);
	QTextStream text( &file );

  // now, to the widget...
  htmlViewer=new QTextBrowser(workspace);
	htmlViewer->setSource(fileName);

  // let's go.
	wsLayout->addWidget(htmlViewer,1,1);
	htmlViewer->repaint();
	htmlViewer->show();
	workspace->repaint();
	workspace->show();
}

// this function is called if the file isn't an html or an image, so
// a simple editor is used
void QuickViewPanel::viewGeneric(QString fileName) {
	// first, create a stream to the file
	QFile file(fileName);
	file.open(IO_ReadOnly | IO_Async | IO_Sequential);
	// now, to the widget...	
	textViewer = new KEdit(workspace);
	textViewer->setReadOnly(true);
	QString line;
	
  // since we want a QUICK view - we read only the first 500 lines.
  // we also don't read more than 200 chars per line - lines longer than that
  // are probably part of a binary file...
	for(int i = 0; i < 500 ; ++i){
    if ( file.readLine(line,200)== -1 ) break;
		else{
		  if(line.right(1) == "\n" ) line.truncate(line.length()-1);
		  textViewer->insertLine(line);
	  }
	}

  //if we cut the file short - notify the user.
	if(file.readLine(line,10) != -1){
		textViewer->insertLine("\n********* KRUSADER MESSAGE ********");
		textViewer->insertLine("ONLY THE FIRST 500 LINES ARE SHOWN:");
		textViewer->insertLine("USE F3/F4 TO SEE THE WHOLE FILE.");
  }
	file.close();
	// let's go.
	wsLayout->addWidget(textViewer,1,1);
	textViewer->repaint();
	textViewer->show();
	workspace->repaint();
	workspace->show();
	otherPanel->fileList->setFocus();
}

// show a default image when we start or trying to view a directory
void QuickViewPanel::viewDefault(){
//	workspace->setBackgroundColor(QColor(0,0,0));
//	workspace->repaint();
//	workspace->show();
	viewImage(about);
	defaultIsOn = true;
}
