/***************************************************************************
                                kcmdline.cpp
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


#include "kcmdline.h"
#include "stdlib.h"
#include <unistd.h>
#include <qmessagebox.h>
#include <kprocess.h>
#include <qiconset.h>
#include <qwhatsthis.h>
#include <unistd.h>
#include "../krusader.h"
#include "../kicons.h"
#include "../krslots.h"
#include "../resources.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../Panel/kvfspanel.h"
#include <qdir.h>
#include <klocale.h>
#include <kglobalsettings.h>


KCMDLine::KCMDLine(QWidget *parent, const char *name ) : QWidget(parent,name) {
  QGridLayout *layout=new QGridLayout(this,1,4);
  path=new QLabel(this);
  path->setAlignment(Qt::AlignRight);
  path->setFrameStyle(QFrame::Box | QFrame::Sunken);
  path->setLineWidth(1);
  path->setFont(KGlobalSettings::generalFont());
  layout->addWidget(path,0,0);
  // and editable command line
  completion.setMode(KURLCompletion::FileCompletion);
	cmdLine=new KRLineEdit(this);
  cmdLine->setFont(KGlobalSettings::generalFont());
  cmdLine->setCompletionObject(&completion);
	QWhatsThis::add(cmdLine,i18n("Well, it's quite simple actually: You write you command in here, and Krusader obays."));
  layout->addWidget(cmdLine,0,1);
  // history button
  history=new QToolButton(this);
  history->setTextLabel("Show the last commands you typed in the command line.");
  history->setPixmap(krLoader->loadIcon("date",KIcon::Panel));
	historyMenu=new QPopupMenu(this);
	historyMenu->setFont(KGlobalSettings::generalFont());
  history->setPopup(historyMenu);	
  history->setPopupDelay(100);
	connect(historyMenu,SIGNAL(activated(int)),this,
          SLOT(setCmdLineText(int)));	
	QWhatsThis::add(history,i18n("The history button pops up a menu with the last 20 commands you wrote to the command line. Choose your path..."));
  layout->addWidget(history,0,2);
  // a run in terminal button
  terminal=new QToolButton(this);
  terminal->setTextLabel("If pressed, Krusader executes command line in a terminal.");
  terminal->setToggleButton(true);
  terminal->setOn(false);
  terminal->setPixmap(krLoader->loadIcon("konsole",KIcon::Panel));
  QWhatsThis::add(terminal,i18n("The 'run in terminal' button allows Krusader to run console (or otherwise non-graphical) programs in a terminal of your choice. If it's pressed, termial mode is active."));
  layout->addWidget(terminal,0,3);
  layout->activate();
  // connections
  connect(cmdLine,SIGNAL(returnFocus()),this,SLOT(slotReturnFocus()));
  connect(cmdLine,SIGNAL(returnPressed(const QString&)),this,
          SLOT(slotRun(/*const QString&*/)));
  connect(cmdLine,SIGNAL(upkeyPressed()),this,
  				SLOT(slotHistoryUp()));
  connect(cmdLine,SIGNAL(downkeyPressed()),this,
  				SLOT(slotHistoryDown()));
	historySize=0;
	currentItem=-1;	// which means up key wasn't pressed yet
	historyList.setAutoDelete(true);
}

KCMDLine::~KCMDLine(){
}

void KCMDLine::slotRun(){
  QString command1=cmdLine->text();
  if(command1.isEmpty()) return;
  krConfig->setGroup("General"); 	
  cmdLine->clear();
	addToHistory(command1);
	currentItem=-1;	// reset the up/down keys
  QString panelPath=path->text().left( path->text().length()-1);
  	
	if (command1.left(3)=="cd ") { // cd command effect the active panel
		QString dir = command1.right(command1.length()-command1.find(" ")).stripWhiteSpace();
		if (dir == "~") dir = QDir::homeDirPath();
		else
		if (dir.left(1)!="/") dir=panelPath+(panelPath=="/" ? "" : "/")+dir;
		SLOTS->refresh(dir);
	} else {
		QString save = getcwd(0,0);
		KShellProcess proc;
		chdir( panelPath.local8Bit() );
		// run in a terminal ???
		if (terminal->isOn()) proc << krConfig->readEntry("Terminal",_Terminal)
		                           << "-e";
		proc <<  command1;
  	proc.start(KProcess::DontCare);
  	
		chdir(save.local8Bit());
	}
}

void KCMDLine::slotHistoryUp() {
	if (currentItem==((signed)historyList.count())-1) return;	// top of the history list
	cmdLine->setText(*historyList.at(++currentItem));
}

void KCMDLine::slotHistoryDown() {
	if (currentItem<=0) return;	// top of the history list
	cmdLine->setText(*historyList.at(--currentItem));
}
	
// history remembers up to 20 commands
  //////////////////////////////////////
void KCMDLine::addToHistory(QString cmd) {
	if (historyList.count()==20)
		if (!historyList.removeLast()) {
			kdFatal() << "KCmdLine: Unable to remove item from list. bailing out" << endl;
			exit(0);
		}
	QString *temp=new QString(cmd);
	if (!historyList.insert(0,temp)) {
		kdFatal() << "KCmdLine: Unable to allocate memory. bailing out" << endl;
		exit(0);
	}
	refreshHistory();
}

void KCMDLine::refreshHistory() {
  historyMenu->clear();   // just in case
	for (int i=historyList.count()-1; i>=0; --i)	
	    historyMenu->insertItem(*historyList.at(i),i);
}

void KCMDLine::setCmdLineText(int id) {
	cmdLine->setText(*historyList.at(id));
	currentItem=id;
}

void KCMDLine::slotReturnFocus() {
  Krusader::App->mainView->cmdLineUnFocus();
}

#include "kcmdline.moc"

