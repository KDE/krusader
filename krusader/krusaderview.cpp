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
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "resources.h"
#include <klibloader.h> //<>

KrusaderView::KrusaderView(QWidget *parent, const char *name ) : QWidget(parent,name),
  konsole_part(0L) {}

void KrusaderView::start() {
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
  left=new ListPanel(horiz_splitter, true);
	right=new ListPanel(horiz_splitter, false);

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
  right->start();left->start();
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

void KrusaderView::cmdLineFocus() {  // command line receive's keyboard focus
  cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus() { // return focus to the active panel
  activePanel->slotFocusOnMe();
}

// Tab - switch focus
void KrusaderView::panelSwitch(){ activePanel->otherPanel->slotFocusOnMe(); }
void KrusaderView::slotSetActivePanel(ListPanel *p) { activePanel=p; }

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
