/***************************************************************************
                                listpanel.cpp
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
#include <qbitmap.h>
#include <qheader.h>
#include <qwhatsthis.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qregexp.h>
// KDE includes
#include <kmessagebox.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include <ktrader.h>
#include <krun.h>
#include <kopenwith.h>
#include <kuserprofile.h>
#include <kiconloader.h>
#include <kshred.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kglobalsettings.h>
#include <kdeversion.h>
#include <qimage.h>
#include <qtabbar.h>
#include <kdebug.h>
// Krusader includes
#include "../krusader.h"
#include "../krslots.h"
#include "../kicons.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/krpermhandler.h"
#include "listpanel.h"
#include "krlistitem.h"
#include "../defaults.h"
#include "../resources.h"
#include "kfilelist.h"
#include "panelfunc.h"
#include "../MountMan/kmountman.h"
#include "../Dialogs/krdialogs.h"
#include "../BookMan/bookman.h"
#include "../Dialogs/krspwidgets.h"
typedef QValueList<KServiceOffer> OfferList;

/////////////////////////////////////////////////////
// 					The list panel constructor             //
/////////////////////////////////////////////////////
ListPanel::ListPanel(QWidget *parent, const bool mirrored, const char *name ) :
					 QWidget(parent,name), colorMask(255), compareMode(false),
           currDragItem(0), statsAgent(0) {

  setNameToMakeCurrent(QString::null);
  func = new ListPanelFunc(this);
  setAcceptDrops(true);
  setMouseTracking(false);
	layout=new QGridLayout(this,3,2);

  status = new KSqueezedTextLabel(this);
  krConfig->setGroup("Look&Feel");
  status->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  status->setBackgroundMode(PaletteBackground);
  status->setFrameStyle( QFrame::Box | QFrame::Raised);
  status->setLineWidth(1);		// a nice 3D touch :-)
  status->setText("");        // needed for initialization code!
  int sheight = QFontMetrics(status->font()).height()+4;
  status->setMaximumHeight(sheight);
  QWhatsThis::add(status,i18n("The status bar displays information about the FILESYSTEM which hold your current directory: Total size, free space, type of filesystem etc."));

   // ... create the bookmark list
  bookmarkList=new QToolButton(this);
  bookmarkList->setFixedSize(sheight, sheight);
  QImage im = krLoader->loadIcon("kr_bookmark",KIcon::Toolbar).convertToImage();
  bookmarkList->setPixmap(im.scale(sheight,sheight));
  bookmarkList->setTextLabel(i18n("Open your bookmarks"),true);
  bookmarkList->setPopupDelay(10); // 0.01 seconds press
  connect(bookmarkList,SIGNAL(pressed()),this,SLOT(slotFocusOnMe()));
  connect(bookmarkList,SIGNAL(pressed()),this,SLOT(slotRefreshBookmarks()));
    // create the pop-up menu for the bookmarks,
    // this means the panel needs to be
  bookmarks=new QPopupMenu(this);
  krConfig->setGroup("Look&Feel");
  bookmarks->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  bookmarks->setBackgroundMode(PaletteLight);
  slotRefreshBookmarks();
    // and connect it to the button
  bookmarkList->setPopup(bookmarks);
  QWhatsThis::add(bookmarkList,i18n("Pressing this button will pop a menu with your bookmarks. Select a bookmark to go where it's pointing. Simple ehh?"));
  connect(bookmarks,SIGNAL(activated(int)),this,
          SLOT(slotBookmarkChosen(int)));

  totals = new KSqueezedTextLabel(this);
  totals->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  totals->setFrameStyle( QFrame::Box | QFrame::Raised);
  totals->setBackgroundMode(PaletteBackground);
  totals->setLineWidth(1);		// a nice 3D touch :-)
  totals->setMaximumHeight(sheight);
  QWhatsThis::add(totals,i18n("The totals bar shows how much files exist, how many did you select and the bytes math"));

  origin = new QPushButton("", this);
  origin->setMaximumHeight(QFontMetrics(origin->font()).height()+4);
  QWhatsThis::add(origin,i18n("This shows the full path to your current directory"));
  connect(origin,SIGNAL(clicked()),this,SLOT(slotFocusOnMe()));

  fileList = new KFileList(this);
  krConfig->setGroup("Look&Feel");
  fileList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
	fileList->setAllColumnsShowFocus(true);
	fileList->setMultiSelection(true);
	fileList->setSelectionMode(QListView::Extended);
	fileList->setShowSortIndicator(true);
	fileList->setVScrollBarMode(QScrollView::AlwaysOn);
	fileList->setHScrollBarMode(QScrollView::Auto);
  //fileList->header()->setMaximumHeight(QFontMetrics(fileList->header()->font()).height()+2);
	int i=QFontMetrics(fileList->font()).width("W");
	int j=QFontMetrics(fileList->font()).width("0");
  if (i<j) i=j;
  // get the save sizes of the columns
  QString side = (mirrored ? "Right " : "Left ");
  krConfig->setGroup("Private");
	fileList->addColumn(i18n("Name"),krConfig->readNumEntry(side+"Name Size",	i*17));
	fileList->addColumn(i18n("Size"),krConfig->readNumEntry(side+"Size Size",
	                    QFontMetrics(fileList->font()).width("00,000,000")+5));
	fileList->setColumnAlignment(1,Qt::AlignRight);
	fileList->addColumn(i18n("Date"),krConfig->readNumEntry(side+"Date Size",
	                    QFontMetrics(fileList->font()).width("09/09/09  09:09")+5));
	fileList->setColumnAlignment(2,Qt::AlignHCenter);
	fileList->addColumn(i18n("r"),17);
	fileList->addColumn(i18n("w"),17);
	fileList->addColumn(i18n("x"),17);
  fileList->setColumnAlignment(3,Qt::AlignCenter);
  fileList->setColumnAlignment(4,Qt::AlignCenter);
  fileList->setColumnAlignment(5,Qt::AlignCenter);
  QWhatsThis::add(fileList,i18n("This is your regular file list. Except, the R,W and X column represent the permission whicb applies to you, i.e: if you see an 'X' in the W column, it means that YOU can't write to this file. No need to read a 10-length permission any more !!"));
    // when fileList wants us to drag ...
  connect(fileList, SIGNAL(letsDrag(int)), this, SLOT(startDragging(int)));	
    // when the drag is finished
  connect(this,SIGNAL(finishedDragging()), fileList, SLOT(finishedDragging()));
  	// double click and pressing return executes
  connect(fileList, SIGNAL(doubleClicked(QListViewItem*)), func,
          SLOT(execute(QListViewItem*)));
  connect(fileList, SIGNAL(returnPressed(QListViewItem *)), func,
          SLOT(execute(QListViewItem*)));
		// even a "drag-swush" through the list changes focus
	connect(fileList, SIGNAL(mouseButtonPressed(int,QListViewItem *,const QPoint &,int)),
					this,SLOT(slotFocusOnMe()));
		// mouse over item event
	connect(fileList, SIGNAL(onItem(QListViewItem*)),
					this,SLOT(slotChangeStatus(QListViewItem*)));
		// a change in the selection needs to update totals
	connect(fileList, SIGNAL(selectionChanged()), this,	SLOT(slotUpdateTotals()));
    // right-click menu
  connect(fileList, SIGNAL(contextMenu(KListView *, QListViewItem*,const QPoint&)), this,
          SLOT(popRightClickMenu(KListView *,QListViewItem*,const QPoint&)));

		// conncet krusder status bar with mouse over event...
	connect(this,SIGNAL(signalStatus(QString)),
					krApp, SLOT(statusBarUpdate(QString)));
	  // when selection change..
	connect(fileList, SIGNAL(currentChanged(QListViewItem *)),
	        this,SLOT(slotSelectionChanged(QListViewItem *)));

		// make sure that a focus/path change reflects in the command line and activePanel
	connect(this,SIGNAL(cmdLineUpdate(QString)),this->parent()->parent()->parent(),
					SLOT(slotCurrentChanged(QString)) );
	connect(this,SIGNAL(activePanelChanged(ListPanel *)),this->parent()->parent()->parent(),
					SLOT(slotSetActivePanel(ListPanel *)) );

    // when the BookMan asks to refresh bookmarks...
  connect(krBookMan, SIGNAL(refreshBookmarks()), this, SLOT(slotRefreshBookmarks()));

  // finish the layout
	layout->addMultiCellWidget(origin,0,0,0,1);
	if (mirrored) {
		layout->addWidget(status,1,0);
		layout->addWidget(bookmarkList,1,1);

	} else {
		layout->addWidget(status,1,1);
		layout->addWidget(bookmarkList,1,0);
	}
	layout->addMultiCellWidget(fileList,2,2,0,1);
	layout->addMultiCellWidget(totals,3,3,0,1);

	filter = ALL;
}

void ListPanel::select(bool select, bool all) {
  if (all)
    if (select) fileList->select(QString("*"));
    else fileList->unselect(QString("*"));
  else {
    QString answer=KRSpWidgets::getMask((select ? i18n(" Select Files ") : i18n(" Unselect Files ")));
  	// if the user canceled - quit
  	if (answer == QString::null) return;
  	if (select) fileList->select(answer);
  	else fileList->unselect(answer);
  }
}

void ListPanel::invertSelection(){
	fileList->invertSelection();
}

void ListPanel::slotFocusOnMe(){		 // give this VFS the focus (the path bar)
  krConfig->setGroup("Look&Feel");
	otherPanel->focused=false;
  QPalette q( otherPanel->status->palette() );
  q.setColor( QColorGroup::Foreground,KGlobalSettings::textColor());
  q.setColor( QColorGroup::Background,KGlobalSettings::baseColor());

  otherPanel->origin->setPalette(q);
  otherPanel->status->setPalette(q);
  otherPanel->totals->setPalette(q);
	otherPanel->bookmarkList->setBackgroundMode(PaletteBackground);
  otherPanel->bookmarkList->setPalette(q);

  focused=true;
  QPalette sp( status->palette() );
  sp.setColor( QColorGroup::Foreground,KGlobalSettings::highlightedTextColor() );
  sp.setColor( QColorGroup::Background,KGlobalSettings::highlightColor() );
  status->setPalette(sp);
  origin->setPalette(sp);
  totals->setPalette(sp);
  bookmarkList->setBackgroundMode(PaletteBackground);
  bookmarkList->setPalette(sp);

	fileList->setFocus();     // get the keyboard attention to the file list
	emit cmdLineUpdate(realPath);
	emit activePanelChanged(this);

  QPalette bp( bookmarks->palette() );
  bp.setColor( QColorGroup::Foreground,KGlobalSettings::activeTextColor() );
  bp.setColor( QColorGroup::Background,KGlobalSettings::activeTitleColor() );
  bookmarks->setPalette(bp);
	func->refreshActions();
}

// overload this to control the NAME column resize
void ListPanel::resizeEvent ( QResizeEvent *e ) {
  QWidget::resizeEvent(e);
	int delta=e->size().width() - fileList->columnWidth(0) - fileList->columnWidth(1)
															- fileList->columnWidth(2) - fileList->columnWidth(3)*5+11;
	// do not down-size below base-size of 8 letters to the name column
	int w=QFontMetrics(fileList->font()).width("W");
	if (fileList->columnWidth(0)<(w*8) && delta<0) return;
	fileList->setColumnWidth(0,fileList->columnWidth(0)+delta);
	fileList->triggerUpdate();
}

// this is used to start the panel, AFTER setOther() has been used
//////////////////////////////////////////////////////////////////
void ListPanel::start(bool left) {
	krConfig->setGroup("Startup");
	
	// set the startup path
	if(left){
	  if(krConfig->readEntry("Left Panel Origin",_LeftPanelOrigin) == i18n("homepage"))
	    virtualPath = krConfig->readEntry("Left Panel Homepage",_LeftHomepage);
	  else if(krConfig->readEntry("Left Panel Origin")==i18n("the last place it was"))
	    virtualPath = krConfig->readEntry("lastHomeLeft","/");
	  else virtualPath = getcwd(0,0);//get_current_dir_name();
	}
	else { // right
	  if(krConfig->readEntry("Right Panel Origin",_RightPanelOrigin) == i18n("homepage"))
	    virtualPath = krConfig->readEntry("Right Panel Homepage",_RightHomepage);
	  else if(krConfig->readEntry("Right Panel Origin")==i18n("the last place it was"))
	    virtualPath = krConfig->readEntry("lastHomeRight","/");
	  else virtualPath = getcwd(0,0);
	}
	
	realPath = virtualPath;
  func->openUrl(virtualPath);
}

void ListPanel::slotStartUpdate(){
	// if the vfs couldn't make it  - go back
	if( func->files()->vfs_error() ){
	  func->inRefresh = false;
	  func->dirUp();
	  return;
	}
	
	while( func->inRefresh ) ; // wait until the last refresh finish
	func->inRefresh = true;  // make sure the next refresh wait for this one
	
	fileList->clear();

	// set the virtual path
	virtualPath = func->files()->vfs_getOrigin();
	if(func->files()->vfs_getType() == "normal")
	  realPath = virtualPath;
	// set the origin path
  QString shortPath = virtualPath ,s1,s2;
	shortPath = shortPath.replace( QRegExp("\\\\"),"#");
	if ( QFontMetrics(origin->font()).width(shortPath)+40 > origin->width() ){
		s1 = shortPath.left((shortPath.length()/2));
		s2 = shortPath.right((shortPath.length()/2));
		while( QFontMetrics(origin->font()).width(s1+"..."+s2)+40 > origin->width() ){
			s1.truncate(s1.length()-1);
			s2 = s2.right(s2.length()-1);
  	}
  	shortPath = s1+"..."+s2;
  }
	origin->setText(shortPath);

	emit cmdLineUpdate(realPath);	// update the command line
}

void ListPanel::slotEndUpdate(){	
	slotGetStats(virtualPath);
	slotUpdate();
	if( compareMode ) {
	  otherPanel->fileList->clear();
	  ((ListPanel*)otherPanel)->slotUpdate();
	}
	// return cursor to normal arrow
	krApp->setCursor(KCursor::arrowCursor());
}

void ListPanel::slotUpdate(){
	QListViewItem *item = fileList->firstChild();
	QListViewItem *currentItem = item;
	QString size;
	QString name;
	vfs* files = func->files();
	
	// if we are not at the root add the ".." entery
	QString origin = func->files()->vfs_getOrigin();
	if( origin.right(1)!="/" && !((files->vfs_getType()=="ftp")&&
      origin.find('/',origin.find(":/")+3)==-1) ) {
		QListViewItem * item=new KRListItem(fileList,"..","<DIR>");
		item->setPixmap(0,FL_LOADICON("up"));
	  item->setSelectable(false);
	}

	for( vfile* vf=files->vfs_getFirstFile(); vf != 0 ; vf=files->vfs_getNextFile() ){
	
		size =  KRpermHandler::parseSize(vf->vfile_getSize());
		name =  vf->vfile_getName();
		bool isDir = vf->vfile_isDir();
		KRListItem::cmpColor color = KRListItem::none;
		if( compareMode ){
		  vfile* ovf = otherPanel->func->files()->vfs_search(vf->vfile_getName());
		  if (ovf == 0 ) color = KRListItem::exclusive;  // this file doesn't exist on the other panel
		  else{ // if we found such a file
        QString date1 = KRpermHandler::date2qstring(vf->vfile_getDateTime());
        QString date2 = KRpermHandler::date2qstring(ovf->vfile_getDateTime());
        if (date1 > date2) color = KRListItem::newer; // this file is newer than the other
        else
        if (date1 < date2) color = KRListItem::older; // this file is older than the other
        else
        if (date1 == date2) color = KRListItem::identical; // the files are the same
		  }
		}
		
		if(!isDir){
			switch(filter){
    		case ALL : 		break;
				case EXEC:  	if (!vf->vfile_isExecutable()) continue;
											break;
				case CUSTOM:  if (!QDir::match(filterMask,name)) continue;
											break;
			}
		  if ( compareMode && !(color & colorMask) ) continue;
		}

		item = new KRListItem(fileList,item,name,
				isDir ? QString("<DIR>") : size,
		    vf->vfile_getDateTime(),"","","",color);

  	switch (vf->vfile_isReadable()){
      case ALLOWED_PERM: item->setText(3,QString("r")); break;
      case UNKNOWN_PERM: item->setText(3,QString("?")); break;
      case NO_PERM:      item->setText(3,QString("-")); break;
    }
  	switch (vf->vfile_isWriteable()){
      case ALLOWED_PERM: item->setText(4,QString("w")); break;
      case UNKNOWN_PERM: item->setText(4,QString("?")); break;
      case NO_PERM:      item->setText(4,QString("-")); break;
    }
  	switch (vf->vfile_isExecutable()){
      case ALLOWED_PERM: item->setText(5,QString("x")); break;
      case UNKNOWN_PERM: item->setText(5,QString("?")); break;
      case NO_PERM:      item->setText(5,QString("-")); break;
    }
				
		item->setPixmap(0,getIcon(vf, color));

		// if the item should be current - make it so
		if(item->text(0) == nameToMakeCurrent) currentItem = item;
	}
	
//	fileList->setSorting(0);
	slotUpdateTotals();
 	// set the currentItem -visible
  fileList->setCurrentItem(currentItem);
	fileList->ensureItemVisible(currentItem);
 	
 	func->inRefresh = false;
}

// walk through the list and update the no. of selected files and their size
////////////////////////////////////////////////////////////////////////////
void ListPanel::slotUpdateTotals() {
  int totalNo = 0, selectedNo = 0;
	long long size=0 , totalSize = 0, selectedSize = 0;
	QString totalsLine;
		
	for (QListViewItem *iterator=fileList->firstChild(); iterator != 0; iterator=iterator->itemBelow()){
		// if the current item is not a directory - calculate it's size
		if ( (iterator->text(1))!=QString("<DIR>") )
			  size=(iterator->text(1).replace(QRegExp(","),"").toLong());
		else
				size = 0;
    // update the totals numbers
		if (iterator->text(0)!=QString("..")) ++totalNo;
		totalSize+=size;
		// if the current item is selected - update the selected numbers
		if (iterator->isSelected() && iterator->text(0)!=QString("..")) {
			++selectedNo;
			selectedSize+=size;
		}
	}

	totalsLine = QString("%1 "+i18n("out of")+" %2 "+i18n("selected")+", %3 "+i18n("out of")+
					" %4").arg(selectedNo).arg(totalNo).arg(KIO::convertSize(selectedSize)).
					arg(KIO::convertSize(totalSize));
	totals->setText(totalsLine);
}

QPixmap ListPanel::getIcon(vfile* vf, KRListItem::cmpColor color){
	krConfig->setGroup("Advanced");
	//////////////////////////////
	QPixmap icon;
	QString mime = vf->vfile_getMime();
	QPixmapCache::setCacheLimit( krConfig->readNumEntry("Icon Cache Size",_IconCacheSize) );
	
	// first try the cache
	if (!QPixmapCache::find(mime,icon)){
	  // get the icon.
	  if ( mime == "Broken Link !"  )
	    icon = FL_LOADICON("file_broken");
	  else {
	    icon = FL_LOADICON(KMimeType::mimeType(mime)->icon(QString::null,true));
	  }
	  // insert it into the cache
    QPixmapCache::insert(mime,icon);
	}
	// if it's a symlink - add an arrow overlay
	if(vf->vfile_isSymLink()){
		QPixmap link(link_xpm);
		bitBlt (&icon,0,icon.height()-11,&link,0,21,10,11,CopyROP,false);
		icon.setMask( icon.createHeuristicMask(false) );
  }

  // color-coding for compare mode
  if (color != KRListItem::none) {
    QPixmap block;
    switch (color) {
      case KRListItem::exclusive : block = QPixmap(blue_xpm);
                                   break;
      case KRListItem::newer     : block = QPixmap(green_xpm);
                                   break;
      case KRListItem::older     : block = QPixmap(red_xpm);
                                   break;
      case KRListItem::identical : block = QPixmap(yellow_xpm);
                                   break;
			case KRListItem::none      : break; /* keep the compiler happy */
    }
		
		bitBlt (&icon,icon.width()-11,0,&block,0,21,10,11,CopyROP,false);
		icon.setMask( icon.createHeuristicMask(false) );
  }
	
	return icon;	
}

void ListPanel::slotChangeStatus(QListViewItem *i){
	if (fileList->getFilename(i)=="..") emit signalStatus(i18n("Climb up the directory tree"));
	else{
 	   	vfile* vf = func->files()->vfs_search(fileList->getFilename(i));
			if(vf == 0) return;
			
			QString text = vf->vfile_getName();
			
			QString comment = KMimeType::mimeType(vf->vfile_getMime() )->comment(text, false );
			
  		QString myLinkDest = vf->vfile_getSymDest();
  		long long mySize = vf->vfile_getSize();

 			QString text2 = text.copy();
  		mode_t m_fileMode = vf->vfile_getMode();

  		if (vf->vfile_isSymLink() ){
      	QString tmp;
      	
				if ( comment.isEmpty() )	tmp = i18n ( "Symbolic Link" ) ;
				else if( vf->vfile_getMime() == "Broken Link !" ) tmp = i18n("(broken link !)");
  		  else  tmp = i18n("%1 (Link)").arg(comment);
      	
				text += "->";
      	text += myLinkDest;
      	text += "  ";
      	text += tmp;
  		} else if ( S_ISREG( m_fileMode ) ){
      	text = QString("%1 (%2)").arg(text2).arg( KIO::convertSize( mySize ) );
      	text += "  ";
      	text += comment;
  		} else if ( S_ISDIR ( m_fileMode ) ){
      	text += "/  ";
     		text += comment;
    	} else {
      	text += "  ";
      	text += comment;
    	}
			
		emit signalStatus(text);
	}
}

//  refresh the panel when a bookmark was chosen
void ListPanel::slotBookmarkChosen(int id) {
 	setNameToMakeCurrent(QString::null);
	QString origin = krBookMan->getUrlById(id);
 	func->openUrl( origin );
}

void ListPanel::slotRefreshBookmarks() {
  krBookMan->fillPopupMenu(bookmarks);
}

void ListPanel::slotGetStats(QString path) {
  QString templabel,label;

	if ( path.contains("\\") ){
    status->setText(i18n("No space information inside archives"));
    return;
	}
	
	if (path.contains(":/")) {
    status->setText(i18n("No space information on non-local filesystems"));
    return;
  }

  // 1st, get information
  // mountMan tries to get information on the mountpoint regardless if
  // it is operational or not - rafi's fix
  status->setText(i18n("Mt.Man: working ..."));
  statsAgent = new MountMan::statsCollector(path,this);
}

void ListPanel::gotStats(QString data) {
  status->setText(data);
  if (statsAgent) delete statsAgent;
}

// calculate item under the mouse, keep it and rebuild the old one
QListViewItem* ListPanel::itemOn(QPoint p) {
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

void ListPanel::dragEnterEvent ( QDragEnterEvent *) {
  currDragItem=0; // just making sure
}

void ListPanel::dragLeaveEvent ( QDragLeaveEvent *) {
  // if leaving panel, make sure we cleaned-up first.
  if (currDragItem) currDragItem->setPixmap(0,currDragPix);
  currDragItem=0;
}

void ListPanel::dragMoveEvent( QDragMoveEvent *ev ) {
  scroll = false;
  QStrList list;
  if (!QUriDrag::canDecode(ev) || !ev->provides("text/uri-list") ||
      !func->files()->vfs_isWritable() ){
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
  // if we below the file list - scroll down
  if ( evPos.y() > (bottomRight.y()-10) && evPos.y() < bottomRight.y() ){
    fileList->startScrolling(1);
    ev->acceptAction();
    return;
  }
  // if we above the file list - scroll up
  if ( evPos.y() < (topLeft.y()+10) && evPos.y() > topLeft.y()){
    fileList->startScrolling(-1);
    ev->acceptAction();
    return;
  }

  // else implied - stop scrolling
  fileList->startScrolling(0);

  if ( evPos.x()<topLeft.x() || evPos.x()>bottomRight.x() ||
       evPos.y()<topLeft.y() || evPos.y()>bottomRight.y() ) {
     ev->ignore();  // if we're in a panel but out of the file list
     return;
  }
  QListViewItem *i=itemOn(evPos);
  // if we're pointing on an empty spot, leave
  if (!i) { ev->acceptAction(); return; }
  // otherwise, check if we're dragging on a directory
  if (i->text(1)=="<DIR>" && i->text(0)!="..") {
    i->setPixmap(0,FL_LOADICON("folder_open"));
  }
  ev->acceptAction();
}

void ListPanel::dropEvent( QDropEvent *ev ) {
  // if copyToPanel is true, then we call a simple vfs_addfiles
  bool copyToDirInPanel=false;
//  bool copyToZip=false;
  bool dragFromOtherPanel=false;
  bool dragFromThisPanel=false;
  bool isWritable=func->files()->vfs_isWritable();

  vfs* tempFiles = func->files();
  vfile *file;
  QPoint evPos=mapToGlobal(ev->pos());
  QListViewItem *i=itemOn(evPos);

  if (ev->source()==otherPanel) dragFromOtherPanel=true;
  if (ev->source()==this)       dragFromThisPanel=true;

  if (i){
     file=func->files()->vfs_search(i->text(0));

     if (!file) { // trying to drop on the ".."
       if(virtualPath.right(1)=="\\") // root of archive..
          isWritable = false;
       else copyToDirInPanel=true;
     }else{
      if (file->vfile_isDir()){
        copyToDirInPanel=true;
        isWritable = file->vfile_isWriteable();
        if (isWritable) {
          // keep the folder_open icon until we're finished, do it only
          // if the folder is writeable, to avoid flicker
          currDragPix=*i->pixmap(0);
          i->setPixmap(0,FL_LOADICON("folder_open"));
        }
      }else
        if (ev->source()==this) return ;// no dragging onto ourselves
    }
  } else    // if dragged from this panel onto an empty spot in the panel...
  if (dragFromThisPanel) {  // leave!
    ev->ignore();
    return;
  }

  if(!isWritable){
    ev->ignore();
    return;
  }
  //////////////////////////////////////////////////////////////////////////////
  // decode the data
  KURL::List URLs;
  QStrList list;
  if (!QUriDrag::decode(ev,list)) {
    ev->ignore(); // not for us to handle!
    if (copyToDirInPanel) i->setPixmap(0,currDragPix); // clean-up
    return;
  } // now, the list of URLs is stored in 'list', we'll create a KURL::List
  QStrListIterator it(list);
  while (it) {
    URLs.append(*it);
    ++it;
  }

  KIO::CopyJob::CopyMode mode = KIO::CopyJob::Copy;

  // the KURL::List is finished, let's go
  // --> display the COPY/MOVE/LINK menu
  if (dragFromOtherPanel && ev->action()==QDropEvent::Move)
    mode = KIO::CopyJob::Move;
  else {
    QPopupMenu popup;
    popup.insertItem(i18n("Copy Here"),1);
    if (func->files()->vfs_isWritable()) popup.insertItem(i18n("Move Here"),2);
    if (func->files()->vfs_getType()=="normal" &&
		    otherPanel->func->files()->vfs_getType()=="normal")
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
        if (copyToDirInPanel) i->setPixmap(0,currDragPix); // clean-up
        return; // cancel was pressed;
    }
  } // big bunch o' stuff is finished, lets go on...

  QString dir = "";
  if (copyToDirInPanel) {
    dir = i->text(0);
    i->setPixmap(0,currDragPix); // clean-up
  }
  QWidget *notify = (!ev->source() ? 0 : ev->source());
  tempFiles->vfs_addFiles(&URLs,mode,notify,dir);
}

void ListPanel::startDragging(int mode) {
	QStringList names;
	getSelectedNames(&names);
	
	bool draggingSingle;
	// if the list is empty, don't drag nothing
	switch (names.count()) {
	  case 0 : emit finishedDragging(); // if dragged an empty space, don't do it!
             return;
    case 1 : draggingSingle=true;     // if drag a single file, change icon
             break;
    default: draggingSingle=false;
  }
	
	KURL::List* urls = func->files()->vfs_getFiles(&names);
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

// pops a right-click menu for items
void ListPanel::popRightClickMenu(KListView *, QListViewItem* item,const QPoint &loc) {
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
  #define PROPERTIES_ID 99
  #define MOUNT_ID      100
  #define UNMOUNT_ID    101
  #define SHRED_ID      102
  #define NEW_LINK      103
  #define NEW_SYMLINK   104
  #define REDIRECT_LINK 105
  #define SEND_BY_EMAIL 106
  #define LINK_HANDLING 107
  #define EJECT_ID      108
  // those will sometimes appear
  #define SERVICE_LIST_ID  200
  //////////////////////////////////////////////////////////
  bool multipleSelections=false;
  QListViewItem *iterator;
  // a quick hack to check if we've got more that one file selected
  iterator=fileList->firstChild();
  int i=0;
  while (iterator) {
    if (iterator->isSelected()) ++i;
    iterator=iterator->itemBelow();
    if (i>1) {
      multipleSelections=true;
      break;
    }
  }

  if (fileList->getFilename(item)=="..") return;
  vfile *vf = func->files()->vfs_search(fileList->getFilename(item));
  if(vf==0) return;
  // create the menu
  QPopupMenu popup,openWith,linkPopup;
  // the OPEN option - open preferd service
  popup.insertItem("Open/Run",OPEN_ID);      // create the open option
  if (!multipleSelections) { // meaningful only if one file is selected
    popup.changeItem(OPEN_ID,*item->pixmap(0), // and add pixmap
       i18n((vf->vfile_isExecutable())&&(!vf->vfile_isDir()) ? "Run" : "Open"));
    popup.insertSeparator();
  }
  // Open with
  // try to find-out which apps can open the file
  OfferList offers =  KServiceTypeProfile::offers(vf->vfile_getMime());
  // this too, is meaningful only if one file is selected
  if (!multipleSelections) {
    for(unsigned int i = 0; i<offers.count(); ++i){
      KService::Ptr service = offers[i].service();
      if( service->isValid() && service->type()=="Application" ) {
        openWith.insertItem( service->name(),SERVICE_LIST_ID+i );
        openWith.changeItem(SERVICE_LIST_ID+i,service->pixmap(KIcon::Small),service->name());
      }
    }
    openWith.insertSeparator();
    if(vf->vfile_isDir())
      openWith.insertItem(krLoader->loadIcon("konsole",KIcon::Small),i18n("Terminal"),OPEN_TERM_ID);
    openWith.insertItem(i18n("Other..."),CHOOSE_ID);
    popup.insertItem(QPixmap(),&openWith,OPEN_WITH_ID);
    popup.changeItem(OPEN_WITH_ID,"Open with");
    popup.insertSeparator();
  }
  // COPY
  popup.insertItem(i18n("Copy"),COPY_ID);
  if (func->files()->vfs_isWritable()) {
  // MOVE
  popup.insertItem(i18n("Move"),MOVE_ID);
  // RENAME - only one file
  if (!multipleSelections)
      popup.insertItem(i18n("Rename"),RENAME_ID);
  // DELETE
  popup.insertItem(i18n("Delete"),DELETE_ID);
  // SHRED - only one file
  if (func->files()->vfs_getType()=="normal" &&
      !vf->vfile_isDir() && !multipleSelections)
  	popup.insertItem(i18n("Shred"),SHRED_ID);
  }
	// create new shortcut or redirect links - not on ftp:
	if ( func->files()->vfs_getType() != "ftp"){
		popup.insertSeparator();
		linkPopup.insertItem(i18n("new symlink"),NEW_SYMLINK);
    linkPopup.insertItem(i18n("new hardlink"),NEW_LINK);
		if( vf->vfile_isSymLink() )
			linkPopup.insertItem(i18n("redirect link"),REDIRECT_LINK);
    popup.insertItem(QPixmap(),&linkPopup,LINK_HANDLING);
    popup.changeItem(LINK_HANDLING,"Link handling");


  }
	popup.insertSeparator();
  if (func->files()->vfs_getType()!="ftp" && (vf->vfile_isDir() || multipleSelections))
    krCalculate->plug(&popup);
  if (func->files()->vfs_getType()=="normal" && vf->vfile_isDir() && !multipleSelections) {
    if (krMtMan.getStatus(func->files()->vfs_getFile(fileList->getFilename(item)))==MountMan::KMountMan::MOUNTED)
        popup.insertItem(i18n("Unmount"),UNMOUNT_ID);
    else if (krMtMan.getStatus(func->files()->vfs_getFile(fileList->getFilename(item)))==MountMan::KMountMan::NOT_MOUNTED)
        popup.insertItem(i18n("Mount"),MOUNT_ID);
    if (krMtMan.ejectable(func->files()->vfs_getFile(fileList->getFilename(item))))
      popup.insertItem(i18n("Eject"), EJECT_ID);
  }

  // send by mail (only for KDE version >= 2.2.0)
  if (Krusader::supportedTools().contains("MAIL") && !vf->vfile_isDir()) {
    popup.insertItem(i18n("Send by email"), SEND_BY_EMAIL);
  }
  // PROPERTIES
  popup.insertSeparator();
  krProperties->plug(&popup);
  // run it, on the mouse location
  int result=popup.exec(loc);
  // check out the user's option
  KURL u;
  KURL::List lst;

  switch (result) {
    case -1 : return;     // the user clicked outside of the menu
    case OPEN_ID :        // Open/Run
      //new KFileOpenWithHandler();
      iterator=fileList->firstChild();
      while (iterator) {
        if (iterator->isSelected()) {
          QString fname = fileList->getFilename(iterator);
    	    u.setPath(func->files()->vfs_getFile(fname));
          KRun::runURL(u, (func->files()->vfs_search(fname))->vfile_getMime());
        }
        iterator=iterator->itemBelow();
      }
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
      MountMan::KMountMan::eject(func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case SHRED_ID :
      if (KMessageBox::warningContinueCancel(krApp,
          i18n("Are you sure you want to shred ")+"\""+fileList->getFilename(item)+"\""+
          " ? Once shred, the file is gone forever !!!",
          QString::null,KStdGuiItem::cont(),"Shred") == KMessageBox::Continue )
        KShred::shred(func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case OPEN_KONQ_ID :   // open in konqueror
      kapp->startServiceByDesktopName("konqueror",func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case CHOOSE_ID :      // Other...
      //new KFileOpenWithHandler();
      u.setPath(func->files()->vfs_getFile(fileList->getFilename(item)));
      lst.append(u);
      KRun::displayOpenWithDialog(lst);
      break;
    case MOUNT_ID :
      krMtMan.mount(func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case NEW_LINK :
			func->krlink(false);
			break;
    case NEW_SYMLINK :
			func->krlink(true);
			break;
		case REDIRECT_LINK :
			func->redirectLink();
			break;
    case UNMOUNT_ID :
      krMtMan.unmount(func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case SEND_BY_EMAIL :
      SLOTS->sendFileByEmail(func->files()->vfs_getFile(fileList->getFilename(item)));
      break;
    case OPEN_TERM_ID :   // open in terminal
		  QString save = getcwd(0,0);
		  chdir( func->files()->vfs_getFile(item->text(0)).local8Bit() );
      KProcess proc;
		  krConfig->setGroup("General");
		  QString term = krConfig->readEntry("Terminal",_Terminal);
		  proc << term;
      if (!vf->vfile_isDir()) proc << "-e" << fileList->getFilename(item);
  	  if(!proc.start(KProcess::DontCare))
  	    KMessageBox::sorry(krApp,i18n("Can't open ")+"\""+term+"\"");
		  chdir(save.local8Bit());
      break;
		}
    if( result >= SERVICE_LIST_ID ){
      u.setPath(func->files()->vfs_getFile(fileList->getFilename(item)));
      lst.append(u);
      KRun::run(*(offers[result-SERVICE_LIST_ID].service()),lst);
    }
}

void ListPanel::slotSelectionChanged(QListViewItem *item){
  if( fileList->getFilename(item) != ".." )
    setNameToMakeCurrent(fileList->getFilename(item));
}

void ListPanel::setFilter(FilterSpec f){
	switch(f){
  	case ALL : 		filter = ALL; 	
									break;
		case EXEC: 		filter = EXEC;
									break;
		case CUSTOM : filterMask = KRSpWidgets::getMask(i18n(" Select Files "));
									// if the user canceled - quit
									if (filterMask == QString::null) return;
									filter = CUSTOM;
									break;
		default:			return;
	}
	func->refresh();
}

void ListPanel::popBookmarks() {
  bookmarks->exec(bookmarkList->mapToGlobal(QPoint(0,bookmarkList->height())));
}

QString ListPanel::getCurrentName() {
  QString name;
  QListViewItem *it = fileList->currentItem();
  if ( !it ) return QString::null;  // safety
  if ( (name = fileList->getFilename(it)) != "..")
    return name;
  else return QString::null;
}

void ListPanel::prepareToDelete() {
	setNameToMakeCurrent(fileList->getFilename(fileList->firstUnmarkedAboveCurrent()));
}

// this is called if the rightclick menu is popped from the keyboard
void ListPanel::popRightClickMenu() {
  // find out what's the current item and it's location
  QListViewItem *item = fileList->currentItem();
  if (item == 0) return;
  QPoint loc(fileList->mapToGlobal(fileList->itemRect(item).topLeft()));
  popRightClickMenu(fileList, item, loc);
}
