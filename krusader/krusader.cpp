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
#include <kapplication.h>
#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kaccelmanager.h>
// QT includes
#include <qpixmap.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qprinter.h>
#include <qprogressdialog.h>
#include <qvaluelist.h>
#include <qwhatsthis.h>
// Krusader includes
#include "krusader.h"
#include "kicons.h"
#include "VFS/krpermhandler.h"
#include "GUI/krusaderstatus.h"
#include "RemoteMan/remoteman.h"
#include "Dialogs/krpleasewait.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "Konfigurator/konfigurator.h"
#include "MountMan/kmountman.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "krslots.h"
#include "krservices.h"
#include "UserMenu/usermenu.h"

// define the static members
Krusader *Krusader::App = 0;
KAction *Krusader::actProperties = 0;
KAction *Krusader::actPack = 0;
KAction *Krusader::actUnpack = 0;
KAction *Krusader::actTest = 0;
KAction *Krusader::actCompare = 0;
KAction *Krusader::actCalculate = 0;
KAction *Krusader::actSelect = 0;
KAction *Krusader::actSelectAll = 0;
KAction *Krusader::actUnselect = 0;
KAction *Krusader::actUnselectAll = 0;
KAction *Krusader::actInvert = 0;
KAction *Krusader::actSync = 0;
KAction *Krusader::actHomeTerminal = 0;
KAction *Krusader::actFTPConnect = 0;
KAction *Krusader::actFTPNewConnect = 0;
KAction *Krusader::actFTPDisconnect = 0;
KAction *Krusader::actMultiRename = 0;
KAction *Krusader::actAllFilter = 0;
KAction *Krusader::actExecFilter = 0;
KAction *Krusader::actCustomFilter = 0;
KAction *Krusader::actMountMan = 0;
KAction *Krusader::actNewTool = 0;
KAction *Krusader::actKonfigurator = 0;
KAction *Krusader::actToolsSetup = 0;
KAction *Krusader::actBack = 0;
KAction *Krusader::actRoot = 0;
KAction *Krusader::actFind = 0;
//KAction *Krusader::actAddBookmark = 0;
KAction *Krusader::actSavePosition = 0;
KAction *Krusader::actSelectColorMask = 0;
KAction *Krusader::actOpenLeftBm = 0;
KAction *Krusader::actOpenRightBm = 0;
KAction *Krusader::actDirUp = 0;
KAction *Krusader::actCmdlinePopup = 0;
KAction *Krusader::actNewTab = 0;
KAction *Krusader::actDupTab = 0;
KAction *Krusader::actCloseTab = 0;
KAction *Krusader::actSplit = 0;
KAction *Krusader::actCombine = 0;
KAction *Krusader::actUserMenu = 0;
KAction *Krusader::actSyncDirs = 0;
KToggleAction *Krusader::actToggleTerminal = 0;
UserMenu *Krusader::userMenu = 0;

bool showTrayIcon = false;

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader() : KParts::MainWindow(), sysTray( 0 ) {
  // parse command line arguments
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString leftPath, rightPath;

  if (args->isSet("left")) leftPath = args->getOption("left");
  else leftPath = QString::null;
  if (args->isSet("right")) rightPath = args->getOption("right");
  else rightPath = QString::null;

  // create the "krusader"
  App = this;
  slot = new KRslots();
  setXMLFile( "krusaderui.rc" ); // kpart-related xml file

  plzWait = new KRPleaseWaitHandler();

  bool runKonfig = versionControl();

  QString message;
  switch ( config->getConfigState() ) {
      case KConfigBase::NoAccess :
      message = "Krusader's configuration file can't be found. Default values will be used.";
      break;
      case KConfigBase::ReadOnly :
      message = "Krusader's configuration file is in READ ONLY mode (why is that !?) Changed values will not be saved";
      break;
      case KConfigBase::ReadWrite :
      message = "";
      break;
  }
  if ( message != "" ) {
    KMessageBox::error( krApp, message );
  }


  // register with the dcop server
  DCOPClient* client = KApplication::kApplication() ->dcopClient();
  if ( !client->attach() )
    exit( 0 );
  client->registerAs( KApplication::kApplication() ->name() );

  // create an icon loader
  iconLoader = KGlobal::iconLoader();

  // create MountMan
  mountMan = new MountMan::KMountMan();

  // setup all the krusader's actions
  setupActions();

  // init the permmision handler class
  KRpermHandler::init();

  // create the main view and set it
  mainView = new KrusaderView( this );
  mainView->start(leftPath, rightPath);

  // create the user menu
  userMenu = new UserMenu(this);

  // setup keyboard accelerators
  setupAccels();

  // create a status bar
  status = new KrusaderStatus( this );
  QWhatsThis::add( status, i18n( "Status bar will show  basic informations "
				"about file below mouse pointer." ) );

  // This enables Krusader to show a tray icon
  config->setGroup( "Look&Feel" );
  if ( krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray ) ) {
    showTrayIcon = true;
    sysTray = new KSystemTray( this );
    sysTray->setPixmap( iconLoader->loadIcon( "krusader", KIcon::Panel, 22 ) );
    sysTray->show();
  }

  setCentralWidget( mainView );
  config->setGroup( "Look&Feel" );

  // manage our keyboard short-cuts
  //KAcceleratorManager::manage(this,true);

  setCursor( KCursor::arrowCursor() );
  // first, resize and move to starting point
  config->setGroup( "Private" );
  move( oldPos = krConfig->readPointEntry( "Start Position", _StartPosition ) );
  resize( oldSize = krConfig->readSizeEntry( "Start Size", _StartSize ) );

  if( krConfig->readBoolEntry( "Maximized" ) )
    showMaximized();
  else
    show();

  // let the good times rool :)
  updateGUI( true );
  if ( runKonfig )
    slot->runKonfigurator( true );

  // refresh the right and left panels
  mainView->right->func->refresh();
  mainView->left->func->refresh(); 
}

Krusader::~Krusader() {}

bool Krusader::versionControl() {
  bool retval = false;
  // create config file
  config = kapp->config();
  QString oldVerText = config->readEntry( "Version", "200" );
  oldVerText.truncate( oldVerText.find( "-" ) );
  float oldVer = oldVerText.toFloat();

  //kdDebug() << QString( "version = %1" ).arg( oldVer ) << endl;

  // older icompatible version
  if ( oldVer < ( 9 / 10 ) ) {
    KMessageBox::information( krApp, i18n( "A configuration older then v0.90 was detected Krusader has to reset your configuration to default values. Krusader will now run Konfigurator." ) );
    if ( !QDir::home().remove( ".kde/share/config/krusaderrc" ) ) {
      KMessageBox::error( krApp, i18n( "Unable to remove your krusaderrc file! Please remove it manually and rerun Krusader." ) );
      exit( 1 );
    }
    retval = true;
    config->reparseConfiguration();
  }
  // first installation of krusader
  if ( oldVer == 100 ) {
    KMessageBox::information( krApp, i18n( "Welcome to Krusader, as this is your first run, Krusader will now run Konfigurator." ) );
    retval = true;
  }
  config->writeEntry( "Version", VERSION );
  config->sync();
  return retval;
}

void Krusader::statusBarUpdate( QString& mess ) {
  // change the message on the statusbar for 2 seconds
  statusBar() ->message( mess, 5000 );
}

void Krusader::showEvent ( QShowEvent *e ) {
  if (showTrayIcon)
    show(); // needed to make sure krusader is removed from
            // the taskbar when minimizing (system tray issue)
  else KParts::MainWindow::showEvent(e);
}

void Krusader::hideEvent ( QHideEvent *e ) {
  if (showTrayIcon)
    hide(); // needed to make sure krusader is removed from
            // the taskbar when minimizing (system tray issue)
  else KParts::MainWindow::hideEvent(e);
}

void Krusader::moveEvent ( QMoveEvent *e ) {
  oldPos = e->oldPos();
  KParts::MainWindow::moveEvent(e);
}

void Krusader::resizeEvent ( QResizeEvent *e ) {
  oldSize = e->oldSize();
  KParts::MainWindow::resizeEvent(e);
}

void Krusader::setupAccels() {
  accels = new KAccel( this );
  // F2
  accels->insert( "F2_Terminal", i18n( "F2 Terminal" ), QString::null,
                  Key_F2, SLOTS, SLOT( terminal() ) );
  // F3
  accels->insert( "F3_View", i18n( "F3 View" ), QString::null,
                  Key_F3, SLOTS, SLOT( view() ) );
  // SHIFT+F3
  accels->insert( "F3_ViewDlg", i18n( "F3 View Dialog" ), QString::null,
                  SHIFT+Key_F3, SLOTS, SLOT( viewDlg() ) );  
  // F4
  accels->insert( "F4_Edit", i18n( "F4 Edit" ), QString::null,
                  Key_F4, SLOTS, SLOT( edit() ) );
  // F5
  accels->insert( "F5_Copy", i18n( "F5 Copy" ), QString::null,
                  Key_F5, SLOTS, SLOT( copyFiles() ) );
  // F6
  accels->insert( "F6_Move", i18n( "F6 Move" ), QString::null,
                  Key_F6, SLOTS, SLOT( moveFiles() ) );
  // F7
  accels->insert( "F7_Mkdir", i18n( "F7 Mkdir" ), QString::null,
                  Key_F7, SLOTS, SLOT( mkdir() ) );
  // F8
  accels->insert( "F8_Delete", i18n( "F8 Delete" ), QString::null,
                  Key_F8, SLOTS, SLOT( deleteFiles() ) );
  // F9
  accels->insert( "F9_Rename", i18n( "F9 Rename" ), QString::null,
                  Key_F9, SLOTS, SLOT( rename() ) );
  // F10
  accels->insert( "F10_Quit", i18n( "F10 Quit" ), QString::null,
                  Key_F10, this, SLOT( quitKrusader() ) );
  // Tab
  accels->insert( "Tab-Switch panel", i18n( "Tab: switch panel" ), QString::null,
                  Key_Tab, mainView, SLOT( panelSwitch() ) );

}

// <patch> Moving from Pixmap actions to generic filenames - thanks to Carsten Pfeiffer
void Krusader::setupActions() {
  // first come the TODO actions
  //actSync =       0;//new KAction(i18n("S&yncronize Dirs"),                         0, this, 0, actionCollection(), "sync dirs");
  //actNewTool =    0;//new KAction(i18n("&Add a new tool"),                          0, this, 0, actionCollection(), "add tool");
  //actToolsSetup = 0;//new KAction(i18n("&Tools Menu Setup"),                        0, 0, this, 0, actionCollection(), "tools setup");
  //KStdAction::print(SLOTS, 0,actionCollection(),"std_print");


  // second, the KDE standard action
  //KStdAction::up( SLOTS, SLOT( dirUp() ), actionCollection(), "std_up" )->setShortcut(Key_Backspace);
  /* Shortcut disabled because of the Terminal Emulator bug. */
  KStdAction::home( SLOTS, SLOT( home() ), actionCollection(), "std_home" ); /*->setShortcut(Key_QuoteLeft);*/
  KAction *actRedisplay = KStdAction::redisplay( SLOTS, SLOT( refresh() ), actionCollection(), "std_redisplay" );
  actRedisplay->setText( "Reload" );
  actRedisplay->setShortcut( CTRL + Key_R );
  actShowToolBar = KStdAction::showToolbar( SLOTS, SLOT( toggleToolbar() ), actionCollection(), "std_toolbar" );
  actShowStatusBar = KStdAction::showStatusbar( SLOTS, SLOT( toggleStatusbar() ), actionCollection(), "std_statusbar" );
  KStdAction::quit( this, SLOT( quitKrusader() ), actionCollection(), "std_quit" );
  KStdAction::configureToolbars( SLOTS, SLOT( configToolbar() ), actionCollection(), "std_config_toolbar" );
  KStdAction::keyBindings( SLOTS, SLOT( configKeys() ), actionCollection(), "std_config_keys" );

  // the toggle actions
  actCompareDirs = new KToggleAction( i18n( "Com&pare Mode" ), "kr_comparedirs",
                                      CTRL + Key_Equal, SLOTS, SLOT( compareDirectories() ), actionCollection(), "compare mode" );
  actCompareDirs->setChecked( false );
  actToggleFnkeys = new KToggleAction( i18n( "Show &FN Keys Bar" ), 0, SLOTS,
                                       SLOT( toggleFnkeys() ), actionCollection(), "toggle fn bar" );
  actToggleFnkeys->setChecked( true );
  actToggleCmdline = new KToggleAction( i18n( "Show &Command Line" ), 0, SLOTS,
                                        SLOT( toggleCmdline() ), actionCollection(), "toggle command line" );
  actToggleCmdline->setChecked( true );
  actToggleTerminal = new KToggleAction( i18n( "Show &Terminal Emulator" ), 0, SLOTS,
                                         SLOT( toggleTerminal() ), actionCollection(), "toggle terminal emulator" );
  actToggleTerminal->setChecked( false );
  actToggleHidden = new KToggleAction( i18n( "Show &Hidden Files" ), CTRL + Key_Period, SLOTS,
                                       SLOT( toggleHidden() ), actionCollection(), "toggle hidden files" );
  actToggleSwapPanels = new KToggleAction( i18n( "S&wap Panels" ), CTRL + Key_U, SLOTS,
                                       SLOT( toggleSwapPanels() ), actionCollection(), "toggle swap panels" );
  krConfig->setGroup( "Look&Feel" );
  actToggleHidden->setChecked( krConfig->readBoolEntry( "Show Hidden", _ShowHidden ) );
  actToggleSwapPanels->setChecked( false );

  // and then the DONE actions
  actCmdlinePopup = new KAction( i18n("popup cmdline"), 0, CTRL + Key_Slash, SLOTS,
                     SLOT(cmdlinePopup()), actionCollection(), "cmdline popup");
  /* Shortcut disabled because of the Terminal Emulator bug. */
  actDirUp = new KAction( i18n("Up"), "up", 0 /*Key_Backspace*/, SLOTS, SLOT(dirUp()), actionCollection(), "dirUp");
  new KAction( i18n("&Edit new file"), "filenew", SHIFT + Key_F4, SLOTS, SLOT(editDlg()), actionCollection(), "edit_new_file");
  new KAction( i18n("Start &Root Mode Krusader"), "krusader", ALT + Key_K, SLOTS, SLOT(rootKrusader()), actionCollection(), "root krusader");

  actSelectColorMask = new KAction( i18n( "Co&nfigure compare-mode" ), 0,
                                    SLOTS, SLOT( selectCompareMask() ), actionCollection(), "select colormask" );
  actTest = new KAction( i18n( "&Test Archive(s)" ), "kr_arc_test", ALT + Key_T,
                         SLOTS, SLOT( testArchive() ), actionCollection(), "test archives" );
  actFTPConnect = new KAction( i18n( "&Net Connections" ), "socket", CTRL + Key_C,
                               SLOTS, SLOT( runRemoteMan() ), actionCollection(), "ftp connect" );
  actFTPNewConnect = new KAction( i18n( "New Net &Connection" ), "ftp", CTRL + Key_N,
                                  SLOTS, SLOT( newFTPconnection() ), actionCollection(), "ftp new connection" );
  actCalculate = new KAction( i18n( "Calculate &Occupied Space" ), "kr_calc", 0,
                              SLOTS, SLOT( calcSpace() ), actionCollection(), "calculate" );
  actProperties = new KAction( i18n( "&Properties" ), "kr_properties", 0,
                               SLOTS, SLOT( properties() ), actionCollection(), "properties" );
  actPack = new KAction( i18n( "Pac&k" ), "kr_arc_pack", ALT + Key_P,
                         SLOTS, SLOT( slotPack() ), actionCollection(), "pack" );
  actUnpack = new KAction( i18n( "&Unpack" ), "kr_arc_unpack", ALT + Key_U,
                           SLOTS, SLOT( slotUnpack() ), actionCollection() , "unpack" );
  actSplit = new KAction( i18n( "Sp&lit file" ), "kr_split", CTRL + Key_P,
                         SLOTS, SLOT( slotSplit() ), actionCollection(), "split" );
  actCombine = new KAction( i18n( "Com&bine files" ), "kr_combine", CTRL + Key_B,
                           SLOTS, SLOT( slotCombine() ), actionCollection() , "combine" );
  actSelect = new KAction( i18n( "Select &Group" ), "kr_select", CTRL + Key_Plus,
                           SLOTS, SLOT( markGroup() ), actionCollection(), "select group" );
  actSelectAll = new KAction( i18n( "&Select All" ), "kr_selectall", ALT + Key_Plus,
                              SLOTS, SLOT( markAll() ), actionCollection(), "select all" );
  actUnselectAll = new KAction( i18n( "U&nselect All" ), "kr_unselectall", ALT + Key_Minus,
                                SLOTS, SLOT( unmarkAll() ), actionCollection(), "unselect all" );
  actHomeTerminal = new KAction( i18n( "&Terminal" ), "kr_terminal", 0,
                                 SLOTS, SLOT( homeTerminal() ), actionCollection(), "terminal@home" );
  actFTPDisconnect = new KAction( i18n( "Disconnect &From Net" ), "kr_ftp_disconnect", SHIFT + CTRL + Key_F,
                                  SLOTS, SLOT( FTPDisconnect() ), actionCollection(), "ftp disconnect" );
  actMountMan = new KAction( i18n( "&MountMan" ), "kcmpartitions", ALT + Key_Slash,
                             SLOTS, SLOT( runMountMan() ), actionCollection(), "mountman" );
  actFind = new KAction( i18n( "&Search" ), "filefind", CTRL + Key_S,
                         SLOTS, SLOT( search() ), actionCollection(), "find" );
  actSyncDirs = new KAction( i18n( "Synchronize &Directories" ), "kr_syncdirs", CTRL + Key_D,
                         SLOTS, SLOT( slotSynchronizeDirs() ), actionCollection(), "sync dirs" );
  actInvert = new KAction( i18n( "&Invert Selection" ), "kr_invert", ALT + Key_Asterisk,
                           SLOTS, SLOT( invert() ), actionCollection(), "invert" );
  actUnselect = new KAction( i18n( "&Unselect Group" ), "kr_unselect", CTRL + Key_Minus,
                             SLOTS, SLOT( unmarkGroup() ), actionCollection(), "unselect group" );
  actKonfigurator = new KAction( i18n( "&Konfigurator" ), "configure", 0,
                                 SLOTS, SLOT( startKonfigurator() ), actionCollection(), "konfigurator" );
  actBack = new KAction( i18n( "Back" ), "back", 0,
                         SLOTS, SLOT( back() ), actionCollection(), "back" );
  actRoot = new KAction( i18n( "Root" ), "top", CTRL + Key_Backspace,
                         SLOTS, SLOT( root() ), actionCollection(), "root" );
  actSavePosition = new KAction( i18n( "Save &Position" ), 0,
                                 krApp, SLOT( savePosition() ), actionCollection(), "save position" );
  actAllFilter = new KAction( i18n( "&All Files" ), SHIFT + Key_F10,
                              SLOTS, SLOT( allFilter() ), actionCollection(), "all files" );
  //actExecFilter = new KAction( i18n( "&Executables" ), SHIFT + Key_F11,
  //                             SLOTS, SLOT( execFilter() ), actionCollection(), "exec files" );
  actCustomFilter = new KAction( i18n( "&Custom" ), SHIFT + Key_F12,
                                 SLOTS, SLOT( customFilter() ), actionCollection(), "custom files" );
  actCompare = new KAction( i18n( "Compare b&y content" ), "proxy", 0,
                            SLOTS, SLOT( compareContent() ), actionCollection(), "compare" );
  actMultiRename = new KAction( i18n( "Multi &Rename" ), "krename", SHIFT + Key_F9,
                                SLOTS, SLOT( multiRename() ), actionCollection(), "multirename" );
  new KAction( i18n( "Right-click menu" ), Key_Menu,
               SLOTS, SLOT( rightclickMenu() ), actionCollection(), "rightclick menu" );
  new KAction( i18n( "Right bookmarks" ), ALT+Key_Right,
               SLOTS, SLOT( openRightBookmarks() ), actionCollection(), "right bookmarks" );
  new KAction( i18n( "Left bookmarks" ), ALT+Key_Left,
               SLOTS, SLOT( openLeftBookmarks() ), actionCollection(), "left bookmarks" );
  actNewTab = new KAction(i18n("New tab"), ALT+CTRL+Key_N, SLOTS,
                          SLOT(newTab()), actionCollection(), "new tab");
  actDupTab = new KAction(i18n("Duplicate tab"), ALT+CTRL+SHIFT+Key_N, SLOTS,
                          SLOT(duplicateTab()), actionCollection(), "duplicate tab");
  actCloseTab = new KAction(i18n("Close tab"), ALT+CTRL+Key_C, SLOTS,
                          SLOT(closeTab()), actionCollection(), "close tab");
  actUserMenu = new KAction(i18n("User Menu"), ALT+CTRL+Key_U, SLOTS,
                          SLOT(userMenu()), actionCollection(), "user menu");
  // and at last we can set the tool-tips
  actSelect->setToolTip( i18n( "Highlight files by using a filter" ) );
  actSelectAll->setToolTip( i18n( "Highlight all the files in the current directory" ) );
  actUnselectAll->setToolTip( i18n( "Remove selection from all highlight files" ) );
  actMountMan->setToolTip( i18n( "Mount.Man: Krusader's mount-manager. Try it!" ) );
  actKonfigurator->setToolTip( i18n( "Setup Krusader the way you like it" ) );
  actBack->setToolTip( i18n( "Back to the place you came from" ) );
  actRoot->setToolTip( i18n( "ROOT (/)" ) );
  actFind->setToolTip( i18n( "Search for files" ) );
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////

void Krusader::savePosition() {
  config->setGroup( "Private" );
  config->writeEntry( "Start Position", isMaximized() ? oldPos : pos() );
  config->writeEntry( "Start Size", isMaximized() ? oldSize : size() );
  config->writeEntry( "Maximized", isMaximized() );
  config->writeEntry( "Panel Size", mainView->vert_splitter->sizes() [ 0 ] );
  config->writeEntry( "Terminal Size", mainView->vert_splitter->sizes() [ 1 ] );
  QValueList<int> lst = mainView->horiz_splitter->sizes();
  config->writeEntry( "Splitter Sizes", lst );
  // save view settings ---> fix when we have tabbed-browsing
  mainView->left->view->saveSettings();
  mainView->right->view->saveSettings();
  config->sync();
}

void Krusader::saveSettings() {
  toolBar() ->saveSettings( krConfig, "Private" );
  config->setGroup( "Startup" );
  bool panelsavesettings = config->readBoolEntry( "Panels Save Settings", _PanelsSave );
  bool rememberpos = config->readBoolEntry( "Remember Position", _RememberPos );
  bool uisavesettings = config->readBoolEntry( "UI Save Settings", _UiSave );
  if ( panelsavesettings ) {
    // left panel
    config->writeEntry( "Left Panel Type", i18n( "List" ) );
    config->writeEntry( "Left Panel Origin", i18n( "the last place it was" ) );
    // right panel
    config->writeEntry( "Right Panel Type", i18n( "List" ) );
    config->writeEntry( "Right Panel Origin", i18n( "the last place it was" ) );
  }

  config->writeEntry( "lastHomeLeft", mainView->left->realPath );
  config->writeEntry( "lastHomeRight", mainView->right->realPath );

  // save size and position
  if ( rememberpos || uisavesettings ) {
    savePosition();
  }

  // save the gui
  if ( uisavesettings ) {
    config->writeEntry( "Show status bar", actShowStatusBar->isChecked() );
    config->writeEntry( "Show tool bar", actShowToolBar->isChecked() );
    config->writeEntry( "Show FN Keys", actToggleFnkeys->isChecked() );
    config->writeEntry( "Show Cmd Line", actToggleCmdline->isChecked() );
    config->writeEntry( "Show Terminal Emulator", actToggleTerminal->isChecked() );
  }
  config->sync();
}

void Krusader::refreshView() {
  delete mainView;
  mainView = new KrusaderView( this );
  setCentralWidget( mainView );
  config->setGroup( "Private" );
  resize( krConfig->readSizeEntry( "Start Size", _StartSize ) );
  move( krConfig->readPointEntry( "Start Position", _StartPosition ) );
  mainView->show();
  show();
}

bool Krusader::queryClose() {
  bool quit = true;
  krConfig->setGroup( "Look&Feel" );
  if ( krConfig->readBoolEntry( "Warn On Exit", _WarnOnExit ) ) {
    switch ( KMessageBox::warningYesNo( this,
                                        i18n( "Ok to shutdown Krusader ?" ) ) ) {
        case KMessageBox::Yes :
        quit = true;
        break;
        case KMessageBox::No :
        quit = false;
        break;
        default:
        quit = false;
    }
  }
  if (quit) {
    // close all open VFS
    //delete krApp->mainView->left->func;
    //delete krApp->mainView->right->func;
    saveSettings();
    delete krApp->mainView;
    return true;
  } else return false;

}

void Krusader::quitKrusader() {
  if ( queryClose() ) {
    kapp->quit();
  }
}

// the please wait dialog functions
void Krusader::startWaiting( QString msg, int count , bool cancel ) {
  plzWait->startWaiting( msg , count, cancel );
}

void Krusader::incProgress( KProcess *, char *buffer, int buflen ) {
  int howMuch = 0;
  for ( int i = 0 ; i < buflen; ++i )
    if ( buffer[ i ] == '\n' )
      ++howMuch;

  plzWait->incProgress( howMuch );
}

void Krusader::stopWait() {
  plzWait->stopWait();
}

void Krusader::updateGUI( bool enforce ) {
  // now, check if we need to create a konsole_part
  config->setGroup( "Startup" );
  if ( config->readBoolEntry( "Show Terminal Emulator", _ShowTerminalEmulator ) ) {
    if ( enforce ) {
      mainView->slotTerminalEmulator( true ); // create konsole_part
      config->setGroup( "Private" );
      QValueList<int> lst;
      lst.append( config->readNumEntry( "Panel Size", _PanelSize ) );
      lst.append( config->readNumEntry( "Terminal Size", _TerminalSize ) );
      mainView->vert_splitter->setSizes( lst );
      config->setGroup( "Startup" );
    }
  }

  // call the XML GUI function to draw the UI
  createGUI( mainView->konsole_part );
  toolBar() ->applySettings( krConfig, "Private" );
  if ( enforce ) {
    // now, hide what need to be hidden
    if ( !krConfig->readBoolEntry( "Show tool bar", _ShowToolBar ) ) {
      toolBar() ->hide();
      actShowToolBar->setChecked( false );
    } else {
      toolBar() ->show();
      actShowToolBar->setChecked( true );
    }
    if ( !krConfig->readBoolEntry( "Show status bar", _ShowStatusBar ) ) {
      statusBar() ->hide();
      actShowStatusBar->setChecked( false );
    } else {
      statusBar() ->show();
      actShowStatusBar->setChecked( true );
    }
    if ( !krConfig->readBoolEntry( "Show Cmd Line", _ShowCmdline ) ) {
      mainView->cmdLine->hide();
      actToggleCmdline->setChecked( false );
    } else {
      mainView->cmdLine->show();
      actToggleCmdline->setChecked( true );
    }
    if ( !krConfig->readBoolEntry( "Show FN Keys", _ShowFNkeys ) ) {
      mainView->fnKeys->hide();
      actToggleFnkeys->setChecked( false );
    } else {
      mainView->fnKeys->show();
      actToggleFnkeys->setChecked( true );
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
  if ( KrServices::cmdExist( "kdiff" ) ) {
    tools.append( "DIFF" );
    tools.append( "kdiff" );
  } else if ( KrServices::cmdExist( "kompare" ) ) {
    tools.append( "DIFF" );
    tools.append( "kompare" );
  } else if ( KrServices::cmdExist( "xxdiff" ) ) {
    tools.append( "DIFF" );
    tools.append( "xxdiff" );
  }
  // a mailer: kmail
  if ( KrServices::cmdExist( "kmail" ) ) {
    tools.append( "MAIL" );
    tools.append( "kmail" );
  }
  // rename tool: krename
  if ( KrServices::cmdExist( "krename" ) ) {
    tools.append( "RENAME" );
    tools.append( "krename" );
  }

  return tools;
}

QString Krusader::getTempDir() {
  // try to make krusader temp dir
  krConfig->setGroup( "General" );
  QString tmpDir = krConfig->readEntry( "Temp Directory", _TempDirectory );
  for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
    QDir().mkdir( tmpDir.left( i ) );
  QDir().mkdir( tmpDir );
  chmod( tmpDir.local8Bit(), 0777 );
  // add a secure sub dir under the user UID
  QString uid;
  uid.sprintf( "%d", getuid() );
  QDir( tmpDir ).mkdir( uid );
  tmpDir = tmpDir + "/" + uid + "/";
  chmod( tmpDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR );
  // add a random sub dir to use
  while ( QDir().exists( tmpDir ) )
    tmpDir = tmpDir + kapp->randomString( 8 );
  QDir().mkdir( tmpDir );

  if ( !QDir( tmpDir ).isReadable() ) {
    KMessageBox::error( krApp, "Could not create a temporary directory. Handling of Archives will not be possible until this is fixed." );
    return QString::null;
  }
  return tmpDir;
}

QString Krusader::getTempFile() {
  // try to make krusader temp dir
  krConfig->setGroup( "General" );
  QString tmpDir = krConfig->readEntry( "Temp Directory", _TempDirectory );
  for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
    QDir().mkdir( tmpDir.left( i ) );
  QDir().mkdir( tmpDir );
  chmod( tmpDir.local8Bit(), 0777 );
  // add a secure sub dir under the user UID
  QString uid;
  uid.sprintf( "%d", getuid() );
  QDir( tmpDir ).mkdir( uid );
  tmpDir = tmpDir + "/" + uid + "/";
  chmod( tmpDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR );

  while ( QDir().exists( tmpDir ) )
    tmpDir = tmpDir + kapp->randomString( 8 );
  return tmpDir;
}


#include "krusader.moc"
