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
#include <kaction.h>
#include <ktoggleaction.h>

//HACK
#include "Panel/listpanelactions.h"

class Krusader;

class KrActions : public QObject
{
public:
    KrActions(QObject *parent) : QObject(parent) {}

    // Actions
    static KAction *actCompare;
    static KAction *actCmdlinePopup, *actLocate, *actSwitchFullScreenTE;
    static KAction *actDiskUsage, *actSavePosition;
    static KAction *actHomeTerminal, *actProfiles;
    static KAction *actMountMan, *actNewTool, *actSwapPanels, *actSwapSides;
    static KAction *actKonfigurator, *actToolsSetup, *actFind, *actRemoteEncoding;
    static KAction *actSelectColorMask, *actMultiRename, *actOpenLeftBm, *actOpenRightBm;
    static KAction *actNewTab, *actDupTab, *actCloseTab, *actPreviousTab, *actNextTab, *actCloseInactiveTabs;
    static KAction *actCloseDuplicatedTabs, *actLockTab, *actSplit, *actQueueManager;
    static KAction *actCombine, *actUserMenu, *actManageUseractions, *actSyncDirs;
    static KAction *actVerticalMode;
    static KAction *actEmptyTrash, *actTrashBin;
    static KAction *actPopularUrls;
    static KAction *actViewSaveDefaultSettings, *actShowViewOptionsMenu;
    static KToggleAction *actToggleTerminal;
    static KAction *actSelectNewerAndSingle, *actSelectNewer, *actSelectSingle,
    *actSelectDifferentAndSingle, *actSelectDifferent;
    static KAction *actZoomIn, *actZoomOut, *actDefaultZoom;
    static KAction *actFocusPanel;
    static KAction *actF10;
    /** actions for setting the execution mode of commands from commanddline */
    static KAction *actExecStartAndForget,
    *actExecCollectSeparate, *actExecCollectTogether,
    *actExecTerminalExternal, *actExecTerminalEmbedded;
    static KToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowToolBar,
    *actShowStatusBar, *actToggleHidden, *actCompareDirs, *actTogglePreviews;

    static KAction **compareArray[];
    /** actions for setting the execution mode of commands from commanddline */
    static KAction **execTypeArray[];



#ifdef __KJSEMBED__
    static KAction *actShowJSConsole;
#endif

    static void setupActions(Krusader *krusader);
};

// krusader's actions - things krusader can do!
#define krProperties      ListPanelActions::actProperties     // file properties
#define krPack            ListPanelActions::actPack           // pack files into an archive
#define krUnpack          ListPanelActions::actUnpack         // unpack archive
#define krTest            ListPanelActions::actTest           // test archive
#define krCompare         ListPanelActions::actCompare        // compare 2 files by content
#define krCalculate       ListPanelActions::actCalculate      // calculate occupied space
#define krCreateCS        ListPanelActions::actCreateChecksum
#define krMatchCS         ListPanelActions::actMatchChecksum
#define krSelect          ListPanelActions::actSelect         // select a group by filter
#define krSelectAll       ListPanelActions::actSelectAll      // select all files
#define krUnselect        ListPanelActions::actUnselect       // unselect by filter
#define krUnselectAll     ListPanelActions::actUnselectAll    // remove all selections
#define krInvert          ListPanelActions::actInvert         // invert the selection
#define krSyncDirs        ListPanelActions::actSync           // synchronize directories
#define krFTPConnect      ListPanelActions::actFTPConnect     // connect to an ftp
#define krFTPNew          ListPanelActions::actFTPNewConnect  // create a new connection
#define krFTPDiss         ListPanelActions::actFTPDisconnect  // disconnect an FTP session
#define krAllFiles        ListPanelActions::actAllFilter      // show all files in list
#define krExecFiles       ListPanelActions::actExecFilter     // show only executables
#define krCustomFiles     ListPanelActions::actCustomFilter   // show a custom set of files
#define krOpenLeftBm      ListPanelActions::actOpenLeftBm     // open left bookmarks
#define krOpenRightBm     ListPanelActions::actOpenRightBm    // open right bookmarks
#define krDirUp           ListPanelActions::actDirUp
#define krCopy            ListPanelActions::actCopy
#define krPaste           ListPanelActions::actPaste
#define krLocationBar     ListPanelActions::actLocationBar
#define krJumpBack        ListPanelActions::actJumpBack
#define krSetJumpBack     ListPanelActions::actSetJumpBack
#define krF2              ListPanelActions::actF2
#define krF3              ListPanelActions::actF3
#define krF4              ListPanelActions::actF4
#define krF5              ListPanelActions::actF5
#define krF6              ListPanelActions::actF6
#define krF7              ListPanelActions::actF7
#define krF8              ListPanelActions::actF8
#define krF9              ListPanelActions::actF9
#define krF10             KrActions::actF10
#define krHomeTerm        KrActions::actHomeTerminal   // open terminal@home dir
#define krRemoteEncoding  KrActions::actRemoteEncoding // remote encoding menu
#define krMountMan        KrActions::actMountMan       // run Mount-manager
#define krNewTool         KrActions::actNewTool        // Add a new tool to menu
#define krKonfigurator    KrActions::actKonfigurator
#define krToolsSetup      KrActions::actToolsSetup     // setup the tools menu
#define krRoot            KrActions::actRoot
#define krFind            KrActions::actFind           // find files
#define krMultiRename     KrActions::actMultiRename
#define krToggleTerminal  KrActions::actToggleTerminal
//#define krToggleSortByExt KrActions::actToggleSortByExt// Sort by extension
#define krSwitchFullScreenTE KrActions::actSwitchFullScreenTE
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
#define krPopularUrls     KrActions::actPopularUrls

#ifdef __KJSEMBED__
#define krJSConsole  KrActions::actShowJSConsole
#endif

#endif
