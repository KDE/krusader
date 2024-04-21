/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRACTIONS_H
#define KRACTIONS_H

#include "GUI/recentlyclosedtabsmenu.h"

// QtCore
#include <QObject>
// QtWidgets
#include <QAction>

#include <KToggleAction>

class Krusader;

class KrActions : public QObject
{
    Q_OBJECT
public:
    explicit KrActions(QObject *parent)
        : QObject(parent)
    {
    }

    // Actions
    static QAction *actCompare;
    static QAction *actCmdlinePopup, *actLocate, *actSwitchFullScreenTE;
    static QAction *actDiskUsage, *actSavePosition;
    static QAction *actHomeTerminal, *actProfiles;
    static QAction *actMountMan, *actNewTool, *actSwapPanels, *actSwapSides;
    static QAction *actKonfigurator, *actToolsSetup, *actFind;
    static QAction *actRemoteEncoding;
    static RecentlyClosedTabsMenu *actClosedTabsMenu;
    static QAction *actSelectColorMask, *actMultiRename, *actOpenLeftBm, *actOpenRightBm, *actAddBookmark;
    static QAction *actSplit;
    static QAction *actCombine;
    static QAction *actUserMenu;
    static QAction *actManageUseractions;
#ifdef SYNCHRONIZER_ENABLED
    static QAction *actSyncDirs;
#endif
    static QAction *actVerticalMode;
    static QAction *actEmptyTrash, *actTrashBin;
    static QAction *actPopularUrls;
    static KToggleAction *actToggleTerminal;
    static QAction *actSelectNewerAndSingle, *actSelectNewer, *actSelectSingle, *actSelectDifferentAndSingle, *actSelectDifferent;
    static QAction *actF10Quit;
    /** actions for setting the execution mode of commands from commanddline */
    static QAction *actExecStartAndForget, *actExecCollectSeparate, *actExecCollectTogether, *actExecTerminalExternal, *actExecTerminalEmbedded;
    static KToggleAction *actToggleFnkeys, *actToggleCmdline, *actShowStatusBar, *actToggleHidden, *actCompareDirs;

    static QAction **compareArray[];
    /** actions for setting the execution mode of commands from commanddline */
    static QAction **execTypeArray[];

    /** JobMan toolbar actions */
    static QAction *actJobProgress;
    static QAction *actJobControl;
    static QAction *actJobMode;
    static QAction *actJobUndo;

    static void setupActions(Krusader *krusader);
};

// krusader's actions - things krusader can do!
#define krHomeTerm KrActions::actHomeTerminal // open terminal@home dir
#define krRemoteEncoding KrActions::actRemoteEncoding // remote encoding menu
#define krMountMan KrActions::actMountMan // run Mount-manager
#define krNewTool KrActions::actNewTool // Add a new tool to menu
#define krKonfigurator KrActions::actKonfigurator
#define krToolsSetup KrActions::actToolsSetup // setup the tools menu
#define krRoot KrActions::actRoot
#define krFind KrActions::actFind // find files
#define krMultiRename KrActions::actMultiRename
// #define krToggleSortByExt KrActions::actToggleSortByExt// Sort by extension
#define krSwitchFullScreenTE KrActions::actSwitchFullScreenTE
#define krCmdlinePopup KrActions::actCmdlinePopup
#define krSplit KrActions::actSplit
#define krCombine KrActions::actCombine
#define krUserMenu KrActions::actUserMenu
#define krPopularUrls KrActions::actPopularUrls

#endif
