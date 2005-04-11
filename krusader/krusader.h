
/***************************************************************************
                                krusader.h
                           -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
 The main application ! what's more to say ?
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD
 
                                                     H e a d e r    F i l e
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRUSADER_H
#define KRUSADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kconfig.h>
#include <kaccel.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <kdebug.h>

#ifdef __KJSEMBED__
#include <kjsembed/kjsembedpart.h>
#endif

class KrusaderStatus;
class KRPleaseWaitHandler;
class KrusaderView;
class KProcess;
class KRslots;
class KIconLoader;
class KSystemTray;
class UserMenu;
class UserAction;
class Expander;
class KMountMan;
class KrBookmarkHandler;
class PopularUrls;

//static QTextOStream krOut(stdout);
#define krOut kdDebug(50010)

class Krusader : public KParts::MainWindow {
    Q_OBJECT
  public:
    Krusader();
    ~Krusader();
    void refreshView();				 // re-create the main view
    static QStringList supportedTools(); // find supported tools
	 void importKeyboardShortcuts(QString filename);
	 void exportKeyboardShortcuts(QString filename);
		
  public slots:
    // increase the internal progress bar
    void incProgress( KProcess *, char *buffer, int buflen );
    void statusBarUpdate( QString& mess );
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void quitKrusader();
    void updateGUI( bool enforce = false );

  protected:
    bool queryExit() {
      config->sync();
      return true;
    }
    bool queryClose();
    void setupActions();
    void setupAccels();
    bool versionControl();  // handle version differences in krusaderrc
    void showEvent ( QShowEvent * );
    void hideEvent ( QHideEvent * );
    void moveEvent ( QMoveEvent * );
    void resizeEvent ( QResizeEvent * );

  public:
    static Krusader *App;       // a kApp style pointer
    KMountMan *mountMan;  // krusader's Mount Manager
    KrusaderView *mainView;  // The GUI
    KConfig *config;    // allow everyone to access the config
    KIconLoader *iconLoader; // the app's icon loader
	 PopularUrls *popularUrls; // holds a sorted list of the most popular urls visited
    // Actions
    static KAction *actProperties, *actPack, *actUnpack, *actTest, *actCompare, *actCmdlinePopup;
    static KAction *actCalculate, *actSelect, *actUnselect, *actSelectAll, *actLocate;
    static KAction *actUnselectAll, *actInvert, *actSync, *actDiskUsage, *actSavePosition, *actCompDirs;
    static KAction *actHomeTerminal, *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect, *actProfiles;
    static KAction *actExecFilter, *actCustomFilter, *actMountMan, *actNewTool, *actSwapPanels;
    static KAction *actKonfigurator, *actToolsSetup, *actBack, *actRoot, *actFind, *actDirUp;
    static KAction *actSelectColorMask, *actMultiRename, *actAllFilter, *actOpenLeftBm, *actOpenRightBm;
    static KAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actSplit; 
	 static KAction *actCombine, *actUserMenu, *actManageUseractions, *actSyncDirs, *actSyncBrowse;
	 static KAction *actF2, *actF3, *actF4, *actF5, *actF6, *actF7, *actF8, *actF9, *actF10;
	 static KAction *actPopularUrls, *actLocationBar;
    static KToggleAction *actToggleTerminal;
    static KRadioAction *actMarkNewerAndSingle, *actMarkNewer, *actMarkSingle, 
                        *actMarkDifferentAndSingle, *actMarkDifferent;
    KToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowToolBar,
                  *actShowStatusBar, *actToggleHidden, *actCompareDirs;

    static KRadioAction **compareArray[];

    // return a path to a temp dir or file we can use.
    QString getTempDir();
    QString getTempFile();

    // the internal progress bar variales + functions
    KRPleaseWaitHandler* plzWait;
    void startWaiting( QString msg = "Please Wait", int count = 0 , bool cancel = false );
    void stopWait();

    KrusaderStatus *status;
    KRslots *slot;
    KAccel *accels; // global accelerators
    
    static KPopupMenu *userActionMenu;
    static UserMenu *userMenu;
    static UserAction *userAction;
    static Expander *expander;
	 static KrBookmarkHandler *bookman;

    #ifdef __KJSEMBED__
    static KJSEmbed::KJSEmbedPart *js;
    static KAction *actShowJSConsole;
    #endif
  
  signals:
    void changeMessage( QString );

  private:
    KSystemTray *sysTray;
    QPoint       oldPos;
    QSize        oldSize;
    bool         isStarting;
};

// main modules
#define krApp        Krusader::App
#define krConfig     Krusader::App->config
#define krMtMan      (*(Krusader::App->mountMan))
#define krBookMan    Krusader::App->bookman
#define SLOTS        Krusader::App->slot
#define krLoader     Krusader::App->iconLoader

#define MAIN_VIEW    (krApp->mainView)
#define ACTIVE_MNG   (MAIN_VIEW->activeManager())
#define ACTIVE_PANEL (MAIN_VIEW->activePanel)
#define ACTIVE_FUNC  (ACTIVE_PANEL->func)
#define OTHER_PANEL	(ACTIVE_PANEL->otherPanel)
#define OTHER_FUNC	(OTHER_PANEL->func)
#define LEFT_PANEL	(MAIN_VIEW->left)
#define LEFT_FUNC		(LEFT_PANEL->func)
#define RIGHT_PANEL  (MAIN_VIEW->right)
#define RIGHT_FUNC	(RIGHT_PANEL->func)

// krusader's actions - things krusader can do!
#define krProperties      Krusader::App->actProperties     // file properties
#define krPack            Krusader::App->actPack           // pack files into an archive
#define krUnpack          Krusader::App->actUnpack         // unpack archive
#define krTest            Krusader::App->actTest           // test archive
#define krCompare         Krusader::App->actCompare        // compare 2 files by content
#define krCalculate       Krusader::App->actCalculate      // calculate occupied space
#define krSelect          Krusader::App->actSelect         // select a group by filter
#define krSelectAll       Krusader::App->actSelectAll      // select all files
#define krUnselect        Krusader::App->actUnselect       // unselect by filter
#define krUnselectAll     Krusader::App->actUnselectAll    // remove all selections
#define krInvert          Krusader::App->actInvert         // invert the selection
#define krSyncDirs        Krusader::App->actSync           // syncronize directories
#define krHomeTerm        Krusader::App->actHomeTerminal   // open terminal@home dir
#define krFTPConnect      Krusader::App->actFTPConnect     // connect to an ftp
#define krFTPNew          Krusader::App->actFTPNewConnect  // create a new connection
#define krFTPDiss         Krusader::App->actFTPDisconnect  // disconnect an FTP session
#define krAllFiles        Krusader::App->actAllFilter      // show all files in list
#define krExecFiles       Krusader::App->actExecFilter     // show only executables
#define krCustomFiles     Krusader::App->actCustomFilter   // show a custom set of files
#define krMountMan        Krusader::App->actMountMan       // run Mount-manager
#define krNewTool         Krusader::App->actNewTool        // Add a new tool to menu
#define krKonfigurator    Krusader::App->actKonfigurator
#define krToolsSetup      Krusader::App->actToolsSetup     // setup the tools menu
#define krBack            Krusader::App->actBack
#define krRoot            Krusader::App->actRoot
#define krFind            Krusader::App->actFind           // find files
#define krMultiRename     Krusader::App->actMultiRename
#define krToggleTerminal  Krusader::App->actToggleTerminal
#define krToggleSortByExt Krusader::App->actToggleSortByExt// Sort by extention
#define krOpenLeftBm      Krusader::App->actOpenLeftBm     // open left bookmarks
#define krOpenRightBm     Krusader::App->actOpenRightBm    // open left bookmarks
#define krDirUp           Krusader::App->actDirUp
#define krCmdlinePopup    Krusader::App->actCmdlinePopup
#define krNewTab          Krusader::App->actNewTab
#define krDupTab          Krusader::App->actDupTab
#define krCloseTab        Krusader::App->actCloseTab
#define krNextTab         Krusader::App->actNextTab
#define krPreviousTab     Krusader::App->actPreviousTab
#define krSplit           Krusader::App->actSplit
#define krCombine         Krusader::App->actCombine
#define krUserMenu        Krusader::App->actUserMenu
#define krUserActionMenu      Krusader::App->userActionMenu
#define krUserAction      Krusader::App->userAction
#define krExpander        Krusader::App->expander
#define krF2				  Krusader::App->actF2
#define krF3				  Krusader::App->actF3
#define krF4				  Krusader::App->actF4
#define krF5				  Krusader::App->actF5
#define krF6				  Krusader::App->actF6
#define krF7				  Krusader::App->actF7
#define krF8				  Krusader::App->actF8
#define krF9				  Krusader::App->actF9
#define krF10				  Krusader::App->actF10
#define krPopularUrls	  Krusader::App->actPopularUrls
#define krLocationBar	  Krusader::App->actLocationBar

#ifdef __KJSEMBED__
#define krJS			Krusader::App->js
#define krJSConsole		Krusader::App->actShowJSConsole
#endif

#endif
