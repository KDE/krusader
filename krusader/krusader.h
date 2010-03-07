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
#include <kstandardaction.h>
#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QtCore/QStringList>
#include <QMoveEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QHideEvent>
#include <kdebug.h>
#include "VFS/kiojobwrapper.h"

#ifdef __KJSEMBED__
class KrJS;
#endif

class KrusaderStatus;
class KRPleaseWaitHandler;
class KrusaderView;
class KRslots;
class KIconLoader;
class KSystemTrayIcon;
class UserMenu;
class UserAction;
class Expander;
class KMountMan;
class KrBookmarkHandler;
class PopularUrls;
class QueueManager;

#define MAX_VIEWS 6

class Krusader : public KParts::MainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.Instance")

public:
    Krusader();
    virtual ~Krusader();
    void refreshView();     // re-create the main view
    void configChanged();
    /**
     * This returns a defferent icon if krusader runs with root-privileges
     * @return a character string with the specitif icon-name
     */
    static const char* privIcon();
    static QStringList supportedTools(); // find supported tools

    void moveToTop();

public slots:
    void statusBarUpdate(QString& mess);
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void updateUserActions();
    void updateGUI(bool enforce = false);
    void slotClose();
    void setDirectExit() {
        directExit = true;
    }

protected:
    bool queryExit();
    bool queryClose();
    void setupActions();
    void setupAccels();
    bool versionControl();  // handle version differences in krusaderrc
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);

public Q_SLOTS:
    Q_SCRIPTABLE bool isRunning();
    Q_SCRIPTABLE bool isLeftActive();

public:
    static Krusader *App;       // a kApp style pointer
    static QString   AppName;   // the name of the application
#if 0
    KMountMan *mountMan;  // krusader's Mount Manager
    KrusaderView *mainView;  // The GUI
    KConfig *config;    // allow everyone to access the config
    KIconLoader *iconLoader; // the app's icon loader
#endif
    PopularUrls *popularUrls; // holds a sorted list of the most popular urls visited
    QueueManager *queueManager;
#if 0
    // Actions
    static KAction *actProperties, *actPack, *actUnpack, *actTest, *actCompare, *actCmdlinePopup;
    static KAction *actCalculate, *actSelect, *actUnselect, *actSelectAll, *actLocate, *actSwitchFullScreenTE;
    static KAction *actUnselectAll, *actInvert, *actSync, *actDiskUsage, *actSavePosition, *actCompDirs;
    static KAction *actHomeTerminal, *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect, *actProfiles;
    static KAction *actExecFilter, *actCustomFilter, *actMountMan, *actNewTool, *actSwapPanels, *actSwapSides;
    static KAction *actKonfigurator, *actToolsSetup, *actBack, *actRoot, *actFind, *actDirUp, *actRemoteEncoding;
    static KAction *actSelectColorMask, *actMultiRename, *actAllFilter, *actOpenLeftBm, *actOpenRightBm;
    static KAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actCloseInactiveTabs;
    static KAction *actCloseDuplicatedTabs, *actLockTab, *actSplit, *actQueueManager;
    static KAction *actCombine, *actUserMenu, *actManageUseractions, *actSyncDirs, *actSyncBrowse, *actCancelRefresh;
    static KAction *actF2, *actF3, *actF4, *actF5, *actF6, *actF7, *actF8, *actF9, *actF10, *actVerticalMode;
    static KAction *actShiftF5, *actShiftF6, *actEmptyTrash, *actTrashBin;
    static KAction *actPopularUrls, *actLocationBar, *actJumpBack, *actSetJumpBack, *actCreateChecksum, *actMatchChecksum;
    static KAction *actView0, *actView1, *actView2, *actView3, *actView4, *actView5, *actCopy, *actPaste;
    static KToggleAction *actToggleTerminal;
    static KAction *actSelectNewerAndSingle, *actSelectNewer, *actSelectSingle,
    *actSelectDifferentAndSingle, *actSelectDifferent;
    static KAction *actZoomIn, *actZoomOut, *actDefaultZoom;
    /** actions for setting the execution mode of commands from commanddline */
    static KAction *actExecStartAndForget,
    *actExecCollectSeparate, *actExecCollectTogether,
    *actExecTerminalExternal, *actExecTerminalEmbedded;
    KToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowToolBar,
    *actShowStatusBar, *actToggleHidden, *actCompareDirs, *actTogglePreviews;

    static KAction **compareArray[];
    /** actions for setting the execution mode of commands from commanddline */
    static KAction **execTypeArray[];
#endif
    // return a path to a temp dir or file we can use.
    QString getTempDir();
    QString getTempFile();

    // the internal progress bar variales + functions
    KRPleaseWaitHandler* plzWait;
    void startWaiting(QString msg = "Please Wait", int count = 0 , bool cancel = false);
    void stopWait();
    bool wasWaitingCancelled() const;

    KrusaderStatus *status;
#if 0
    KRslots *slot;
#endif
    static UserMenu *userMenu;
#if 0
    static KrBookmarkHandler *bookman;
#endif

#ifdef __KJSEMBED__
    static KrJS *js;
#endif

signals:
    void changeMessage(QString);
    // emitted when we are about to quit
    void shutdown();

private:
    KSystemTrayIcon *sysTray;
    QPoint       oldPos;
    QSize        oldSize;
    bool         isStarting;
    bool         isExiting;
    bool         directExit;
    KrJobStarter jobStarter;
    static void supportedTool(QStringList &tools, QString toolType,
                              QStringList names, QString confName);
};


// main modules
#define krApp        Krusader::App

#if 0
#define krConfig     Krusader::App->config
#define krMtMan      (*(Krusader::App->mountMan))
#define krBookMan    Krusader::App->bookman
#define SLOTS        Krusader::App->slot
#define krLoader     Krusader::App->iconLoader

#define MAIN_VIEW    (krApp->mainView)
#define ACTIVE_MNG   (MAIN_VIEW->activeManager())
#define ACTIVE_PANEL (MAIN_VIEW->activePanel)
#define ACTIVE_FUNC  (ACTIVE_PANEL->func)
#define OTHER_MNG  (MAIN_VIEW->inactiveManager())
#define OTHER_PANEL (ACTIVE_PANEL->otherPanel)
#define OTHER_FUNC (OTHER_PANEL->func)
#define LEFT_PANEL (MAIN_VIEW->left)
#define LEFT_FUNC  (LEFT_PANEL->func)
#define LEFT_MNG  (MAIN_VIEW->leftMng)
#define RIGHT_PANEL  (MAIN_VIEW->right)
#define RIGHT_FUNC (RIGHT_PANEL->func)
#define RIGHT_MNG  (MAIN_VIEW->rightMng)

// krusader's actions - things krusader can do!
#define krProperties      Krusader::App->actProperties     // file properties
#define krPack            Krusader::App->actPack           // pack files into an archive
#define krUnpack          Krusader::App->actUnpack         // unpack archive
#define krTest            Krusader::App->actTest           // test archive
#define krCompare         Krusader::App->actCompare        // compare 2 files by content
#define krCalculate       Krusader::App->actCalculate      // calculate occupied space
#define krCreateCS   Krusader::App->actCreateChecksum
#define krMatchCS   Krusader::App->actMatchChecksum
#define krSelect          Krusader::App->actSelect         // select a group by filter
#define krSelectAll       Krusader::App->actSelectAll      // select all files
#define krUnselect        Krusader::App->actUnselect       // unselect by filter
#define krUnselectAll     Krusader::App->actUnselectAll    // remove all selections
#define krInvert          Krusader::App->actInvert         // invert the selection
#define krSyncDirs        Krusader::App->actSync           // synchronize directories
#define krHomeTerm        Krusader::App->actHomeTerminal   // open terminal@home dir
#define krFTPConnect      Krusader::App->actFTPConnect     // connect to an ftp
#define krFTPNew          Krusader::App->actFTPNewConnect  // create a new connection
#define krFTPDiss         Krusader::App->actFTPDisconnect  // disconnect an FTP session
#define krRemoteEncoding  Krusader::App->actRemoteEncoding // remote encoding menu
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
#define krToggleSortByExt Krusader::App->actToggleSortByExt// Sort by extension
#define krOpenLeftBm      Krusader::App->actOpenLeftBm     // open left bookmarks
#define krOpenRightBm     Krusader::App->actOpenRightBm    // open right bookmarks
#define krDirUp           Krusader::App->actDirUp
#define krCmdlinePopup    Krusader::App->actCmdlinePopup
#define krNewTab          Krusader::App->actNewTab
#define krLockTab         Krusader::App->actLockTab
#define krDupTab          Krusader::App->actDupTab
#define krCloseTab        Krusader::App->actCloseTab
#define krNextTab         Krusader::App->actNextTab
#define krPreviousTab     Krusader::App->actPreviousTab
#define krCloseInactiveTabs Krusader::App->actCloseInactiveTabs
#define krCloseDuplicatedTabs Krusader::App->actCloseDuplicatedTabs
#define krSplit           Krusader::App->actSplit
#define krCombine         Krusader::App->actCombine
#define krUserMenu        Krusader::App->actUserMenu
#define krF2      Krusader::App->actF2
#define krF3      Krusader::App->actF3
#define krF4      Krusader::App->actF4
#define krF5      Krusader::App->actF5
#define krF6      Krusader::App->actF6
#define krF7      Krusader::App->actF7
#define krF8      Krusader::App->actF8
#define krF9      Krusader::App->actF9
#define krF10      Krusader::App->actF10
#define krPopularUrls   Krusader::App->actPopularUrls
#define krLocationBar   Krusader::App->actLocationBar
#define krJumpBack   Krusader::App->actJumpBack
#define krSetJumpBack   Krusader::App->actSetJumpBack
#endif


#ifdef __KJSEMBED__
#define krJS   Krusader::App->js
#endif

#endif
