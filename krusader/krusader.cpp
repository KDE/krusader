/***************************************************************************
                                krusader.cpp
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
#include <sys/types.h>
#include <sys/stat.h>
// KDE includes
#include <kmessagebox.h>
#include <kaction.h>
#include <kcursor.h>
#include <ksystemtray.h>
#include <kmenubar.h>
#include <kapp.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <klocale.h>
// QT includes
#include <qpixmap.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qprinter.h>
#include <qprogressdialog.h>
#include <qvaluelist.h>
// Krusader includes
#include "krusader.h"
#include "kicons.h"
#include "VFS/krpermhandler.h"
#include "BookMan/bookman.h"
#include "GUI/krusaderstatus.h"
#include "RemoteMan/remoteman.h"
#include "Dialogs/krpleasewait.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Konfigurator/konfigurator.h"
#include "MountMan/kmountman.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "krslots.h"
#include "krservices.h"

// define the static members
Krusader *Krusader::App=0;
KAction  *Krusader::actProperties=0;    KAction  *Krusader::actPack=0;
KAction  *Krusader::actUnpack=0;        KAction  *Krusader::actTest=0;
KAction  *Krusader::actCompare=0;       KAction  *Krusader::actCalculate=0;
KAction  *Krusader::actSelect=0;        KAction  *Krusader::actSelectAll=0;
KAction  *Krusader::actUnselect=0;      KAction  *Krusader::actUnselectAll=0;
KAction  *Krusader::actInvert=0;        KAction  *Krusader::actSysInfo=0;
KAction  *Krusader::actSync=0;          KAction  *Krusader::actHomeTerminal=0;
KAction  *Krusader::actFTPConnect=0;    KAction  *Krusader::actFTPNewConnect=0;
KAction  *Krusader::actFTPDisconnect=0; KAction  *Krusader::actMultiRename=0;
KAction  *Krusader::actAllFilter=0;     KAction  *Krusader::actExecFilter=0;
KAction  *Krusader::actCustomFilter=0;  KAction  *Krusader::actMountMan=0;
KAction  *Krusader::actBookMan=0;       KAction  *Krusader::actNewTool=0;
KAction  *Krusader::actKonfigurator=0;  KAction  *Krusader::actToolsSetup=0;
KAction  *Krusader::actBack=0;          KAction  *Krusader::actRoot=0;
KAction  *Krusader::actFind=0;          KAction  *Krusader::actAddBookmark=0;
KAction  *Krusader::actSavePosition=0;  KAction  *Krusader::actSelectColorMask=0;

KToggleAction *Krusader::actToggleTerminal=0;


// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader() : KParts::MainWindow() {
  // create the "krusader"
  App=this;
	slot = new KRslots();
	setXMLFile("krusaderui.rc"); // kpart-related xml file

  plzWait = new KRPleaseWaitHandler();

  bool runKonfig=versionControl();

  QString message;
  switch (config->getConfigState()) {
    case KConfigBase::NoAccess :
      message="Krusader's configuration file can't be found. Default values will be used.";
      break;
    case KConfigBase::ReadOnly :
      message="Krusader's configuration file is in READ ONLY mode (why is that !?) Changed values will not be saved";
      break;
    case KConfigBase::ReadWrite :
      message="";
     break;
  }
  if (message!="") {
    KMessageBox::error(krApp,message);
  }


  // register with the dcop server
	DCOPClient* client = KApplication::kApplication()->dcopClient();
	if ( !client->attach() ) exit(0);
	client->registerAs( KApplication::kApplication()->name() );
	
	// create an icon loader
	iconLoader = KGlobal::iconLoader();
	
	// create BookMan
  bookMan  = new BookMan();
  
	// create MountMan
	mountMan = new MountMan::KMountMan();

	// setup all the krusader's actions
	setupActions();

	// init the permmision handler class
  KRpermHandler::init();

  // create the main view and set it
  mainView=new KrusaderView(this);

  // setup keyboard accelerators	
	setupAccels();
	
	// create a status bar
	status=new KrusaderStatus(this);

	// This enables Krusader to minimize to tray if needed
	KSystemTray *st=new KSystemTray(this);
	st->setPixmap(iconLoader->loadIcon("krusader",KIcon::Panel));

  setCentralWidget(mainView);
  config->setGroup("Look&Feel");
  if (krConfig->readBoolEntry("Minimize To Tray",_MinimizeToTray))
    st->show(); else show();

  setCursor(KCursor::arrowCursor());
  // first, resize and move to starting point
  config->setGroup("Private");
  move(krConfig->readPointEntry("Start Position",_StartPosition));
  resize(krConfig->readSizeEntry("Start Size",_StartSize));

  // let the good times rool :)
  updateGUI(true);
  if (runKonfig) slot->runKonfigurator(true);

  // refresh the right and left panels
	mainView->left->refresh();
	mainView->right->refresh();
}

Krusader::~Krusader(){}

bool Krusader::versionControl() {
  bool retval=false;
  // create config file
  config=kapp->config();
  QString oldVerText = config->readEntry("Version","200");
  oldVerText.truncate(oldVerText.find("-"));
  float oldVer = oldVerText.toFloat();
	
	kdDebug() << QString("version = %1").arg(oldVer) << endl;

  // older icompatible version
	if ( oldVer < (9/10) ){
		KMessageBox::information(krApp,i18n("A configuration older then v0.90 was detected Krusader has to reset your configuration to default values. Krusader will now run Konfigurator."));
    if (!QDir::home().remove(".kde/share/config/krusaderrc")) {
      KMessageBox::error(krApp,i18n("Unable to remove your krusaderrc file! Please remove it manually and rerun Krusader."));
      exit(1);
    }
    retval=true;
    config->reparseConfiguration();
  }
  // first installation of krusader
	if ( oldVer == 100 ){
		KMessageBox::information(krApp,i18n("Welcome to Krusader, as this is your first run, Krusader will now run Konfigurator."));
  	retval = true;
	}
	config->writeEntry("Version",VERSION);
  config->sync();
  return retval;
}

void Krusader::statusBarUpdate(QString mess) {
  // change the message on the statusbar for 2 seconds
  statusBar()->message(mess,5000);
}

void Krusader::setupAccels() {
  accels = new KAccel(this);
  // F2
  accels->insert("F2_Terminal", i18n("F2 Terminal"), QString::null,
                  Key_F2, SLOTS, SLOT(terminal()));
  // F3
  accels->insert("F3_View", i18n("F3 View"), QString::null,
                  Key_F3, SLOTS, SLOT(view()));
  // F4
  accels->insert("F4_Edit", i18n("F4 Edit"), QString::null,
                  Key_F4, SLOTS, SLOT(edit()));
  // F5
  accels->insert("F5_Copy", i18n("F5 Copy"), QString::null,
                  Key_F5, SLOTS, SLOT(copyFiles()));
  // F6
  accels->insert("F6_Move", i18n("F6 Move"), QString::null,
                  Key_F6, SLOTS, SLOT(moveFiles()));
  // F7
  accels->insert("F7_Mkdir", i18n("F7 Mkdir"), QString::null,
                  Key_F7, SLOTS, SLOT(mkdir()));
  // F8
  accels->insert("F8_Delete", i18n("F8 Delete"), QString::null,
                  Key_F8, SLOTS, SLOT(deleteFiles()));
  // F9
  accels->insert("F9_Rename", i18n("F9 Rename"), QString::null,
                  Key_F9, SLOTS, SLOT(rename()));
  // F10
  accels->insert("F10_Quit", i18n("F10 Quit"), QString::null,
                  Key_F10, this, SLOT(quitKrusader()));
  // Tab
  accels->insert("Tab-Switch panel", i18n("Tab: switch panel"), QString::null,
                  Key_Tab, mainView, SLOT(panelSwitch()));

}

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
  // first come the TODO actions
  //actSync =       0;//new KAction(i18n("S&yncronize Dirs"),                         0, this, 0, actionCollection(), "sync dirs");
	//actNewTool =    0;//new KAction(i18n("&Add a new tool"),                          0, this, 0, actionCollection(), "add tool");
	//actToolsSetup = 0;//new KAction(i18n("&Tools Menu Setup"),                        0, 0, this, 0, actionCollection(), "tools setup");
  //KStdAction::print(SLOTS, 0,actionCollection(),"std_print");


	// second, the KDE standard action
  KStdAction::up(SLOTS,SLOT(dirUp()),actionCollection(),"std_up");
	KStdAction::home(SLOTS,SLOT(home()),actionCollection(),"std_home");
  KStdAction::redisplay(SLOTS,SLOT(refresh()),actionCollection(),"std_redisplay");
  actShowToolBar   = KStdAction::showToolbar(SLOTS, SLOT(toggleToolbar()),actionCollection(),"std_toolbar");	
  actShowStatusBar = KStdAction::showStatusbar(SLOTS, SLOT(toggleStatusbar()),actionCollection(),"std_statusbar");	
  KStdAction::quit(this, SLOT(quitKrusader()),actionCollection(),"std_quit");
  KStdAction::configureToolbars(SLOTS,SLOT(configToolbar()),actionCollection(),"std_config_toolbar");
  KStdAction::keyBindings(SLOTS,SLOT(configKeys()),actionCollection(),"std_config_keys");

  // the toggle actions
  actCompareDirs = new KToggleAction(i18n("Compare Mode"), "kr_comparedirs",
                           CTRL+Key_Equal, SLOTS, SLOT(compareDirectories()), actionCollection(), "compare mode");
  actCompareDirs->setChecked(false);
	actToggleFnkeys =   new KToggleAction(i18n("Show &FN Keys Bar"),0,SLOTS,
	                       SLOT(toggleFnkeys()),actionCollection(),"toggle fn bar");
	actToggleFnkeys->setChecked(true);
	actToggleCmdline =  new KToggleAction(i18n("Show &Command Line"),0,SLOTS,
	                       SLOT(toggleCmdline()),actionCollection(),"toggle command line");
	actToggleCmdline->setChecked(true);
	actToggleTerminal = new KToggleAction(i18n("Show &Terminal Emulator"),0,SLOTS,
	                       SLOT(toggleTerminal()),actionCollection(),"toggle terminal emulator");
	actToggleTerminal->setChecked(false);
	actToggleHidden =   new KToggleAction(i18n("Show H&idden Files"),0,SLOTS,
                         SLOT(toggleHidden()),actionCollection(),"toggle hidden files");
	actToggleSortByExt = new KToggleAction(i18n("Sort By E&xtention"),0,SLOTS,
                         SLOT(toggleSortByExt()),actionCollection(),"toggle sort by extention");
  krConfig->setGroup("Look&Feel");
	actToggleHidden->setChecked(krConfig->readBoolEntry("Show Hidden",_ShowHidden));
	actToggleSortByExt->setChecked(krConfig->readBoolEntry("Sort By Extention",_SortByExt));
	
	// and then the DONE actions
	actSelectColorMask =new KAction(i18n("Configure compare-mode"), 0,
	                      SLOTS,     SLOT(selectCompareMask()),      actionCollection(), "select colormask");
	actTest =           new KAction(i18n("Test Archi&ve(s)"), "kr_arc_test", CTRL+Key_T,
	                      SLOTS,     SLOT(testArchive()),      actionCollection(), "test archives");
  actFTPConnect =     new KAction(i18n("&FTP Connect"), "kr_ftp_connect", CTRL+Key_F,
												SLOTS, 		SLOT(runRemoteMan()), 		actionCollection(), "ftp connect");
	actFTPNewConnect =  new KAction(i18n("FT&P New Connection"), "kr_ftp_new",    CTRL+Key_N,
	                      SLOTS,     SLOT(newFTPconnection()), actionCollection(), "ftp new connection");
	actCalculate =      new KAction(i18n("Calculate &Occupied Space"), "kr_calc",  0,
                        SLOTS,     SLOT(calcSpace()),        actionCollection(), "calculate");
  actProperties =     new KAction(i18n("&Properties"), "kr_properties", 0,
                        SLOTS,     SLOT(properties()),       actionCollection(),"properties");
  actPack =           new KAction(i18n("Pac&k"),            "kr_arc_pack", CTRL+Key_P,
                        SLOTS,     SLOT(slotPack()),         actionCollection(), "pack");
  actUnpack =         new KAction(i18n("&Unpack"),          "kr_arc_unpack", CTRL+Key_U,
                        SLOTS,     SLOT(slotUnpack()),       actionCollection() , "unpack");
  actSelect =         new KAction(i18n("Select &Group"),    "kr_select",         CTRL+Key_Plus,
                        SLOTS,     SLOT(markGroup()),        actionCollection(), "select group");
  actSelectAll =      new KAction(i18n("&Select All"),      "kr_selectall",        ALT+Key_Plus,
                        SLOTS,     SLOT(markAll()),          actionCollection(), "select all");
  actUnselectAll =    new KAction(i18n("U&nselect All"),    "kr_unselectall",      ALT+Key_Minus,
                        SLOTS,     SLOT(unmarkAll()),        actionCollection(), "unselect all");
	actHomeTerminal =   new KAction(i18n("&Terminal"),        "kr_terminal", 0,
                        SLOTS,     SLOT(homeTerminal()),     actionCollection(), "terminal@home");
  actFTPDisconnect =  new KAction(i18n("FTP Disc&onnect"),  "kr_ftp_disconnect",     SHIFT+CTRL+Key_F,
	                      SLOTS,     SLOT(FTPDisconnect()),    actionCollection(), "ftp disconnect");
	actMountMan =       new KAction(i18n("&MountMan"),        "kr_mountman",       ALT+Key_Slash,
                        SLOTS,     SLOT(runMountMan()),      actionCollection(), "mountman");
	actBookMan =        new KAction(i18n("&BookMan"),         "kr_bookman",  0,
	                      krBookMan,SLOT(showGUI()),          actionCollection(), "bookman");
	actFind =           new KAction(i18n("&Search"),            "filefind",         CTRL+Key_S,
                        SLOTS,     SLOT(search()),           actionCollection(), "find");
  actInvert =         new KAction(i18n("&Invert Selection"),"kr_invert",         ALT+Key_Asterisk,
	                      SLOTS,     SLOT(invert()),           actionCollection(), "invert");
	actUnselect =       new KAction(i18n("&Unselect &Group"), "kr_unselect",         CTRL+Key_Minus,
                        SLOTS,     SLOT(unmarkGroup()),      actionCollection(), "unselect group");
  actAddBookmark =    new KAction(i18n("Add Bookmark"),     "kr_addbookmark",    CTRL+Key_B,
                        SLOTS,     SLOT(addBookmark()),      actionCollection(), "add bookmark");
  actKonfigurator =   new KAction(i18n("&Konfigurator"),    "configure", 0,
                        SLOTS,     SLOT(startKonfigurator()),  actionCollection(), "konfigurator");
	actBack =           new KAction(i18n("Back"),             "back",         0,
                        SLOTS,     SLOT(back()),             actionCollection(), "back");
  actRoot =           new KAction(i18n("Root"),             "top",  CTRL+Key_Backspace,
                        SLOTS,     SLOT(root()),             actionCollection(), "root");
  actSysInfo =        new KAction(i18n("&Device Manager"),   "kr_hwinfo",   0,
                        SLOTS,     SLOT(sysInfo()),          actionCollection(), "sysinfo");
  actSavePosition =   new KAction(i18n("Save &Position"),                             0,
	                      krApp,     SLOT(savePosition()),     actionCollection(), "save position");
  actAllFilter =  		new KAction(i18n("&All Files"),  SHIFT+Key_F10,
												SLOTS, 		SLOT(allFilter()),				 actionCollection(), "all files");
	actExecFilter = 		new KAction(i18n("&Executables"),SHIFT+Key_F11,
												SLOTS, 		SLOT(execFilter()),     	 actionCollection(), "exec files");
	actCustomFilter=		new KAction(i18n("&Custom"), 				 SHIFT+Key_F12,
												SLOTS,			SLOT(customFilter()), 	 actionCollection(), "custom files");
  actCompare = 				new KAction(i18n("Compare b&y content"), "kr_compare", 0,
												SLOTS, SLOT(compareContent()), actionCollection(), "compare");
  actMultiRename = 		new KAction(i18n("Multi Rename"), "krename", SHIFT+Key_F9,
                        SLOTS, SLOT(multiRename()), actionCollection(), "multirename");
	new KAction(i18n("Right-click menu"), Key_Menu,
      				SLOTS,			SLOT(rightclickMenu()), 		actionCollection(), "rightclick menu");
																
	// and at last we can set the tool-tips
  actSelect->setToolTip(i18n("Highlight files by using a filter"));
  actSelectAll->setToolTip(i18n("Highlight all the files in the current directory"));
  actUnselectAll->setToolTip(i18n("Remove selection from all highlight files"));
  actMountMan->setToolTip(i18n("Mount.Man: Krusader's mount-manager. Try it!"));
  actKonfigurator->setToolTip(i18n("Setup Krusader the way you like it"));
	actBack->setToolTip(i18n("Back to the place you came from"));
  actRoot->setToolTip(i18n("ROOT (/)"));
  actFind->setToolTip(i18n("Search for files"));
  actAddBookmark->setToolTip(i18n("Add the current path to your bookmarks"));
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////
	
void Krusader::savePosition(){
  config->setGroup("Private");
  config->writeEntry( "Start Position", pos() );
  config->writeEntry( "Start Size", size() );
  config->writeEntry( "Panel Size", mainView->vert_splitter->sizes()[0]);
  config->writeEntry( "Terminal Size", mainView->vert_splitter->sizes()[1]);

  config->writeEntry( "Left Name Size", mainView->left->fileList->columnWidth(0));
  config->writeEntry( "Left Size Size", mainView->left->fileList->columnWidth(1));
  config->writeEntry( "Left Date Size", mainView->left->fileList->columnWidth(2));
  config->writeEntry( "Right Name Size", mainView->right->fileList->columnWidth(0));
  config->writeEntry( "Right Size Size", mainView->right->fileList->columnWidth(1));
  config->writeEntry( "Right Date Size", mainView->right->fileList->columnWidth(2));
  QValueList<int> lst = mainView->horiz_splitter->sizes();
  config->writeEntry( "Splitter Sizes", lst);

  config->sync();
}

void Krusader::saveSettings() {
  toolBar()->saveSettings(krConfig,"Private");
  config->setGroup("Startup");
  if (config->readBoolEntry("Panels Save Settings",_PanelsSave)){
    // left panel
    config->writeEntry("Left Panel Type",i18n("List"));
    config->writeEntry("Left Panel Origin",i18n("the last place it was"));
    // right panel
    config->writeEntry("Right Panel Type",i18n("List"));
    config->writeEntry("Right Panel Origin",i18n("the last place it was"));
  }

  config->writeEntry("lastHomeLeft",mainView->left->realPath);
  config->writeEntry("lastHomeRight",mainView->right->realPath);

  // save size and position
  if (config->readBoolEntry("Remember Position",_RememberPos) ||
      config->readBoolEntry("UI Save Settings",_UiSave) ) {
      savePosition();
  }

  // save the gui
  if ( config->readBoolEntry("UI Save Settings",_UiSave ) ){
    config->writeEntry( "Show status bar",actShowStatusBar->isChecked() );
    config->writeEntry( "Show tool bar",actShowToolBar->isChecked() );
    config->writeEntry( "Show FN Keys",actToggleFnkeys->isChecked() );
    config->writeEntry( "Show Cmd Line",actToggleCmdline->isChecked() );
    config->writeEntry( "Show Terminal Emulator",actToggleTerminal->isChecked());
  }
  config->sync();
  // delete all vfs records...
  mainView->left->cleanUp();
  mainView->right->cleanUp();
}

void Krusader::refreshView(){
	delete mainView;
	mainView=new KrusaderView(this);
  setCentralWidget(mainView);
  config->setGroup("Private");
  resize(krConfig->readSizeEntry("Start Size",_StartSize));
  move(krConfig->readPointEntry("Start Position",_StartPosition));
  mainView->show();
  show();	
}

bool Krusader::queryClose() {
  saveSettings();
  krConfig->setGroup("Look&Feel");
	if (krConfig->readBoolEntry("Warn On Exit",_WarnOnExit)) {
    switch ( KMessageBox::warningYesNo( this,
      i18n("Ok to shutdown Krusader ?")) ) {
      case KMessageBox::Yes :
        return true;
      case KMessageBox::No :
        return false;
      default:
        return false;
    }
  } else return true;
}

void Krusader::quitKrusader() {
  if (queryClose()) kapp->quit();
}

// the please wait dialog functions
void Krusader::startWaiting( QString msg, int count , bool cancel){
  plzWait->startWaiting( msg ,count, cancel );
}

void Krusader::incProgress(KProcess *,char *buffer,int buflen){
  int howMuch=0;
  for (int i=0 ; i<buflen; ++i)
   if ( buffer[i] == '\n' ) ++howMuch;

  plzWait->incProgress(howMuch);
}

void Krusader::stopWait(){
  plzWait->stopWait();
}

void Krusader::updateGUI(bool enforce) {
  // now, check if we need to create a konsole_part
  config->setGroup("Startup");
  if (config->readBoolEntry("Show Terminal Emulator",_ShowTerminalEmulator)) {
    if (enforce) {
      mainView->slotTerminalEmulator(true); // create konsole_part
      config->setGroup("Private");
      QValueList<int> lst;
      lst.append(config->readNumEntry("Panel Size",_PanelSize));
      lst.append(config->readNumEntry("Terminal Size",_TerminalSize));
      mainView->vert_splitter->setSizes(lst);
      config->setGroup("Startup");
    }
  }

  // call the XML GUI function to draw the UI
  createGUI(mainView->konsole_part);
  toolBar()->applySettings(krConfig,"Private");
  if (enforce) {
    // now, hide what need to be hidden
    if (!krConfig->readBoolEntry("Show tool bar",_ShowToolBar)){
      toolBar()->hide();
      actShowToolBar->setChecked(false);
    } else {
      toolBar()->show();
      actShowToolBar->setChecked(true);
    }
    if (!krConfig->readBoolEntry("Show status bar",_ShowStatusBar)){
      statusBar()->hide();
      actShowStatusBar->setChecked(false);
    } else {
      statusBar()->show();
      actShowStatusBar->setChecked(true);
    }
    if (!krConfig->readBoolEntry("Show Cmd Line",_ShowCmdline)){
      mainView->cmdLine->hide();
      actToggleCmdline->setChecked(false);
    } else {
      mainView->cmdLine->show();
      actToggleCmdline->setChecked(true);
    }
    if (!krConfig->readBoolEntry("Show FN Keys",_ShowFNkeys)){
      mainView->fnKeys->hide();
      actToggleFnkeys->setChecked(false);
    } else {
      mainView->fnKeys->show();
      actToggleFnkeys->setChecked(true);
    }
  }
}

// return a list in the format of TOOLS,PATH. for example
// DIFF,kdiff,TERMINAL,konsole,...
//
// currently supported tools: DIFF, MAIL, RENAME
//
// to use it: QStringList lst = supportedTools();
//            int i = lst.findIndex("DIFF");
//            if (i!=-1) pathToDiff=lst[i+1];
QStringList Krusader::supportedTools() {
  QStringList tools;

  // first, a diff program: kdiff
  if ( KrServices::cmdExist("kdiff") ){
		tools.append("DIFF"); tools.append("kdiff");
  }
	else if ( KrServices::cmdExist("kompare") ) {
		tools.append("DIFF"); tools.append("kompare");
	}
  else if ( KrServices::cmdExist("xxdiff") ) {
		tools.append("DIFF"); tools.append("xxdiff");
	}
  // a mailer: kmail
  if ( KrServices::cmdExist("kmail") ) {
    tools.append("MAIL"); tools.append("kmail");
	}
	// rename tool: krename
  if ( KrServices::cmdExist("krename") ) {
  	tools.append("RENAME"); tools.append("krename");
	}

  return tools;
}

QString Krusader::getTempDir(){
	// try to make krusader temp dir
  krConfig->setGroup("General");
  QString tmpDir =  krConfig->readEntry("Temp Directory",_TempDirectory);
  for(int i = 1 ; i != -1 ; i=tmpDir.find('/',i+1) )
    QDir().mkdir( tmpDir.left(i) );
  QDir().mkdir( tmpDir );
  chmod( tmpDir.local8Bit(), 0777);
	// add a secure sub dir under the user UID
	QString uid;
	uid.sprintf("%d",getuid());
	QDir(tmpDir).mkdir(uid);
	tmpDir=tmpDir+"/"+uid+"/";
	chmod(tmpDir.local8Bit(),S_IRUSR | S_IWUSR | S_IXUSR );
	// add a random sub dir to use
	while (QDir().exists(tmpDir)) tmpDir = tmpDir+kapp->randomString(8);
	QDir().mkdir(tmpDir);

  if( !QDir(tmpDir).isReadable() ){
    KMessageBox::error(krApp,"Could not create a temporary directory. Handling of Archives will not be possible until this is fixed.");
		return QString::null;
	}
	return tmpDir;
}

QString Krusader::getTempFile(){
	// try to make krusader temp dir
  krConfig->setGroup("General");
  QString tmpDir =  krConfig->readEntry("Temp Directory",_TempDirectory);
  for(int i = 1 ; i != -1 ; i=tmpDir.find('/',i+1) )
    QDir().mkdir( tmpDir.left(i) );
  QDir().mkdir( tmpDir );
  chmod( tmpDir.local8Bit(), 0777);
	// add a secure sub dir under the user UID
	QString uid;
	uid.sprintf("%d",getuid());
	QDir(tmpDir).mkdir(uid);
	tmpDir=tmpDir+"/"+uid+"/";
	chmod(tmpDir.local8Bit(),S_IRUSR | S_IWUSR | S_IXUSR );

  while(QDir().exists(tmpDir)) tmpDir = tmpDir+kapp->randomString(8);
  return tmpDir;
}


#include "krusader.moc"
