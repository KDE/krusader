/***************************************************************************
                                kractions.h
                           -------------------
    begin                : Thu May 4 2000
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRACTIONS_H
#define KRACTIONS_H

#include <QObject>

class Krusader;

class KrActions : public QObject
{
public:
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
    static KToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowToolBar,
    *actShowStatusBar, *actToggleHidden, *actCompareDirs, *actTogglePreviews;

    static KAction **compareArray[];
    /** actions for setting the execution mode of commands from commanddline */
    static KAction **execTypeArray[];

    static void setupActions(Krusader *krusader);
};

// krusader's actions - things krusader can do!
#define krProperties      KrActions::actProperties     // file properties
#define krPack            KrActions::actPack           // pack files into an archive
#define krUnpack          KrActions::actUnpack         // unpack archive
#define krTest            KrActions::actTest           // test archive
#define krCompare         KrActions::actCompare        // compare 2 files by content
#define krCalculate       KrActions::actCalculate      // calculate occupied space
#define krCreateCS   KrActions::actCreateChecksum
#define krMatchCS   KrActions::actMatchChecksum
#define krSelect          KrActions::actSelect         // select a group by filter
#define krSelectAll       KrActions::actSelectAll      // select all files
#define krUnselect        KrActions::actUnselect       // unselect by filter
#define krUnselectAll     KrActions::actUnselectAll    // remove all selections
#define krInvert          KrActions::actInvert         // invert the selection
#define krSyncDirs        KrActions::actSync           // synchronize directories
#define krHomeTerm        KrActions::actHomeTerminal   // open terminal@home dir
#define krFTPConnect      KrActions::actFTPConnect     // connect to an ftp
#define krFTPNew          KrActions::actFTPNewConnect  // create a new connection
#define krFTPDiss         KrActions::actFTPDisconnect  // disconnect an FTP session
#define krRemoteEncoding  KrActions::actRemoteEncoding // remote encoding menu
#define krAllFiles        KrActions::actAllFilter      // show all files in list
#define krExecFiles       KrActions::actExecFilter     // show only executables
#define krCustomFiles     KrActions::actCustomFilter   // show a custom set of files
#define krMountMan        KrActions::actMountMan       // run Mount-manager
#define krNewTool         KrActions::actNewTool        // Add a new tool to menu
#define krKonfigurator    KrActions::actKonfigurator
#define krToolsSetup      KrActions::actToolsSetup     // setup the tools menu
#define krBack            KrActions::actBack
#define krRoot            KrActions::actRoot
#define krFind            KrActions::actFind           // find files
#define krMultiRename     KrActions::actMultiRename
#define krToggleTerminal  KrActions::actToggleTerminal
#define krToggleSortByExt KrActions::actToggleSortByExt// Sort by extension
#define krSwitchFullScreenTE KrActions::actSwitchFullScreenTE
#define krOpenLeftBm      KrActions::actOpenLeftBm     // open left bookmarks
#define krOpenRightBm     KrActions::actOpenRightBm    // open right bookmarks
#define krDirUp           KrActions::actDirUp
#define krCmdlinePopup    KrActions::actCmdlinePopup
#define krNewTab          KrActions::actNewTab
#define krLockTab         KrActions::actLockTab
#define krDupTab          KrActions::actDupTab
#define krCloseTab        KrActions::actCloseTab
#define krNextTab         KrActions::actNextTab
#define krPreviousTab     KrActions::actPreviousTab
#define krCloseInactiveTabs KrActions::actCloseInactiveTabs
#define krCloseDuplicatedTabs KrActions::actCloseDuplicatedTabs
#define krSplit           KrActions::actSplit
#define krCombine         KrActions::actCombine
#define krUserMenu        KrActions::actUserMenu
//#define krUserActionMenu      KrActions::userActionMenu
//#define krUserAction      KrActions::userAction
#define krF2      KrActions::actF2
#define krF3      KrActions::actF3
#define krF4      KrActions::actF4
#define krF5      KrActions::actF5
#define krF6      KrActions::actF6
#define krF7      KrActions::actF7
#define krF8      KrActions::actF8
#define krF9      KrActions::actF9
#define krF10      KrActions::actF10
#define krCopy      KrActions::actCopy
#define krPaste      KrActions::actPaste
#define krPopularUrls   KrActions::actPopularUrls
#define krLocationBar   KrActions::actLocationBar
#define krJumpBack   KrActions::actJumpBack
#define krSetJumpBack   KrActions::actSetJumpBack

#endif
