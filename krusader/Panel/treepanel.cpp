/***************************************************************************
                                treepanel.cpp
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
#include <qdragobject.h>
#include <qmessagebox.h>
#include <qheader.h>
// KDE includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
// Krusader includes
#include "kcursor.h"
#include "kvfspanel.h"
#include "krlistitem.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"
#include "../resources.h"
#include "../VFS/tree_vfs.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krdirwatch.h"
#include "../MountMan/kmountman.h"
#include "panelfunc.h"
#include "kfilelist.h"

////////////////////////////////////////////////////
//           The TREE panel constructor           //
////////////////////////////////////////////////////
TreePanel::TreePanel(QWidget *parent,const char *name) :
    KVFSPanel(parent,name), currDragItem(0), statsAgent(0) {
  type = "tree";
	func = new TreePanelFunc(this);
	files= new tree_vfs("/",this);
	
	watcher = new KRdirWatch();

  setAcceptDrops(true);
	layout=new QGridLayout(this,4,1);
  // totals will show how many dirs are marked
  totals = new KSqueezedTextLabel(this);
  krConfig->setGroup("Look&Feel");
  totals->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  totals->setFrameStyle( QFrame::Box | QFrame::Raised);
  totals->setLineWidth(1);		// a nice 3D touch :-)

 	origin = new QPushButton("/",this);
  krConfig->setGroup("Look&Feel");
	origin->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
	origin->font().setBold(true);
	origin->font().setPointSize(origin->font().pointSize()+2);		
		// pressing the origin bar sets focus
	connect(origin,SIGNAL(clicked()),this,SLOT(slotFocusOnMe()));

	status = new KSqueezedTextLabel(this);
  status->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  status->setFrameStyle( QFrame::Box | QFrame::Raised);
  status->setText("");        // needed for initialization code!

  status->setLineWidth(1);		// a nice 3D touch :-)
  fileList = new KFileList(this); // the same KFilelist can do trees!
  fileList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
	fileList->setMultiSelection(false);
	fileList->setVScrollBarMode(QScrollView::AlwaysOn);
	fileList->setHScrollBarMode(QScrollView::Auto);
	fileList->setRootIsDecorated(true);
	fileList->setShowSortIndicator(true);	
	int i=QFontMetrics(fileList->font()).width("W");
	fileList->addColumn("Name",i*37); // create an even look between the panels
	// connections
	connect(fileList, SIGNAL(expanded(QListViewItem*)),
					this,SLOT(slotExpandTree(QListViewItem*)));
	connect(fileList, SIGNAL(collapsed(QListViewItem*)),
					this,SLOT(slotCollapsedTree(QListViewItem*)));
  // when fileList wants us to drag ...
  connect(fileList, SIGNAL(letsDrag(int)), this, SLOT(startDragging(int)));	
  // when the drag is finished
  connect(this,SIGNAL(finishedDragging()), fileList, SLOT(finishedDragging()));
  	
  	// double click and pressing return executes
  connect(fileList, SIGNAL(doubleClicked(QListViewItem *)), this,
          SLOT(slotChangeFromTree(QListViewItem*)));
  connect(fileList, SIGNAL(returnPressed(QListViewItem *)), this,
          SLOT(slotChangeFromTree(QListViewItem*)));
	// even a "drag-swush" through the list changes focus
	connect(fileList, SIGNAL(mouseButtonPressed(int,QListViewItem *,const QPoint &,int)),
					this,SLOT(slotFocusOnMe()));
  // when selection change..
	connect(fileList, SIGNAL(selectionChanged(QListViewItem *)),
	        this,SLOT(slotSelectionChanged(QListViewItem *)));
  // and not forgetting to reroute active panel!!
	connect(this,SIGNAL(activePanelChanged(KVFSPanel *)),this->parent()->parent()->parent(),
					SLOT(slotSetActivePanel(KVFSPanel *)) );
	// make sure that a focus/path change reflects in the command line and activePanel
	connect(this,SIGNAL(cmdLineUpdate(QString)),this->parent()->parent()->parent(),
					SLOT(slotCurrentChanged(QString)) );		
	connect(fileList, SIGNAL(rightClickMenu(QListViewItem*,QPoint)), this,
          SLOT(popRightClickMenu(QListViewItem*,QPoint)));
  // connect the watcher to panel refresh
	connect(watcher,SIGNAL(dirty()),this,SLOT(refresh()));
 	
	// finish the layout
	layout->addWidget(origin,0,0);
	layout->addWidget(status,1,0);
	layout->addWidget(fileList,2,0);
  layout->addWidget(totals,3,0);
}

void TreePanel::slotRecreateTree(QListViewItem *item) {
  if (item == 0){
    fileList->clear();
    // insert the root item to the list manually
    item=new QListViewItem(fileList,"/");
  }
  QString path = returnPath(item);
  // if the item was current before - it should also be now
  if( virtualPath == path )
    fileList->setCurrentItem(item);

  // if the item was not opened before - don't open it now
  if ( !openItems.contains(path) ){
    item->setPixmap(0,FL_LOADICON("folder"));
    return;
  }

  // if the path have child directories - add them and recreate them
  if ( QDir(path,QString::null,QDir::Dirs|QDir::Hidden|QDir::NoSymLinks).count() > 0 ){
    item->setExpandable(true); // item has children ...
    item->setOpen(true);       // ... show them
    item->setPixmap(0,FL_LOADICON("folder"));
    // try to recreate all child directories
    for( QListViewItem *child = item->firstChild(); child; child = child->nextSibling() )
      slotRecreateTree(child);
  }
  else{
    item->setPixmap(0,FL_LOADICON("folder"));
    item->setExpandable(false);
    // update the openItems list
    for (QStringList::Iterator it = openItems.begin(); it != openItems.end();){
      if ( (*it).left(path.length()) == path) it = openItems.remove(it);
      else ++it;
    }
  }
  fileList->triggerUpdate();
}

void TreePanel::refresh(const QString){
  // change the cursor to busy
	krApp->setCursor(KCursor::waitCursor());
	
  watcher->stopScan(); //stop watching the old dir
	watcher->clearList();
	slotRecreateTree();
	watcher->startScan();

  // return cursor to normal arrow
	krApp->setCursor(KCursor::arrowCursor());
}


void TreePanel::slotFocusOnMe(){		 // give this VFS the focus (the path bar)
  // we start by calling the KVFS function
  KVFSPanel::slotFocusOnMe();
  // then we set up actions
  krProperties->setEnabled(true);       // file properties
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
  krTreePanel->setEnabled(false);       // switch to tree panel
  krQuickPanel->setEnabled(true);       // switch to quick view panel
  krAllFiles->setEnabled(false); 				// show all files in list
  krExecFiles->setEnabled(false);      	// show only executables
  krCustomFiles->setEnabled(false);    	// show a custom set of files
  krBack->setEnabled(false);            // go back
  krRoot->setEnabled(false);            // go all the way up
  krAddBookmark->setEnabled(false);     // add a bookmark
	krCompare->setEnabled(false);         // compare by content
  krCompareDirs->setEnabled(false); compareMode=false; otherPanel->compareMode=false;
	krMultiRename->setEnabled(false);
}

// this is used to start the panel, AFTER setOther() has been used
//////////////////////////////////////////////////////////////////
void TreePanel::start(bool) {
	openItems.append("/");
	refresh("");
	// set "/" as the currentItem
  if ( !fileList->firstChild() ){
		fileList->setCurrentItem(fileList->firstChild());
  	slotSelectionChanged(fileList->firstChild());
  }
	watcher->startScan();
}

// readTree reads the tree and creates a tree-view Panel
/////////////////////////////////////////////////////
int TreePanel::readTree(QString path,QListViewItem *father) {
  KRListItem *item;
  int i=0;
  QDir d(path);

  // read only true directories (no symlinks), including hidden and sort by name
  d.setFilter( QDir::Dirs | QDir::Hidden | QDir::NoSymLinks );
  d.setSorting(QDir::Name);

  const QFileInfoList *list = d.entryInfoList();
  QFileInfoListIterator it( *list );      // create list iterator
  QFileInfo *fi;                          // pointer for traversing

  while ( (fi=it.current()) ) {           // for each directory...
    if ((fi->fileName()!=".") && (fi->fileName()!="..")) {
      item=new KRListItem(father,fi->fileName());
      item->setPixmap(0,FL_LOADICON("folder"));
      if (path+"/"+fi->fileName() != "/mnt" && (path+"/"+fi->fileName()).left(5) != "/proc" )
        watcher->addDir(path+"/"+fi->fileName());
      // this part checks if the dir should have "expandable" icon
      ////////////////////////////////////////////////////////////
      QDir temp(path+"/"+fi->fileName());
      temp.setFilter( QDir::Dirs | QDir::Hidden | QDir::NoSymLinks );
      if (temp.count()>2) item->setExpandable(true);
      ////////////////////////////////////////////////////////////
      i++;                                // counter
    }
    ++it;                               // goto next list element
  }
  return i;
}

void TreePanel::slotExpandTree(QListViewItem *item) {
  QString path=returnPath(item);
  // call readTree to do it's job
  readTree(path,item);

  item->setPixmap(0,FL_LOADICON("folder_open")); // set the icon
  openItems.append(path);             // update the openItems list
  fileList->triggerUpdate();          // and update the list
}

void TreePanel::slotCollapsedTree(QListViewItem *item) {
  item->setPixmap(0,FL_LOADICON("folder")); // set the icon
  QListViewItem *temp,*child = item->firstChild();
  while( child ){
    temp = child;
    child = child->nextSibling();
    delete temp;
  }
  // update the openItems list
  QString path = returnPath(item);
  for (QStringList::Iterator it = openItems.begin(); it != openItems.end();){
    if ( (*it) != "/" && (*it).left(path.length()) == path ){
      watcher->removeDir(*it);
      it = openItems.remove(it);
    }
    else
      ++it;
  }
  fileList->triggerUpdate();          // and update the list

}

QString TreePanel::getPath(){return returnPath( fileList->currentItem() );}

// returns a full path originating at root until hitting the ITEM
/////////////////////////////////////////////////////////////////
QString TreePanel::returnPath(QListViewItem *item) {
  QString path=fileList->getFilename(item);

  QString s;
  // 1st, create the full path to the expanded directory
  QListViewItem *iterator=item->parent();
  while (iterator!=0) {
    s=fileList->getFilename(iterator)=="/" ? "" : "/";
    path=(fileList->getFilename(iterator))+s+path;
    iterator=iterator->parent();
  }
  return path;
}

void TreePanel::slotSelectionChanged(QListViewItem *item){
  origin->setText(returnPath(item));
  files->vfs_origin = virtualPath = realPath = returnPath(item);

  // set the writable and readable attribute
	files->isWritable = KRpermHandler::fileWriteable(realPath);
	files->supportMoveFrom = KRpermHandler::fileReadable(realPath);

  emit cmdLineUpdate(realPath);
  slotGetStats(realPath);
}

void TreePanel::slotGetStats(QString path) {
  QString templabel,label;

	if (files->vfs_getType()=="ftp") {
    status->setText(i18n("No space information on remote filesystems"));
    return;
  }

  // 1st, get information
  if (!krMtMan.operational()) {       // don't try if no mountman !! :-(
    status->setText(i18n("Mt.Man is inactive. sorry."));
    return;
  } else status->setText(i18n("Mt.Man: working ..."));
  statsAgent = new MountMan::statsCollector(path,this);
}

void TreePanel::gotStats(QString data) {
  status->setText(data);
  if (statsAgent) delete statsAgent;
}

// pops a right-click menu for items
void TreePanel::popRightClickMenu(KListView *kl, QListViewItem* item,const QPoint &loc) {
  // these are the values that will always exist in the menu
  #define OPEN_ID       90
  #define OPEN_WITH_ID  91
  #define OPEN_KONQ_ID  92
  #define OPEN_TERM_ID  93
  #define CHOOSE_ID     94
  #define DELETE_ID     95
  #define COPY_ID       96
  #define MOVE_ID       97
  #define RENAME_ID     98
  #define MOUNT_ID      80
  #define UNMOUNT_ID    81
  #define EJECT_ID      82
  #define PROPERTIES_ID 99
  //////////////////////////////////////////////////////////
  if (fileList->getFilename(item)=="..") return;
	QString file=returnPath(item);		// full path
  QString localFile=fileList->getFilename(item);  // file name
  KURL u;
  u.setPath(file);
  KURL::List lst;
  lst.append(u);
  // create the menu
  QPopupMenu popup,openWith;
  // the OPEN option - generic "execute"
  popup.insertItem("Open",OPEN_ID);          // create the open option
	popup.changeItem(OPEN_ID,*item->pixmap(0), // and add pixmap
     i18n("Open"));
  popup.insertSeparator();
  // Open with
  // try to find-out which apps can open the file
//KURL url;
//url.setPath(files->vfs_getFile(item->text(0)));
//KFileItem fi(-1,-1,url);
//KTrader *trader = KdedInstance::self()->ktrader();
//KTrader::OfferList offers = trader->query(fi.mimetype());
//if (offers.count()>0) cerr << ">0";
  openWith.insertItem(i18n("Konqueror"),OPEN_KONQ_ID);
  openWith.insertItem("a terminal",OPEN_TERM_ID);
  openWith.changeItem(OPEN_TERM_ID,krLoader->loadIcon("konsole",KIcon::Small),i18n("Terminal"));
//  openWith.insertSeparator();
//  openWith.insertItem(i18n("Other..."),CHOOSE_ID);
  popup.insertItem(QPixmap(),&openWith,OPEN_WITH_ID);
  popup.changeItem(OPEN_WITH_ID,"Open in");
  popup.insertSeparator();
  // COPY
  popup.insertItem(i18n("Copy"),COPY_ID);
  if (files->isWritable) {
    // MOVE
    popup.insertItem(i18n("Move"),MOVE_ID);
    // RENAME
    popup.insertItem(i18n("Rename"),RENAME_ID);
    // DELETE
    popup.insertItem(i18n("Delete"),DELETE_ID);
  }
	// Calculate occupied space
  popup.insertSeparator();
  krCalculate->plug(&popup);
  // mount / unmount
  if (krMtMan.getStatus(returnPath(item))==MountMan::KMountMan::MOUNTED)
    popup.insertItem(i18n("Unmount"),UNMOUNT_ID);
  else if (krMtMan.getStatus(returnPath(item))==MountMan::KMountMan::NOT_MOUNTED)
    popup.insertItem(i18n("Mount"),MOUNT_ID);
  if (krMtMan.ejectable(files->vfs_getFile(fileList->getFilename(item))))
    popup.insertItem(i18n("Eject"), EJECT_ID);
  // PROPERTIES
  popup.insertSeparator();
  krProperties->plug(&popup);
  // run it, on the mouse location
  int i=QFontMetrics(popup.font()).height()*2;
  int result=popup.exec(QPoint(loc.x()+5,loc.y()+i));
  // check out the user's option
  switch (result) {
    case -1 : return;     // the user clicked outside of the menu
    case OPEN_ID :        // Open/Run
      slotChangeFromTree(item);
      break;
    case COPY_ID :
      func->copyFiles();
      break;
    case MOVE_ID :
      func->moveFiles();
      break;
    case RENAME_ID :
      func->rename();
      break;
    case DELETE_ID :
      func->deleteFiles();
      break;
    case EJECT_ID :
      MountMan::KMountMan::eject(files->vfs_getFile(fileList->getFilename(item)));
      break;
    case MOUNT_ID :
//      krMtMan.mount(files->vfs_getFile(item->text(0)));
      krMtMan.mount(returnPath(item));
      break;
    case UNMOUNT_ID :
//      krMtMan.unmount(files->vfs_getFile(item->text(0)));
      krMtMan.unmount(returnPath(item));
      break;
    case OPEN_KONQ_ID :   // open in konqueror
      kapp->startServiceByDesktopName("konqueror",file);
      break;
    case OPEN_TERM_ID :   // open in terminal
		  QString save = getcwd(0,0);
		  KProcess proc;
	    krConfig->setGroup("General");
		  QString term = krConfig->readEntry("Terminal",_Terminal);
	    proc <<  term;
	    chdir(file.local8Bit());
	    if(!proc.start(KProcess::DontCare))
	      KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+term+"\"");
	    chdir(save.local8Bit());
      break;
  }
}

// calculate item under the mouse, keep it and rebuild the old one
QListViewItem* TreePanel::itemOn(QPoint p) {
  // if there is a previous dealt-with item, rebuild it's pixmap
  if (currDragItem) currDragItem->setPixmap(0,currDragPix);
  // find the size of the listview's header
  int headerSize=height()-totals->height()-origin->height()-status->height()-fileList->visibleHeight();
  headerSize-=(fileList->horizontalScrollBar()->isVisible() ?
              fileList->horizontalScrollBar()->height() : 0);
//  QPoint evPos=mapToGlobal(p);
  p.setY(p.y()-headerSize);
  QListViewItem *i=fileList->itemAt(fileList->mapFromGlobal(p));
  if (!i) return 0;
  currDragItem=i;             // keep the current item's id and pixmap
  currDragPix=*i->pixmap(0);
  return i;
}

void TreePanel::dragEnterEvent ( QDragEnterEvent *) {
  currDragItem=0; // just making sure
}

void TreePanel::dragLeaveEvent ( QDragLeaveEvent *) {
  // if leaving panel, make sure we cleaned-up first - gui wise
  if (currDragItem) currDragItem->setPixmap(0,currDragPix);
  currDragItem=0;
}

void TreePanel::dragMoveEvent( QDragMoveEvent *ev ) {
  QStrList list;
  if (!QUriDrag::canDecode(ev) || !ev->provides("text/uri-list") /*||
      !files->isWritable || !files->supportCopyTo*/ ){
    ev->ignore(); // not for us to handle!
    return;
  } // if we got here, let's check the mouse location
  QPoint evPos=mapToGlobal(ev->pos());
  // calculate true width of fileList
  int hdelta=(fileList->horizontalScrollBar()->isVisible() ?
              fileList->horizontalScrollBar()->height() : 0);
  // create locations
  QPoint bottomRight(fileList->visibleWidth(),height()-totals->height()-hdelta);
  QPoint topLeft(0,bottomRight.y()-fileList->visibleHeight());
  topLeft=mapToGlobal(topLeft);
  bottomRight=mapToGlobal(bottomRight);
  if (evPos.y()<topLeft.y() || evPos.y()>bottomRight.y() ||
      evPos.x()<topLeft.x() || evPos.x()>bottomRight.x() ) {
     ev->ignore();  // if we're in a panel but out of the file list
     return;
  }
  QListViewItem *i=itemOn(evPos);
  // if we're pointing on an empty spot, leave
  if (!i) { ev->acceptAction(); return; }
  // if we're pointing on the "root decoration", leave
  if ( !( ev->pos().x() > fileList->header()->cellPos(
       fileList->header()->mapToActual( 0 ) ) + fileList->treeStepSize() * (
       i->depth() + ( fileList->rootIsDecorated() ? 1 : 0) ) +
       fileList->itemMargin() || ev->pos().x() < fileList->header()->cellPos(
       fileList->header()->mapToActual( 0 ))) )  {
          ev->ignore();
          return;
  }

  // change the folder's pixmap to an "open-folder"
  i->setPixmap(0,FL_LOADICON("folder_open"));
  ev->acceptAction();
}

void TreePanel::dropEvent( QDropEvent *ev ) {
  // if copyToPanel is true, then we call a simple vfs_addfiles
  bool dragFromOtherPanel=false;

  QPoint evPos=mapToGlobal(ev->pos());
  QListViewItem *i=itemOn(evPos);

  if (ev->source()==this) return ;// no dragging onto ourselves
  if (ev->source()==otherPanel) dragFromOtherPanel=true;

  if (!i) {
    ev->ignore();
    return;
  }
  // keep the folder_open icon until we're finished, do it only
  // if the folder is writeable, to avoid flicker
  currDragPix=*i->pixmap(0);
  i->setPixmap(0,FL_LOADICON("folder_open"));
  QString dest=returnPath(i); // used later for vfs building

  //////////////////////////////////////////////////////////////////////////////
  // decode the data
  KURL::List URLs;
  QStrList list;
  if (!QUriDrag::decode(ev,list)) {
    ev->ignore(); // not for us to handle!
    i->setPixmap(0,currDragPix); // clean-up
    return;
  } // now, the list of URLs is stored in 'list', we'll create a KURL::List
  QStrListIterator it(list);
  while (it) {
    URLs.append(*it);
    ++it;
  }

  // now, create the vfs and prepare the job
  fileList->setCurrentItem(i);
  slotSelectionChanged(i);
  KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy;
  bool isWritable=files->isWritable;

  // the KURL::List is finished, let's go
  // --> display the COPY/MOVE/LINK menu
  if (dragFromOtherPanel && ev->action()==QDropEvent::Move)
    mode = KIO::CopyJob::Move;
  else {
    QPopupMenu popup;
    if (isWritable) popup.insertItem(i18n("Copy Here"),1);
    if (otherPanel->files->supportMoveFrom && isWritable)
      popup.insertItem(i18n("Move Here"),2);
    if (otherPanel->files->vfs_getType()=="normal" && isWritable)
      popup.insertItem(i18n("Link Here"),3);
    popup.insertItem(i18n("Cancel"),4);
    int result=popup.exec(evPos);
    switch (result) {
      case 1 :
        mode = KIO::CopyJob::Copy;
        break;
      case 2 :
        mode = KIO::CopyJob::Move;
        break;
      case 3 :
        mode = KIO::CopyJob::Link;
        break;
      case -1 :  // user pressed outside the menu
      case 4:
        i->setPixmap(0,currDragPix); // clean-up
        return; // cancel was pressed;
    }
  } // big bunch o' stuff is finished, lets go on...

  i->setPixmap(0,currDragPix); // clean-up
  QWidget *notify = (!ev->source() ? 0 : ev->source());
  files->vfs_addFiles(&URLs,mode,notify,"");
}

QString TreePanel::getCurrentName() {
  QString name;
  QListViewItem *it = fileList->currentItem();
  if ( !it ) return QString::null;  // safety
  if ( (name = fileList->getFilename(it)) != "..")
    return name;
  else return QString::null;
}

void TreePanel::startDragging(int mode) {
	QStringList names;
	getSelectedNames(&names);
	
	bool draggingSingle;
	// if the list is empty, don't drag nothing
	switch (names.count()) {
	  case 0 : emit finishedDragging(); // if dragged an empty space, don't do it!
             return;
    case 1 : draggingSingle=true;     // if drag a single file, change icon
             break;
    default: emit finishedDragging();
             return;
  }
	
	KURL::List* urls = files->vfs_getFiles(&names);
	if( urls->isEmpty() ){ // avoid draging empty urls
	  emit finishedDragging();
	  return;
	}
	// KURL::List -> QStrList
  QStrList URIs;
  for(KURL::List::Iterator url = urls->begin(); url != urls->end() ; ++url)
    URIs.append( (*url).url().local8Bit() );
	delete urls; // free memory
          	
  QUriDrag *d=new QUriDrag(URIs,this);
  if (draggingSingle) d->setPixmap(*fileList->currentItem()->pixmap(0),QPoint(-7,0));
                 else d->setPixmap(FL_LOADICON("queue"),QPoint(-7,0));
  emit finishedDragging();
  switch (mode) {
    case 0 : d->dragCopy(); break; // copy will be used for copy and for link
    case 1 : d->dragMove(); break;
  }
}
