
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
#include "MountMan/kmountman.h"

class KrusaderStatus;
class KRPleaseWaitHandler;
class KrusaderView;
class KProcess;
class KRslots;
class KIconLoader;
class KSystemTray;
class UserMenu;
class UserAction;

class Krusader : public KParts::MainWindow {
    Q_OBJECT
  public:
    Krusader();
    ~Krusader();
    void refreshView();				 // re-create the main view
    static QStringList supportedTools(); // find supported tools

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
    MountMan::KMountMan *mountMan;  // krusader's Mount Manager
    KrusaderView *mainView;  // The GUI
    KConfig *config;    // allow everyone to access the config
    KIconLoader *iconLoader; // the app's icon loader
    // Actions
    static KAction *actProperties, *actPack, *actUnpack, *actTest, *actCompare, *actCmdlinePopup;
    static KAction *actCalculate, *actSelect, *actUnselect, *actSelectAll, *actLocate;
    static KAction *actUnselectAll, *actInvert, *actSync, *actSavePosition, *actCompDirs;
    static KAction *actHomeTerminal, *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect;
    static KAction *actExecFilter, *actCustomFilter, *actMountMan, *actNewTool, *actSwapPanels;
    static KAction *actKonfigurator, *actToolsSetup, *actBack, *actRoot, *actFind, *actDirUp;
    static KAction *actSelectColorMask, *actMultiRename, *actAllFilter, *actOpenLeftBm, *actOpenRightBm;
    static KAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actSplit; 
	 static KAction *actCombine, *actUserMenu, *actSyncDirs;
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
    static UserMenu *userMenu;
    static UserAction *userAction;

  signals:
    void changeMessage( QString );

  private:
    KSystemTray *sysTray;
    QPoint       oldPos;
    QSize        oldSize;
};

// main modules
#define krApp        Krusader::App
#define krConfig     Krusader::App->config
#define krMtMan      (*(Krusader::App->mountMan))
#define SLOTS        Krusader::App->slot
#define krLoader     Krusader::App->iconLoader
// krusader's actions - things krusader can do!
#define krProperties      Krusader::App->actProperties     // file properties
#define krPack            Krusader::App->actPack           // pack files into an archive
#define krUnpack          Krusader::App->actUnpack         // unpack archive
#define krTest            Krusader::App->actTest           // test archive
#define krCompare         Krusader::App->actCompare        // compare 2 files by content
#define krCompareDirs     Krusader::App->actCompareDirs    // compare 2 directories
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
#define krSelectColorMask Krusader::App->actSelectColorMask// select compare-mask
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
#define krUserAction           Krusader::App->userAction

#endif
