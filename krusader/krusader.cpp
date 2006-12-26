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
#include <sys/param.h>
#include <unistd.h>
#ifdef BSD
#include <sys/types.h>
#endif
// KDE includes
#include <kmessagebox.h>
#include <kaction.h>
#include <kcursor.h>
#include <ksystemtray.h>
#include <kmenubar.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kaccelmanager.h>
#include <kwin.h>

#if KDE_IS_VERSION(3,2,0)
#include <kactionclasses.h>
#endif

#include <kdeversion.h> 
// QT includes
#include <qpixmap.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qprinter.h>
#include <qprogressdialog.h>
#include <qvaluelist.h>
#include <qwhatsthis.h> 
#include <qwidgetlist.h>
#include <qdatetime.h>
#include <dcopclient.h>
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
#include "Konfigurator/kgprotocols.h"
#include "MountMan/kmountman.h"
#include "Panel/panelpopup.h"
#include "Queue/queue_mgr.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "krslots.h"
#include "krservices.h"
#include "UserAction/useraction.h"
// This makes gcc-4.1 happy. Warning about possible problem with KrAction's dtor not called
#include "UserAction/kraction.h"
#include "UserAction/expander.h"
#include "UserMenu/usermenu.h"
#include "panelmanager.h"
#include "MountMan/kmountman.h"
#include "BookMan/krbookmarkhandler.h"
#include "Dialogs/popularurls.h"
#include "GUI/krremoteencodingmenu.h"
#include "Dialogs/checksumdlg.h"

#ifdef __KJSEMBED__
#include "KrJS/krjs.h"
#endif

// define the static members
Krusader *Krusader::App = 0;
KAction *Krusader::actProperties = 0;
KAction *Krusader::actPack = 0;
KAction *Krusader::actUnpack = 0;
KAction *Krusader::actTest = 0;
KAction *Krusader::actCompare = 0;
KAction *Krusader::actCalculate = 0;
KAction *Krusader::actCreateChecksum = 0;
KAction *Krusader::actMatchChecksum = 0;
KAction *Krusader::actSelect = 0;
KAction *Krusader::actSelectAll = 0;
KAction *Krusader::actUnselect = 0;
KAction *Krusader::actUnselectAll = 0;
KAction *Krusader::actInvert = 0;
KAction *Krusader::actCompDirs = 0;
KAction *Krusader::actSync = 0;
KAction *Krusader::actDiskUsage = 0;
KAction *Krusader::actHomeTerminal = 0;
KAction *Krusader::actFTPConnect = 0;
KAction *Krusader::actFTPNewConnect = 0;
KAction *Krusader::actFTPDisconnect = 0;
KAction *Krusader::actProfiles = 0;
KAction *Krusader::actMultiRename = 0;
KAction *Krusader::actAllFilter = 0;
KAction *Krusader::actExecFilter = 0;
KAction *Krusader::actCustomFilter = 0;
KAction *Krusader::actMountMan = 0;
KAction *Krusader::actNewTool = 0;
KAction *Krusader::actKonfigurator = 0;
KAction *Krusader::actToolsSetup = 0;
KAction *Krusader::actSwapPanels = 0;
KAction *Krusader::actSwapSides = 0;
KAction *Krusader::actBack = 0;
KAction *Krusader::actRoot = 0;
KAction *Krusader::actFind = 0;
KAction *Krusader::actLocate = 0;
KAction *Krusader::actSwitchFullScreenTE = 0;
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
KAction *Krusader::actNextTab = 0;
KAction *Krusader::actPreviousTab = 0;
KAction *Krusader::actSplit = 0;
KAction *Krusader::actCombine = 0;
KAction *Krusader::actUserMenu = 0;
KAction *Krusader::actManageUseractions = 0;
KAction *Krusader::actSyncDirs = 0;
KAction *Krusader::actSyncBrowse = 0;
KAction *Krusader::actF2 = 0;
KAction *Krusader::actF3 = 0;
KAction *Krusader::actF4 = 0;
KAction *Krusader::actF5 = 0;
KAction *Krusader::actF6 = 0;
KAction *Krusader::actF7 = 0;
KAction *Krusader::actF8 = 0;
KAction *Krusader::actF9 = 0;
KAction *Krusader::actF10 = 0;
KAction *Krusader::actLocationBar = 0;
KAction *Krusader::actPopularUrls = 0;
KAction *Krusader::actJumpBack = 0;
KAction *Krusader::actSetJumpBack = 0;
KToggleAction *Krusader::actToggleTerminal = 0;
KToggleAction *Krusader::actVerticalMode = 0;
KRadioAction  *Krusader::actSelectNewerAndSingle = 0;
KRadioAction  *Krusader::actSelectSingle = 0;
KRadioAction  *Krusader::actSelectNewer = 0;
KRadioAction  *Krusader::actSelectDifferentAndSingle = 0;
KRadioAction  *Krusader::actSelectDifferent = 0;
KRadioAction  **Krusader::compareArray[] = {&actSelectNewerAndSingle, &actSelectNewer, &actSelectSingle, 
                                            &actSelectDifferentAndSingle, &actSelectDifferent, 0};
KPopupMenu *Krusader::userActionMenu = 0;
UserAction *Krusader::userAction = 0;
UserMenu *Krusader::userMenu = 0;
KrBookmarkHandler *Krusader::bookman = 0;
//QTextOStream *Krusader::_krOut = QTextOStream(::stdout);

#ifdef __KJSEMBED__
KrJS *Krusader::js = 0;
KAction *Krusader::actShowJSConsole = 0;
#endif

// construct the views, statusbar and menu bars and prepare Krusader to start
Krusader::Krusader() : KParts::MainWindow(0,0,WType_TopLevel|WDestructiveClose|Qt::WStyle_ContextHelp),
   DCOPObject("Krusader-Interface"), status(NULL), sysTray( 0 ), isStarting( true ), isExiting( false ), directExit( false ) {
   // parse command line arguments
   KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

   kapp->ref(); // FIX: krusader exits at closing the viewer when minimized to tray

   // create the "krusader"
   App = this;
   slot = new KRslots(this);
   setXMLFile( "krusaderui.rc" ); // kpart-related xml file

   plzWait = new KRPleaseWaitHandler();

   bool runKonfig = versionControl();

   QString message;
   switch ( config->getConfigState() ) {
         case KConfigBase::NoAccess :
         message = "Krusader's configuration file can't be found. Default values will be used.";
         break;
         case KConfigBase::ReadOnly :
         message = "Krusader's configuration file is in READ ONLY mode (why is that!?) Changed values will not be saved";
         break;
         case KConfigBase::ReadWrite :
         message = "";
         break;
   }
   if ( message != "" ) {
      KMessageBox::error( krApp, message );
   }

   // create an icon loader
   iconLoader = KGlobal::iconLoader();

   // create MountMan
   mountMan = new KMountMan();

   // create bookman
   bookman = new KrBookmarkHandler();

   popularUrls = new PopularUrls(this);

   queueManager = new QueueManager();

   // create the main view
   mainView = new KrusaderView( this );

   // setup all the krusader's actions
   setupActions();

   // init the permmision handler class
   KRpermHandler::init();

   // init the protocol handler
   KgProtocols::init();

   // init the checksum tools
   initChecksumModule();

   krConfig->setGroup( "Startup" );
   QStringList leftTabs = krConfig->readPathListEntry( "Left Tab Bar" );
   QStringList rightTabs = krConfig->readPathListEntry( "Right Tab Bar" );
   int         leftActiveTab = krConfig->readNumEntry( "Left Active Tab", 0 );
   int         rightActiveTab = krConfig->readNumEntry( "Right Active Tab", 0 );
   QString     startProfile = krConfig->readEntry("Starter Profile Name", QString::null );

   // get command-line arguments
   if ( args->isSet( "left" ) ) {
      leftTabs = QStringList::split( ',', args->getOption( "left" ) );

      leftActiveTab = 0;

      // make sure left or right are not relative paths
      for(unsigned int i = 0; i != leftTabs.count(); i++ )
      {
        leftTabs[ i ] = leftTabs[ i ].stripWhiteSpace();
        if( !leftTabs[ i ].startsWith( "/" ) && leftTabs[ i ].find( ":/" ) < 0 )
          leftTabs[ i ] = QDir::currentDirPath() + "/" + leftTabs[ i ];
      }
      startProfile = QString::null;
   }
   if ( args->isSet( "right" ) ) {
      rightTabs = QStringList::split( ',', args->getOption( "right" ) );

      rightActiveTab = 0;

      // make sure left or right are not relative paths
      for(unsigned int i = 0; i != rightTabs.count(); i++ )
      {
        rightTabs[ i ] = rightTabs[ i ].stripWhiteSpace();
        if( !rightTabs[ i ].startsWith( "/" ) && rightTabs[ i ].find( ":/" ) < 0 )
          rightTabs[ i ] = QDir::currentDirPath() + "/" + rightTabs[ i ];
      }
      startProfile = QString::null;
   }

   if ( args->isSet( "profile" ) )
    startProfile = args->getOption( "profile" );

   if( !startProfile.isEmpty() ) {
      leftTabs.clear();
      rightTabs.clear();      
      leftActiveTab = rightActiveTab = 0;
   }

   if( leftTabs.count() == 0 )
     leftTabs.push_back( QDir::homeDirPath() );
   if( rightTabs.count() == 0 )
     rightTabs.push_back( QDir::homeDirPath() );

   // starting the panels
   mainView->start( leftTabs, leftActiveTab, rightTabs, rightActiveTab );

   // create the user menu
   userMenu = new UserMenu( this );
   userMenu->hide();

   // setup keyboard accelerators
   setupAccels();

   // create a status bar
   status = new KrusaderStatus( this );
   QWhatsThis::add( status, i18n( "Statusbar will show basic information "
                                          "about file below mouse pointer." ) );

   // This enables Krusader to show a tray icon
   sysTray = new KSystemTray( this );
   // Krusader::privIcon() returns either "krusader_blue" or "krusader_red" if the user got root-privileges
   sysTray->setPixmap( iconLoader->loadIcon( privIcon(), KIcon::Panel, 22 ) );
   sysTray->hide();

   connect( sysTray, SIGNAL( quitSelected() ), this, SLOT( setDirectExit() ) );

   setCentralWidget( mainView );
   config->setGroup( "Startup" );
   bool startToTray = config->readBoolEntry( "Start To Tray", _StartToTray );
   config->setGroup( "Look&Feel" );
   bool minimizeToTray = config->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = config->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );

   startToTray = startToTray && minimizeToTray;

   if( singleInstanceMode && minimizeToTray )
     sysTray->show();


   // manage our keyboard short-cuts
   //KAcceleratorManager::manage(this,true);

   setCursor( KCursor::arrowCursor() );

   if ( ! startProfile.isEmpty() )
       mainView->profiles( startProfile );

   if (!runKonfig) {
		config->setGroup( "Private" );
		if ( krConfig->readBoolEntry( "Maximized" ) )
			restoreWindowSize(config);
		else {
			move( oldPos = krConfig->readPointEntry( "Start Position", _StartPosition ) );
			resize( oldSize = krConfig->readSizeEntry( "Start Size", _StartSize ));
		}
			
		if( startToTray ) {
			sysTray->show();
			hide();
		}
		else
			show();
	}
   // let the good times rool :)
   updateGUI( true );

   if ( runKonfig )
      slot->runKonfigurator( true );

   isStarting = false;
}

Krusader::~Krusader() {
   if( !isExiting )   // save the settings if it was not saved (SIGTERM)
      saveSettings();

   delete mainView;
   delete queueManager;
   mainView = 0;
   App = 0;
}

bool Krusader::versionControl() {
#define FIRST_RUN	"First Time"
   bool retval = false;
   // create config file
   config = kapp->config();
   bool firstRun = config->readBoolEntry(FIRST_RUN, true);

#if 0      
	QString oldVerText = config->readEntry( "Version", "10.0" );
   oldVerText.truncate( oldVerText.findRev( "." ) ); // remove the third dot
   float oldVer = oldVerText.toFloat();
   // older icompatible version
   if ( oldVer <= 0.9 ) {
      KMessageBox::information( krApp, i18n( "A configuration of 1.51 or older was detected. Krusader has to reset your configuration to default values.\nNote: Your bookmarks and keybindings will remain intact.\n Krusader will now run Konfigurator." ) );
      /*if ( !QDir::home().remove( ".kde/share/config/krusaderrc" ) ) {
         KMessageBox::error( krApp, i18n( "Unable to remove your krusaderrc file! Please remove it manually and rerun Krusader." ) );
         exit( 1 );
      }*/
      retval = true;
      config->reparseConfiguration();
   }
#endif

   // first installation of krusader
   if ( firstRun ) {
      KMessageBox::information( krApp, i18n( "<qt><b>Welcome to Krusader!</b><p>As this is your first run, your machine will now be checked for external applications. Then the Konfigurator will be launched where you can customize Krusader to your needs.</p></qt>" ) );
      retval = true;
   }
   config->writeEntry( "Version", VERSION );
   config->writeEntry( FIRST_RUN, false);
   config->sync();
   return retval;
}

void Krusader::statusBarUpdate( QString& mess ) {
   // change the message on the statusbar for 2 seconds
   if (status) // ugly!!!! But as statusBar() creates a status bar if there is no, I have to ask status to prevent 
               // the creation of the KDE default status bar instead of KrusaderStatus.
      statusBar() ->message( mess, 5000 );
}

void Krusader::showEvent ( QShowEvent * ) {
   if( isExiting )
     return;
   config->setGroup( "Look&Feel" );
   bool showTrayIcon = krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = krConfig->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );
   
   if( showTrayIcon && !singleInstanceMode )
     sysTray->hide();
   show(); // needed to make sure krusader is removed from
   // the taskbar when minimizing (system tray issue)
}

void Krusader::hideEvent ( QHideEvent *e ) {
   if( isExiting ) {
     KParts::MainWindow::hideEvent( e );
     sysTray->hide();
     return;
   }
   QString lastGroup = config->group();
   config->setGroup( "Look&Feel" );
   bool showTrayIcon = krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   config->setGroup ( lastGroup );

   bool isModalTopWidget = false;

   QWidget *actWnd = qApp->activeWindow();
   if ( actWnd )
      isModalTopWidget = actWnd->isModal();

   if ( showTrayIcon  && !isModalTopWidget  && KWin::windowInfo( winId() ).isOnCurrentDesktop() ) {
      sysTray->show();
      hide(); // needed to make sure krusader is removed from
      // the taskbar when minimizing (system tray issue)
   } else KParts::MainWindow::hideEvent( e );
}

void Krusader::moveEvent ( QMoveEvent *e ) {
   oldPos = e->oldPos();
   KParts::MainWindow::moveEvent( e );
}

void Krusader::resizeEvent ( QResizeEvent *e ) {
   oldSize = e->oldSize();
   KParts::MainWindow::resizeEvent( e );
}

void Krusader::setupAccels() {
	 accels = new KAccel( this );
	 // SHIFT+F3
   accels->insert( "F3_ViewDlg", i18n( "F3 View Dialog" ), QString::null,
                   SHIFT + Key_F3, SLOTS, SLOT( viewDlg() ) );
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
   //KStdAction::showMenubar( SLOTS, SLOT( showMenubar() ), actionCollection(), "std_menubar" );


   // second, the KDE standard action
   //KStdAction::up( SLOTS, SLOT( dirUp() ), actionCollection(), "std_up" )->setShortcut(Key_Backspace);
   /* Shortcut disabled because of the Terminal Emulator bug. */
   krConfig->setGroup( "Private" );
   int compareMode = krConfig->readNumEntry( "Compare Mode", 0 );

   KStdAction::home( SLOTS, SLOT( home() ), actionCollection(), "std_home" )->setText( i18n("Home") ); /*->setShortcut(Key_QuoteLeft);*/
   new KAction( i18n( "&Reload" ), "reload", CTRL + Key_R, SLOTS, SLOT( refresh() ), actionCollection(), "std_redisplay" );
   actShowToolBar = KStdAction::showToolbar( SLOTS, SLOT( toggleToolbar() ), actionCollection(), "std_toolbar" );
   new KToggleAction( i18n("Show Actions Toolbar"), 0, SLOTS, SLOT( toggleActionsToolbar() ),
                      actionCollection(), "toggle actions toolbar" );
   actShowStatusBar = KStdAction::showStatusbar( SLOTS, SLOT( toggleStatusbar() ), actionCollection(), "std_statusbar" );
   KStdAction::quit( this, SLOT( slotClose() ), actionCollection(), "std_quit" );
   KStdAction::configureToolbars( SLOTS, SLOT( configToolbar() ), actionCollection(), "std_config_toolbar" );
   KStdAction::keyBindings( SLOTS, SLOT( configKeys() ), actionCollection(), "std_config_keys" );

   KStdAction::cut( SLOTS, SLOT( cut() ), actionCollection(), "std_cut" )->setText( i18n("Cut to Clipboard") );
   KStdAction::copy( SLOTS, SLOT( copy() ), actionCollection(), "std_copy" )->setText( i18n("Copy to Clipboard") );
   KStdAction::paste( SLOTS, SLOT( paste() ), actionCollection(), "std_paste" )->setText( i18n("Paste from Clipboard") );

   // the toggle actions
   actToggleFnkeys = new KToggleAction( i18n( "Show &FN Keys Bar" ), 0, SLOTS,
                                        SLOT( toggleFnkeys() ), actionCollection(), "toggle fn bar" );
   actToggleFnkeys->setChecked( true );
   actToggleCmdline = new KToggleAction( i18n( "Show &Command Line" ), 0, SLOTS,
                                         SLOT( toggleCmdline() ), actionCollection(), "toggle command line" );
   actToggleCmdline->setChecked( true );
   actToggleTerminal = new KToggleAction( i18n( "Show Terminal &Emulator" ), ALT + CTRL + Key_T, SLOTS,
                                          SLOT( toggleTerminal() ), actionCollection(), "toggle terminal emulator" );
   actToggleTerminal->setChecked( false );
   actToggleHidden = new KToggleAction( i18n( "Show &Hidden Files" ), CTRL + Key_Period, SLOTS,
                                        SLOT( toggleHidden() ), actionCollection(), "toggle hidden files" );
   actSwapPanels = new KAction( i18n( "S&wap Panels" ), CTRL + Key_U, SLOTS,
                                SLOT( swapPanels() ), actionCollection(), "swap panels" );
   actSwapSides = new KAction( i18n( "Sw&ap Sides" ), CTRL + SHIFT + Key_U, SLOTS,
                                SLOT( toggleSwapSides() ), actionCollection(), "toggle swap sides" );
   krConfig->setGroup( "Look&Feel" );
   actToggleHidden->setChecked( krConfig->readBoolEntry( "Show Hidden", _ShowHidden ) );

   // and then the DONE actions
   actCmdlinePopup = new KAction( i18n( "popup cmdline" ), 0, CTRL + Key_Slash, SLOTS,
                                  SLOT( cmdlinePopup() ), actionCollection(), "cmdline popup" );
   /* Shortcut disabled because of the Terminal Emulator bug. */
   actDirUp = new KAction( i18n( "Up" ), "up", CTRL+Key_PageUp /*Key_Backspace*/, SLOTS, SLOT( dirUp() ), actionCollection(), "dirUp" );
   new KAction( i18n( "&New Text File..." ), "filenew", SHIFT + Key_F4, SLOTS, SLOT( editDlg() ), actionCollection(), "edit_new_file" );
   new KAction( i18n( "Start &Root Mode Krusader" ), "krusader_root", ALT + Key_K, SLOTS, SLOT( rootKrusader() ), actionCollection(), "root krusader" );

   actTest = new KAction( i18n( "T&est Archive" ), "ark", ALT + Key_E,
                          SLOTS, SLOT( testArchive() ), actionCollection(), "test archives" );
   //actFTPConnect = new KAction( i18n( "&Net Connections" ), "domtreeviewer", 0,
   //                             SLOTS, SLOT( runRemoteMan() ), actionCollection(), "ftp connect" );
   actFTPNewConnect = new KAction( i18n( "New Net &Connection..." ), "connect_creating", CTRL + Key_N,
                                   SLOTS, SLOT( newFTPconnection() ), actionCollection(), "ftp new connection" );
   actProfiles = new KAction( i18n( "Pro&files" ), "kr_profile", ALT + Key_L,
                                   MAIN_VIEW, SLOT( profiles() ), actionCollection(), "profile" );
   actCalculate = new KAction( i18n( "Calculate &Occupied Space" ), "kcalc", 0,
                               SLOTS, SLOT( calcSpace() ), actionCollection(), "calculate" );
   actCreateChecksum = new KAction( i18n( "Create Checksum..." ), "binary", 0,
                               SLOTS, SLOT( createChecksum() ), actionCollection(), "create checksum" );
   actMatchChecksum = new KAction( i18n( "Verify Checksum..." ), "match_checksum", 0,
                               SLOTS, SLOT( matchChecksum() ), actionCollection(), "match checksum" );
   actProperties = new KAction( i18n( "&Properties..." ), 0, ALT+Key_Enter,
                                SLOTS, SLOT( properties() ), actionCollection(), "properties" );
   actPack = new KAction( i18n( "Pac&k..." ), "kr_arc_pack", ALT + Key_P,
                          SLOTS, SLOT( slotPack() ), actionCollection(), "pack" );
   actUnpack = new KAction( i18n( "&Unpack..." ), "kr_arc_unpack", ALT + Key_U,
                            SLOTS, SLOT( slotUnpack() ), actionCollection() , "unpack" );
   actSplit = new KAction( i18n( "Sp&lit File..." ), "kr_split", CTRL + Key_P,
                           SLOTS, SLOT( slotSplit() ), actionCollection(), "split" );
   actCombine = new KAction( i18n( "Com&bine Files..." ), "kr_combine", CTRL + Key_B,
                             SLOTS, SLOT( slotCombine() ), actionCollection() , "combine" );
   actSelect = new KAction( i18n( "Select &Group..." ), "kr_select", CTRL + Key_Plus,
                            SLOTS, SLOT( markGroup() ), actionCollection(), "select group" );
   actSelectAll = new KAction( i18n( "&Select All" ), "kr_selectall", ALT + Key_Plus,
                               SLOTS, SLOT( markAll() ), actionCollection(), "select all" );
   actUnselect = new KAction( i18n( "&Unselect Group..." ), "kr_unselect", CTRL + Key_Minus,
                              SLOTS, SLOT( unmarkGroup() ), actionCollection(), "unselect group" );
   actUnselectAll = new KAction( i18n( "U&nselect All" ), "kr_unselectall", ALT + Key_Minus,
                                 SLOTS, SLOT( unmarkAll() ), actionCollection(), "unselect all" );
   actInvert = new KAction( i18n( "&Invert Selection" ), "kr_invert", ALT + Key_Asterisk,
                            SLOTS, SLOT( invert() ), actionCollection(), "invert" );
   actCompDirs = new KAction( i18n( "&Compare Directories" ), "view_left_right", 0,
                              SLOTS, SLOT( compareDirs() ), actionCollection(), "compare dirs" );
   actSelectNewerAndSingle = new KRadioAction( i18n( "&Select Newer and Single" ), 0,
                                 SLOTS, SLOT( compareSetup() ), actionCollection(), "select_newer_and_single" );
   actSelectNewer = new KRadioAction( i18n( "Select &Newer" ), 0,
                                 SLOTS, SLOT( compareSetup() ), actionCollection(), "select_newer" );
   actSelectSingle = new KRadioAction( i18n( "Select &Single" ), 0,
                                 SLOTS, SLOT( compareSetup() ), actionCollection(), "select_single" );
   actSelectDifferentAndSingle = new KRadioAction( i18n( "Select Different &and Single" ), 0,
                                 SLOTS, SLOT( compareSetup() ), actionCollection(), "select_different_and_single" );
   actSelectDifferent = new KRadioAction( i18n( "Select &Different" ), 0,
                                 SLOTS, SLOT( compareSetup() ), actionCollection(), "select_different" );
   actSelectNewerAndSingle->setExclusiveGroup( "the_select_group" );
   actSelectNewer->setExclusiveGroup( "the_select_group" );
   actSelectSingle->setExclusiveGroup( "the_select_group" );
   actSelectDifferentAndSingle->setExclusiveGroup( "the_select_group" );
   actSelectDifferent->setExclusiveGroup( "the_select_group" );
   if( compareMode < (int)( sizeof( compareArray ) / sizeof( KRadioAction ** ) ) -1 )
     (*compareArray[ compareMode ])->setChecked( true );
   actHomeTerminal = new KAction( i18n( "Start &Terminal" ), "terminal", 0,
                                  SLOTS, SLOT( homeTerminal() ), actionCollection(), "terminal@home" );
   actFTPDisconnect = new KAction( i18n( "Disconnect &from Net" ), "kr_ftp_disconnect", SHIFT + CTRL + Key_F,
                                   SLOTS, SLOT( FTPDisconnect() ), actionCollection(), "ftp disconnect" );
#if KDE_IS_VERSION(3,2,0)	/* new mountman feature is available in kde 3.2 only! */
   actMountMan = new KToolBarPopupAction( i18n( "&MountMan..." ), "kr_mountman", ALT + Key_Slash,
                                          SLOTS, SLOT( runMountMan() ), actionCollection(), "mountman" );
   connect( ( ( KToolBarPopupAction* ) actMountMan ) ->popupMenu(), SIGNAL( aboutToShow() ),
            mountMan, SLOT( quickList() ) );
#else
   actMountMan = new KAction( i18n( "&MountMan..." ), "kr_mountman", ALT + Key_Slash,
                              SLOTS, SLOT( runMountMan() ), actionCollection(), "mountman" );
#endif /* KDE 3.2 */

   actFind = new KAction( i18n( "&Search..." ), "filefind", CTRL + Key_S,
                          SLOTS, SLOT( search() ), actionCollection(), "find" );
   actLocate = new KAction( i18n( "&Locate..." ), "find", SHIFT+CTRL + Key_L,
                            SLOTS, SLOT( locate() ), actionCollection(), "locate" );
   actSyncDirs = new KAction( i18n( "Synchronize &Directories..." ), "kr_syncdirs", CTRL + Key_Y,
                              SLOTS, SLOT( slotSynchronizeDirs() ), actionCollection(), "sync dirs" );
   actSyncBrowse = new KAction( i18n( "S&ynchron Directory Changes" ), "kr_syncbrowse_off", ALT + Key_Y,
                              SLOTS, SLOT( slotSyncBrowse() ), actionCollection(), "sync browse" );
   actDiskUsage = new KAction( i18n( "D&isk Usage..." ), "kchart", ALT + Key_D,
                              SLOTS, SLOT( slotDiskUsage() ), actionCollection(), "disk usage" );
   actKonfigurator = new KAction( i18n( "Configure &Krusader..." ), "configure", 0,
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
   actCompare = new KAction( i18n( "Compare b&y Content..." ), "kmultiple", 0,
                             SLOTS, SLOT( compareContent() ), actionCollection(), "compare" );
   actMultiRename = new KAction( i18n( "Multi &Rename..." ), "krename", SHIFT + Key_F9,
                                 SLOTS, SLOT( multiRename() ), actionCollection(), "multirename" );
   new KAction( i18n( "Right-click Menu" ), Key_Menu,
                SLOTS, SLOT( rightclickMenu() ), actionCollection(), "rightclick menu" );
   new KAction( i18n( "Right Bookmarks" ), ALT + Key_Right,
                SLOTS, SLOT( openRightBookmarks() ), actionCollection(), "right bookmarks" );
   new KAction( i18n( "Left Bookmarks" ), ALT + Key_Left,
                SLOTS, SLOT( openLeftBookmarks() ), actionCollection(), "left bookmarks" );
   new KAction( i18n( "Bookmarks" ), CTRL + Key_D,
                SLOTS, SLOT( openBookmarks() ), actionCollection(), "bookmarks" );
   new KAction( i18n( "Bookmark Current" ), CTRL + SHIFT + Key_D,
                SLOTS, SLOT( bookmarkCurrent() ), actionCollection(), "bookmark current" );
   new KAction( i18n( "History" ), CTRL + Key_H,
                SLOTS, SLOT( openHistory() ), actionCollection(), "history" );
   new KAction( i18n( "Sync Panels" ), ALT + Key_O,
                SLOTS, SLOT( syncPanels() ), actionCollection(), "sync panels");
   new KAction( i18n( "Left History" ), ALT + CTRL + Key_Left,
                SLOTS, SLOT( openLeftHistory() ), actionCollection(), "left history" );
   new KAction( i18n( "Right History" ), ALT + CTRL + Key_Right,
                SLOTS, SLOT( openRightHistory() ), actionCollection(), "right history" );
   new KAction( i18n( "Media" ), CTRL + Key_M,
                SLOTS, SLOT( openMedia() ), actionCollection(), "media" );
   new KAction( i18n( "Left Media" ), CTRL + SHIFT + Key_Left,
                SLOTS, SLOT( openLeftMedia() ), actionCollection(), "left media" );
   new KAction( i18n( "Right Media" ), CTRL + SHIFT + Key_Right,
                SLOTS, SLOT( openRightMedia() ), actionCollection(), "right media" );
   new KAction( i18n( "New Symlink..." ), CTRL + ALT + Key_S,
                SLOTS, SLOT( newSymlink() ), actionCollection(), "new symlink");
   new KToggleAction( i18n( "Toggle Popup Panel" ), ALT + Key_Down, SLOTS,
                            SLOT( togglePopupPanel() ), actionCollection(), "toggle popup panel" );
   actVerticalMode = new KToggleAction( i18n( "Vertical Mode" ), "view_top_bottom", ALT + CTRL + Key_R, MAIN_VIEW, 
                                        SLOT( toggleVerticalMode() ), actionCollection(), "toggle vertical mode" );
   actNewTab = new KAction( i18n( "New Tab" ), "tab_new", ALT + CTRL + Key_N, SLOTS,
                            SLOT( newTab() ), actionCollection(), "new tab" );
   actDupTab = new KAction( i18n( "Duplicate Current Tab" ), "tab_duplicate", ALT + CTRL + SHIFT + Key_N, SLOTS,
                            SLOT( duplicateTab() ), actionCollection(), "duplicate tab" );
   actCloseTab = new KAction( i18n( "Close Current Tab" ), "tab_remove", CTRL + Key_W, SLOTS,
                              SLOT( closeTab() ), actionCollection(), "close tab" );
   actNextTab  = new KAction( i18n( "Next Tab" ), SHIFT + Key_Right, SLOTS,
                              SLOT( nextTab() ), actionCollection(), "next tab" );
   actPreviousTab  = new KAction( i18n( "Previous Tab" ), SHIFT + Key_Left, SLOTS,
                                  SLOT( previousTab() ), actionCollection(), "previous tab" );
/*
   actUserMenu = new KAction( i18n( "User Menu" ), ALT + Key_QuoteLeft, SLOTS,
                              SLOT( userMenu() ), actionCollection(), "user menu" );
*/
   actManageUseractions = new KAction( i18n( "Manage User Actions..." ), 0, SLOTS,
                              SLOT( manageUseractions() ), actionCollection(), "manage useractions" );
   new KrRemoteEncodingMenu(i18n("Select Remote Charset"), "charset", actionCollection(), "changeremoteencoding");

   // setup the Fn keys
   actF2 = new KAction( i18n( "Start Terminal Here" ), "terminal", Key_F2,
                        SLOTS, SLOT( terminal() ) , actionCollection(), "F2_Terminal" );
   actF3 = new KAction( i18n( "View File" ), Key_F3,
                        SLOTS, SLOT( view() ) , actionCollection(), "F3_View" );
   actF4 = new KAction( i18n( "Edit File" ), Key_F4,
                        SLOTS, SLOT( edit() ) , actionCollection(), "F4_Edit" );
   actF5 = new KAction( i18n( "Copy..." ), Key_F5,
                        SLOTS, SLOT( copyFiles() ) , actionCollection(), "F5_Copy" );
   actF6 = new KAction( i18n( "Move..." ), Key_F6,
                        SLOTS, SLOT( moveFiles() ) , actionCollection(), "F6_Move" );
   actF7 = new KAction( i18n( "New Directory..." ), "folder_new", Key_F7,
                        SLOTS, SLOT( mkdir() ) , actionCollection(), "F7_Mkdir" );
   actF8 = new KAction( i18n( "Delete" ), "editdelete", Key_F8,
                        SLOTS, SLOT( deleteFiles() ) , actionCollection(), "F8_Delete" );
   actF9 = new KAction( i18n( "Rename" ), Key_F9,
                        SLOTS, SLOT( rename() ) , actionCollection(), "F9_Rename" );
   actF10 = new KAction( i18n( "Quit" ), Key_F10,
                         this, SLOT( slotClose() ) , actionCollection(), "F10_Quit" );
   actPopularUrls = new KAction( i18n("Popular URLs..."), CTRL+Key_Z,
                                 popularUrls, SLOT( showDialog() ), actionCollection(), "Popular_Urls");
   actLocationBar = new KAction( i18n("Go to Location Bar"), CTRL+Key_L,
                                 SLOTS, SLOT( slotLocationBar() ), actionCollection(), "location_bar");
   actJumpBack = new KAction( i18n("Jump Back"), "kr_jumpback", CTRL+Key_J,
                              SLOTS, SLOT( slotJumpBack() ), actionCollection(), "jump_back");
   actSetJumpBack = new KAction( i18n("Set Jump Back Point"), "kr_setjumpback", CTRL+SHIFT+Key_J,
                                 SLOTS, SLOT( slotSetJumpBack() ), actionCollection(), "set_jump_back");
   actSwitchFullScreenTE = new KAction( i18n( "Toggle Fullwidget Terminal Emulator" ), 0, CTRL + Key_F,
                                        MAIN_VIEW, SLOT( switchFullScreenTE() ), actionCollection(), "switch_fullscreen_te" );

   // and at last we can set the tool-tips
   actSelect->setToolTip( i18n( "Select files using a filter" ) );
   actSelectAll->setToolTip( i18n("Select all files in the current directory" ) );
   actUnselectAll->setToolTip( i18n( "Unselect all selected files" ) );
   actKonfigurator->setToolTip( i18n( "Setup Krusader the way you like it" ) );
   actBack->setToolTip( i18n( "Back to the place you came from" ) );
   actRoot->setToolTip( i18n( "ROOT (/)" ) );
   actFind->setToolTip( i18n( "Search for files" ) );

   // setup all UserActions
   userAction = new UserAction();

   #ifdef __KJSEMBED__
   actShowJSConsole = new KAction( i18n( "JavaScript Console..." ), ALT + CTRL + Key_J, SLOTS, SLOT( jsConsole() ), actionCollection(), "JS_Console" );
   #endif
}

///////////////////////////////////////////////////////////////////////////
//////////////////// implementation of slots //////////////////////////////
///////////////////////////////////////////////////////////////////////////

void Krusader::savePosition() {
   config->setGroup( "Private" );
   config->writeEntry( "Maximized", isMaximized() );
   if (isMaximized())
      saveWindowSize(config);
   else {
      config->writeEntry( "Start Position", isMaximized() ? oldPos : pos() );
      config->writeEntry( "Start Size", isMaximized() ? oldSize : size() );
   }
   config->writeEntry( "Panel Size", mainView->vert_splitter->sizes() [ 0 ] );
   config->writeEntry( "Terminal Size", mainView->vert_splitter->sizes() [ 1 ] );
   QValueList<int> lst = mainView->horiz_splitter->sizes();
   config->writeEntry( "Splitter Sizes", lst );
   mainView->left->popup->saveSizes();
   mainView->right->popup->saveSizes();
   if( !MAIN_VIEW->getTerminalEmulatorSplitterSizes().isEmpty() )
     config->writeEntry( "Terminal Emulator Splitter Sizes", MAIN_VIEW->getTerminalEmulatorSplitterSizes() );
   
   // save view settings ---> fix when we have tabbed-browsing
   mainView->left->view->saveSettings();
   mainView->right->view->saveSettings();
   
   config->setGroup( "Startup" );
   config->writeEntry( "Vertical Mode", actVerticalMode->isChecked());
   config->sync();
}

void Krusader::saveSettings() {
   toolBar() ->saveSettings( krConfig, "Private" );
   toolBar("actionsToolBar")->saveSettings( krConfig, "Actions Toolbar" );
   config->setGroup( "Startup" );   
   config->writeEntry( "Left Active Tab", mainView->leftMng->activeTab() );
   config->writeEntry( "Right Active Tab", mainView->rightMng->activeTab() );
   mainView->leftMng->saveSettings( krConfig, "Left Tab Bar" );
   mainView->rightMng->saveSettings( krConfig, "Right Tab Bar" );
   
   bool rememberpos = config->readBoolEntry( "Remember Position", _RememberPos );
   bool uisavesettings = config->readBoolEntry( "UI Save Settings", _UiSave );

   // save the popup panel's page of the CURRENT tab
   config->writeEntry( "Left Panel Popup", mainView->left->popup->currentPage() );
   config->writeEntry( "Right Panel Popup", mainView->right->popup->currentPage() );

   // save size and position
   if ( rememberpos || uisavesettings ) {
      savePosition();
   }

   // save the gui
   if ( uisavesettings ) {
      config->setGroup( "Startup" );
      config->writeEntry( "Show status bar", actShowStatusBar->isChecked() );
      config->writeEntry( "Show tool bar", actShowToolBar->isChecked() );
      config->writeEntry( "Show FN Keys", actToggleFnkeys->isChecked() );
      config->writeEntry( "Show Cmd Line", actToggleCmdline->isChecked() );
      config->writeEntry( "Show Terminal Emulator", actToggleTerminal->isChecked() );
      config->writeEntry( "Vertical Mode", actVerticalMode->isChecked());
      config->writeEntry( "Start To Tray", isHidden());
   }

   // save popular links
   popularUrls->save();

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

void Krusader::configChanged() {
   config->setGroup( "Look&Feel" );
   bool minimizeToTray = config->readBoolEntry( "Minimize To Tray", _MinimizeToTray );
   bool singleInstanceMode = config->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );
   
   if( !isHidden() ) {
     if( singleInstanceMode && minimizeToTray )
       sysTray->show();
     else
       sysTray->hide();
   } else {
     if( minimizeToTray )
       sysTray->show();
   }
}

void Krusader::slotClose() {
   directExit = true;
   close();
}

bool Krusader::queryClose() {
   if( isStarting || isExiting )
     return false;
   
   if( kapp->sessionSaving() ) // KDE is logging out, accept the close
   { 
     saveSettings();

     kapp->dcopClient()->registerAs( KApplication::kApplication()->name(), true );

     kapp->deref(); // FIX: krusader exits at closing the viewer when minimized to tray
     kapp->deref(); // and close the application
     return isExiting = true;              // this will also kill the pending jobs
   }
   
   krConfig->setGroup( "Look&Feel" );
   if( !directExit && krConfig->readBoolEntry( "Single Instance Mode", _SingleInstanceMode ) && 
                      krConfig->readBoolEntry( "Minimize To Tray", _MinimizeToTray ) ) {
     hide();
     return false;
   }

   // the shutdown process can be cancelled. That's why
   // the directExit variable is set to normal here.
   directExit = false;

   bool quit = true;
   
   if ( krConfig->readBoolEntry( "Warn On Exit", _WarnOnExit ) ) {
      switch ( KMessageBox::warningYesNo( this,
                                          i18n( "Are you sure you want to quit?" ) ) ) {
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
   if ( quit ) {
      /* First try to close the child windows, because it's the safer
         way to avoid crashes, then close the main window.
         If closing a child is not successful, then we cannot let the
         main window close. */

      for(;;) {
        QWidgetList * list = QApplication::topLevelWidgets();
        QWidget *activeModal = QApplication::activeModalWidget();
        QWidget *w = list->first();

        if( activeModal && activeModal != this && activeModal != sysTray && list->contains( activeModal ) && !activeModal->isHidden() )
          w = activeModal;
        else {
          while(w && (w==this || w==sysTray || w->isHidden()))
            w = list->next();
        }
        delete list;

        if(!w) break;
        bool hid = false;

        if( w->inherits( "KDialogBase" ) ) { // KDE is funny and rejects the close event for
          w->hide();                         // playing a fancy animation with the CANCEL button.
          hid = true;                        // if we hide the widget, KDialogBase accepts the close event
        }

        if( !w->close() ) {
          if( hid )
            w->show();

          if( w->inherits( "QDialog" ) )
            fprintf( stderr, "Failed to close: %s\n", w->className() );
  
          return false;
        }
      }
   
      saveSettings();

      isExiting = true;
      hide();        // hide

      // Changes the name of the application. Single instance mode requires unique appid.
      // As Krusader is exiting, we release that unique appid, so new Krusader instances
      // can be started.
      kapp->dcopClient()->registerAs( KApplication::kApplication()->name(), true );

      kapp->deref(); // FIX: krusader exits at closing the viewer when minimized to tray
      kapp->deref(); // and close the application
      return false;  // don't let the main widget close. It stops the pendig copies!
   } else
      return false;
}

// the please wait dialog functions
void Krusader::startWaiting( QString msg, int count , bool cancel ) {
   plzWait->startWaiting( msg , count, cancel );
}

bool Krusader::wasWaitingCancelled() const { 
	return plzWait->wasCancelled(); 
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

   // call the XML GUI function to draw the UI
   createGUI( mainView->konsole_part );
   
   // this needs to be called AFTER createGUI() !!!
   userActionMenu = (KPopupMenu*) guiFactory()->container( "useractionmenu", this );
   if ( userActionMenu )
      userAction->populateMenu( userActionMenu );
   
   toolBar() ->applySettings( krConfig, "Private" );
	
	toolBar("actionsToolBar") ->applySettings( krConfig, "Actions Toolbar" );
	static_cast<KToggleAction*>(actionCollection()->action("toggle actions toolbar"))->
		setChecked(toolBar("actionsToolBar")->isVisible());
	
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
      // update the Fn bar to the shortcuts selected by the user
      mainView->fnKeys->updateShortcuts();
      if ( !krConfig->readBoolEntry( "Show FN Keys", _ShowFNkeys ) ) {
         mainView->fnKeys->hide();
         actToggleFnkeys->setChecked( false );
      } else {
         mainView->fnKeys->show();
         actToggleFnkeys->setChecked( true );
      }
      // set vertical mode
      if (krConfig->readBoolEntry( "Vertical Mode", false)) {
      	actVerticalMode->setChecked(true);
			mainView->toggleVerticalMode();
      }
   }
	// popular urls
	popularUrls->load();
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
   if ( KrServices::cmdExist( "kdiff3" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "kdiff3", "diff utility" ) );
   } else if ( KrServices::cmdExist( "kompare" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "kompare", "diff utility" ) );
   } else if ( KrServices::cmdExist( "xxdiff" ) ) {
      tools.append( "DIFF" );
      tools.append( KrServices::fullPathName( "xxdiff", "diff utility" ) );
   }
   // a mailer: kmail
   if ( KrServices::cmdExist( "kmail" ) ) {
      tools.append( "MAIL" );
      tools.append( KrServices::fullPathName( "kmail" ) );
   }
   // rename tool: krename
   if ( KrServices::cmdExist( "krename" ) ) {
      tools.append( "RENAME" );
      tools.append( KrServices::fullPathName( "krename" ) );
   }
  // checksum utility
  if (KrServices::cmdExist("md5deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("md5deep", "checksum utility"));
  } else if (KrServices::cmdExist("md5sum")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("md5sum", "checksum utility"));
  } else if (KrServices::cmdExist("sha1deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("sha1deep", "checksum utility"));
  } else if (KrServices::cmdExist("sha256deep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("sha256deep", "checksum utility"));
  } else if (KrServices::cmdExist("tigerdeep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("tigerdeep", "checksum utility"));
  } else if (KrServices::cmdExist("whirlpooldeep")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("whirlpooldeep", "checksum utility"));
  } else if (KrServices::cmdExist("cfv")) {
      tools.append("MD5");
      tools.append(KrServices::fullPathName("cfv", "checksum utility"));
  }

   return tools;
}

QString Krusader::getTempDir() {
   // try to make krusader temp dir
   krConfig->setGroup( "General" );
   QString tmpDir = krConfig->readEntry( "Temp Directory", _TempDirectory );

   if ( ! QDir( tmpDir ).exists() ) {
      for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
         QDir().mkdir( tmpDir.left( i ) );
      QDir().mkdir( tmpDir );
      chmod( tmpDir.local8Bit(), 0777 );
   }

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

   if ( ! QDir( tmpDir ).exists() ) {
      for ( int i = 1 ; i != -1 ; i = tmpDir.find( '/', i + 1 ) )
         QDir().mkdir( tmpDir.left( i ) );
      QDir().mkdir( tmpDir );
      chmod( tmpDir.local8Bit(), 0777 );
   }

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

const char* Krusader::privIcon() {
   if ( geteuid() )
      return "krusader_user";
   else
      return "krusader_root";
}

bool Krusader::process(const QCString &fun, const QByteArray &/* data */, QCString &/* replyType */, QByteArray &/* replyData */) {
   if (fun == "moveToTop()") {
      moveToTop();
      return true;
   } else {
      fprintf( stderr, "Processing DCOP call failed. Function unknown!\n" );
      return false;
   }
}

void Krusader::moveToTop() {
   if( isHidden() )
     show();

   KWin::forceActiveWindow( winId() );
}

#include "krusader.moc"
