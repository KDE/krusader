/***************************************************************************
                                krusaderview.cpp
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

// Krusader includes
#include "krusaderview.h"
#include "krusader.h"
#include "krslots.h"
#include "defaults.h"
#include "Panel/kvfspanel.h"
#include "Panel/panelfunc.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "resources.h"
#include <klibloader.h> //<>

KrusaderView::KrusaderView(QWidget *parent, const char *name ) : QWidget(parent,name),
  konsole_part(0L) {
  ////////////////////////////////
  // make a 1x1 mainLayout, it will auto-expand:
	mainLayout = new QGridLayout(this, 1, 1);
	// vertical splitter
	vert_splitter = new QSplitter(this); // splits between panels and terminal/cmdline
	vert_splitter->setOrientation(QObject::Vertical);
	// horizontal splitter
	horiz_splitter = new QSplitter( vert_splitter );
  (terminal_dock = new QHBox(vert_splitter))->hide(); // create it hidden
	// create a command line thing
  cmdLine=new KCMDLine(this);

  // add 2 pseudo widgets and layouts to allow a smoother movement of the
  // whole screen, and allow the status bars to resize with the panels
  krConfig->setGroup("Startup");  // this is needed !
  QString leftType = krConfig->readEntry("Left Panel Type",_LeftPanelType);
  if      ( leftType == i18n("Tree") )        left=new TreePanel(horiz_splitter);
  else if ( leftType == i18n("Quickview") )   left=new QuickViewPanel(horiz_splitter);
  else                                  left=new ListPanel(horiz_splitter,false);

  krConfig->setGroup("Startup");  // this is needed too
  QString rightType = krConfig->readEntry("Right Panel Type",_RightPanelType);
  if      ( rightType == i18n("Tree") )       right=new TreePanel(horiz_splitter);
  else if ( rightType == i18n("Quickview") )  right=new QuickViewPanel(horiz_splitter);
  else                                  right=new ListPanel(horiz_splitter,true);

  left->setOther(right); right->setOther(left);

  // create the function keys widget
	fnKeys=new KFnKeys(this);
	fnKeys->show();

  // and insert the whole thing into the main layout... at last	
	mainLayout->addWidget(vert_splitter,0,0);  //<>
	mainLayout->addWidget(cmdLine,1,0);
	mainLayout->addWidget(fnKeys,2,0);
	mainLayout->activate();
	
	// get the last saved sizes of the splitter
	krConfig->setGroup("Private");
	QValueList<int> lst = krConfig->readIntListEntry("Splitter Sizes");
	if (!lst.isEmpty()) horiz_splitter->setSizes(lst);
	show();

  qApp->processEvents();
  // make the left panel focused at program start
  right->start(false);left->start(true);
  activePanel=left; activePanel->slotFocusOnMe();  // left starts out active
}

// updates the command line whenever current panel changes
//////////////////////////////////////////////////////////
void KrusaderView::slotCurrentChanged(QString p) {
  cmdLine->setCurrent(p);
  if (konsole_part != 0L) {
    konsole_part->openURL(KURL(p));
  }
}

void KrusaderView::setQuickView() {
  KVFSPanel *temp;
 	
	if (activePanel->type == "quickview") return; // if the panel is a quick view panel, exit
  // otherwise, we first kill the active panel and recreate it, saving pointers
  hide();
	
	if (activePanel==right) {
	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    temp=left;
    // create a new panel
		left = new QuickViewPanel(horiz_splitter);
		left->otherPanel=right;
    right->otherPanel=left;
//    activePanel=left;
    // move the new one to the left-most position
    horiz_splitter->moveToFirst(left);
 	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    // and kill the old one
    horiz_splitter->refresh();
    right->slotFocusOnMe();
    left->start(true);
    ((QuickViewPanel*)left)->view(right->fileList->currentItem());
  }
  else {
	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    temp=right;
    // create a new panel
    right = new QuickViewPanel(horiz_splitter);
		right->otherPanel=left;
    left->otherPanel=right;
//    activePanel=right;
    // move the new one to the right-most position
    horiz_splitter->moveToLast(right);
 	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    // and kill the old one
    horiz_splitter->refresh();
    left->slotFocusOnMe();
		right->start(false);
    ((QuickViewPanel*)right)->view(left->fileList->currentItem());
  }
	delete temp;
	show();
}


void KrusaderView::setListView() {
  KVFSPanel *temp;

  if (activePanel->type == "list") return; // if the panel is a list-panel, exit
  // otherwise, we first kill the active panel and recreate it, saving pointers
  if (activePanel==left) {
	  hide();
	  // keep the right immobile
	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    temp=left;
    // create a new panel
    left = new ListPanel(horiz_splitter,false);
    left->otherPanel=right;
    right->otherPanel=left;
    activePanel=left;
    left->start(true);
    // move the new one to the left-most position
    horiz_splitter->moveToFirst(left);
 	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    // and kill the old one
    delete temp;
    horiz_splitter->refresh();
    left->slotFocusOnMe();
    show();
		left->refresh();
    return;
  }
  if (activePanel==right) {
	  hide();
	  // keep the left immobile
	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    temp=right;
    // create a new panel
    right = new ListPanel(horiz_splitter,true);
    right->otherPanel=left;
    left->otherPanel=right;
    activePanel=right;
    right->start(false);
    // move the new one to the right-most position
    horiz_splitter->moveToLast(right);
 	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    // and kill the old one
    delete temp;
    horiz_splitter->refresh();
    right->slotFocusOnMe();
    show();
		right->refresh();
    return;
  }
}

void KrusaderView::setTreeView() {
  KVFSPanel *temp;

  if (activePanel->type == "tree") return; // if the panel is a tree-view, exit
  if (activePanel->otherPanel->type == "tree") return;  // we can't have 2 trees!!!
  // otherwise, we first kill the active panel and recreate it, saving pointers
  if (activePanel==left) {
	  hide();
	  // keep the right immobile
	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    temp=left;
    // create a new panel
    left = new TreePanel(horiz_splitter);
    left->otherPanel=right;
    right->otherPanel=left;
    activePanel=left;
    left->start(true);
    // move the new one to the left-most position
    horiz_splitter->moveToFirst(left);
 	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    // and kill the old one
    delete temp;
    horiz_splitter->refresh();
    left->slotFocusOnMe();
    show();
    return;
  }
  if (activePanel==right) {
	  hide();
	  // keep the left immobile
	  horiz_splitter->setResizeMode(left,QSplitter::KeepSize);
    temp=right;
    // create a new panel
    right = new TreePanel(horiz_splitter);
    right->otherPanel=left;
    left->otherPanel=right;
    activePanel=right;
    right->start(false);
    // move the new one to the right-most position
    horiz_splitter->moveToLast(right);
 	  horiz_splitter->setResizeMode(right,QSplitter::KeepSize);
    // and kill the old one
    delete temp;
    horiz_splitter->refresh();
    right->slotFocusOnMe();
    show();
    return;
  }
}

void KrusaderView::cmdLineFocus() {  // command line receive's keyboard focus
  cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus() { // return focus to the active panel
  activePanel->slotFocusOnMe();
}

// Tab - switch focus
void KrusaderView::panelSwitch(){ activePanel->otherPanel->slotFocusOnMe(); }
void KrusaderView::slotSetActivePanel(KVFSPanel *p) { activePanel=p; }

void KrusaderView::slotTerminalEmulator(bool show) {
  if (!show) {  // hiding the terminal
    terminal_dock->hide();
    return;
  }
  // else implied
  if (konsole_part==0L) {  // konsole part is not yet loaded
    KLibFactory *factory = KLibLoader::self()->factory( "libkonsolepart" );
    if (factory) {
      // Create the part
      konsole_part = (KParts::ReadOnlyPart *)factory->create( terminal_dock, "konsolepart",
                      "KParts::ReadOnlyPart" );
      connect(konsole_part, SIGNAL(destroyed()), this, SLOT(killTerminalEmulator()));
    } else konsole_part = 0L;
  }
  if (konsole_part) {      // if we succeeded in creating the konsole
    terminal_dock->show();
    krToggleTerminal->setChecked(true);
  } else {
    terminal_dock->hide();
    krToggleTerminal->setChecked(false);
  }
}

void KrusaderView::killTerminalEmulator() {
  konsole_part = 0L;
  slotTerminalEmulator(false);  // hide the docking widget
  krToggleTerminal->setChecked(false);
}

#include "krusaderview.moc"
