/***************************************************************************
                          kractions.cpp
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

#include "kractions.h"
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <ktoolbarpopupaction.h>
#include <QMenu>

#include "defaults.h"
#include "krusader.h"
#include "krusaderview.h"
#include "krslots.h"
#include "krtrashhandler.h"
#include "Panel/krviewfactory.h"
#include "GUI/krremoteencodingmenu.h"
#include "UserAction/useraction.h"
#include "MountMan/kmountman.h"
#include "Dialogs/popularurls.h"

KAction *KrActions::actCompare = 0;
KAction *KrActions::actDiskUsage = 0;
KAction *KrActions::actQueueManager = 0;
KAction *KrActions::actHomeTerminal = 0;
KAction *KrActions::actRemoteEncoding = 0;
KAction *KrActions::actProfiles = 0;
KAction *KrActions::actMultiRename = 0;
KAction *KrActions::actMountMan = 0;
KAction *KrActions::actNewTool = 0;
KAction *KrActions::actKonfigurator = 0;
KAction *KrActions::actToolsSetup = 0;
KAction *KrActions::actSwapPanels = 0;
KAction *KrActions::actSwapSides = 0;
KAction *KrActions::actFind = 0;
KAction *KrActions::actLocate = 0;
KAction *KrActions::actSwitchFullScreenTE = 0;
//KAction *KrActions::actAddBookmark = 0;
KAction *KrActions::actSavePosition = 0;
KAction *KrActions::actSelectColorMask = 0;
KAction *KrActions::actOpenLeftBm = 0;
KAction *KrActions::actOpenRightBm = 0;
KAction *KrActions::actCmdlinePopup = 0;
KAction *KrActions::actNewTab = 0;
KAction *KrActions::actDupTab = 0;
KAction *KrActions::actLockTab = 0;
KAction *KrActions::actCloseTab = 0;
KAction *KrActions::actNextTab = 0;
KAction *KrActions::actPreviousTab = 0;
KAction *KrActions::actCloseInactiveTabs = 0;
KAction *KrActions::actCloseDuplicatedTabs = 0;
KAction *KrActions::actSplit = 0;
KAction *KrActions::actCombine = 0;
KAction *KrActions::actUserMenu = 0;
KAction *KrActions::actManageUseractions = 0;
KAction *KrActions::actSyncDirs = 0;
KAction *KrActions::actF10 = 0;
KAction *KrActions::actEmptyTrash = 0;
KAction *KrActions::actTrashBin = 0;
KAction *KrActions::actPopularUrls = 0;

KToggleAction *KrActions::actToggleTerminal = 0;
KAction  *KrActions::actVerticalMode = 0;
KAction  *KrActions::actSelectNewerAndSingle = 0;
KAction  *KrActions::actSelectSingle = 0;
KAction  *KrActions::actSelectNewer = 0;
KAction  *KrActions::actSelectDifferentAndSingle = 0;
KAction  *KrActions::actSelectDifferent = 0;
KAction  **KrActions::compareArray[] = {&actSelectNewerAndSingle, &actSelectNewer, &actSelectSingle,
                                       &actSelectDifferentAndSingle, &actSelectDifferent, 0
                                      };
KAction *KrActions::actExecStartAndForget = 0;
KAction *KrActions::actExecCollectSeparate = 0;
KAction *KrActions::actExecCollectTogether = 0;
KAction *KrActions::actExecTerminalExternal = 0;
KAction *KrActions::actExecTerminalEmbedded = 0;
KAction **KrActions::execTypeArray[] = {&actExecStartAndForget, &actExecCollectSeparate, &actExecCollectTogether,
                                       &actExecTerminalExternal, &actExecTerminalEmbedded, 0
                                      };
KToggleAction *KrActions::actToggleFnkeys = 0;
KToggleAction *KrActions::actToggleCmdline = 0;
KToggleAction *KrActions::actShowToolBar = 0;
KToggleAction *KrActions::actShowStatusBar = 0;
KToggleAction *KrActions::actToggleHidden = 0;
KToggleAction *KrActions::actCompareDirs = 0;


#ifdef __KJSEMBED__
    static KAction *actShowJSConsole;
#endif


KAction *createAction(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, Krusader *krusaderApp)
{
    KAction *a;
    if (icon == 0)
        a = new KAction(text, krusaderApp);
    else
        a = new KAction(KIcon(icon), text, krusaderApp);
    a->setShortcut(shortcut);
    krusaderApp->connect(a, SIGNAL(triggered(bool)), recv, slot);
    krusaderApp->actionCollection()->addAction(name, a);
    return a;
}

KToggleAction *createToggleAction(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, Krusader *krusaderApp)
{
    KToggleAction *a;
    if (icon == 0)
        a = new KToggleAction(text, krusaderApp);
    else
        a = new KToggleAction(KIcon(icon), text, krusaderApp);
    a->setShortcut(shortcut);
    krusaderApp->connect(a, SIGNAL(triggered(bool)), recv, slot);
    krusaderApp->actionCollection()->addAction(name, a);
    return a;
}


void KrActions::setupActions(Krusader *krusaderApp)
{

#define NEW_KACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krusaderApp);

#define NEW_KTOGGLEACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createToggleAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krusaderApp);


    // first come the TODO actions
    //actSync =       0;//new KAction(i18n("S&ynchronize Dirs"),                         0, krusaderApp, 0, actionCollection(), "sync dirs");
    //actNewTool =    0;//new KAction(i18n("&Add a new tool"),                          0, krusaderApp, 0, actionCollection(), "add tool");
    //actToolsSetup = 0;//new KAction(i18n("&Tools Menu Setup"),                        0, 0, krusaderApp, 0, actionCollection(), "tools setup");
    //KStandardAction::print(SLOTS, 0,actionCollection(),"std_print");
    //KStandardAction::showMenubar( SLOTS, SLOT( showMenubar() ), actionCollection(), "std_menubar" );

    /* Shortcut disabled because of the Terminal Emulator bug. */
    KConfigGroup group(krConfig, "Private");
    int compareMode = group.readEntry("Compare Mode", 0);
    int cmdExecMode =  group.readEntry("Command Execution Mode", 0);

    KAction *tabSwitch;
    NEW_KACTION(tabSwitch, i18n("Tab-Switch panel"), 0, Qt::Key_Tab, MAIN_VIEW, SLOT(panelSwitch()), "tab");

    actShowToolBar = (KToggleAction*)KStandardAction::create(KStandardAction::ShowToolbar, SLOTS, SLOT(toggleToolbar()), krusaderApp->actionCollection()/*, "std_toolbar"*/);

    KToggleAction *toggleActToolbar;
    NEW_KTOGGLEACTION(toggleActToolbar, i18n("Show Actions Toolbar"), 0, 0, SLOTS, SLOT(toggleActionsToolbar()), "toggle actions toolbar");

    actShowStatusBar = KStandardAction::showStatusbar(SLOTS, SLOT(toggleStatusbar()), krusaderApp->actionCollection());
    KStandardAction::quit(krusaderApp, SLOT(slotClose()), krusaderApp->actionCollection());
    KStandardAction::configureToolbars(SLOTS, SLOT(configToolbar()), krusaderApp->actionCollection());
    KStandardAction::keyBindings(SLOTS, SLOT(configKeys()), krusaderApp->actionCollection());

    // the toggle actions
    NEW_KTOGGLEACTION(actToggleFnkeys, i18n("Show &FN Keys Bar"), 0, 0, SLOTS,  SLOT(toggleFnkeys()), "toggle fn bar");

    NEW_KTOGGLEACTION(actToggleCmdline, i18n("Show &Command Line"), 0, 0, SLOTS, SLOT(toggleCmdline()), "toggle command line");

    NEW_KTOGGLEACTION(actToggleTerminal, i18n("Show Terminal &Emulator"), 0, Qt::ALT + Qt::CTRL + Qt::Key_T, SLOTS, SLOT(toggleTerminal()), "toggle terminal emulator");


    NEW_KTOGGLEACTION(actToggleHidden, i18n("Show &Hidden Files"), 0, Qt::CTRL + Qt::Key_Period, SLOTS, SLOT(toggleHidden()), "toggle hidden files");

    NEW_KACTION(actSwapPanels, i18n("S&wap Panels"), 0, Qt::CTRL + Qt::Key_U, SLOTS, SLOT(swapPanels()), "swap panels");

    NEW_KACTION(actEmptyTrash, i18n("Empty Trash"), "trash-empty", 0, SLOTS, SLOT(emptyTrash()), "emptytrash");

    NEW_KACTION(actTrashBin, i18n("Trash bin"), KrTrashHandler::trashIcon(), 0, SLOTS, SLOT(trashBin()), "trashbin");

    NEW_KACTION(actSwapSides, i18n("Sw&ap Sides"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_U, SLOTS, SLOT(toggleSwapSides()), "toggle swap sides");
    actToggleHidden->setChecked(KConfigGroup(krConfig, "Look&Feel").readEntry("Show Hidden", _ShowHidden));

    // and then the DONE actions
    NEW_KACTION(actCmdlinePopup, i18n("popup cmdline"), 0, Qt::CTRL + Qt::Key_Slash, SLOTS, SLOT(cmdlinePopup()), "cmdline popup");


    KAction *tmp2;
    NEW_KACTION(tmp2, i18n("Start &Root Mode Krusader"), "krusader_root", Qt::ALT + Qt::SHIFT + Qt::Key_K, SLOTS, SLOT(rootKrusader()), "root krusader");
    NEW_KACTION(actProfiles, i18n("Pro&files"), "user-identity", Qt::ALT + Qt::SHIFT + Qt::Key_L, MAIN_VIEW, SLOT(profiles()), "profile");
    NEW_KACTION(actSplit, i18n("Sp&lit File..."), "kr_split", Qt::CTRL + Qt::Key_P, SLOTS, SLOT(slotSplit()), "split");
    NEW_KACTION(actCombine, i18n("Com&bine Files..."), "kr_combine", Qt::CTRL + Qt::Key_B, SLOTS, SLOT(slotCombine()), "combine");
    NEW_KACTION(actSelectNewerAndSingle, i18n("&Select Newer and Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_newer_and_single");
    NEW_KACTION(actSelectNewer, i18n("Select &Newer"), 0, 0, SLOTS, SLOT(compareSetup()), "select_newer");
    NEW_KACTION(actSelectSingle, i18n("Select &Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_single");
    NEW_KACTION(actSelectDifferentAndSingle, i18n("Select Different &and Single"), 0, 0, SLOTS, SLOT(compareSetup()), "select_different_and_single");
    NEW_KACTION(actSelectDifferent,  i18n("Select &Different"), 0, 0, SLOTS, SLOT(compareSetup()), "select_different");
    actSelectNewerAndSingle->setCheckable(true);
    actSelectNewer->setCheckable(true);
    actSelectSingle->setCheckable(true);
    actSelectDifferentAndSingle->setCheckable(true);
    actSelectDifferent->setCheckable(true);
    QActionGroup *selectGroup = new QActionGroup(krusaderApp);
    selectGroup->setExclusive(true);
    selectGroup->addAction(actSelectNewerAndSingle);
    selectGroup->addAction(actSelectNewer);
    selectGroup->addAction(actSelectSingle);
    selectGroup->addAction(actSelectDifferentAndSingle);
    selectGroup->addAction(actSelectDifferent);
    if (compareMode < (int)(sizeof(compareArray) / sizeof(KAction **)) - 1)
        (*compareArray[ compareMode ])->setChecked(true);
    NEW_KACTION(actExecStartAndForget, i18n("Start and &Forget"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_start_and_forget");
    NEW_KACTION(actExecCollectSeparate, i18n("Display &Separated Standard and Error Output"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_collect_separate");
    NEW_KACTION(actExecCollectTogether, i18n("Display &Mixed Standard and Error Output"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_collect_together");
    NEW_KACTION(actExecTerminalExternal, i18n("Start in &New Terminal"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_terminal_external");
    NEW_KACTION(actExecTerminalEmbedded, i18n("Send to &Embedded Terminal Emulator"), 0, 0, SLOTS, SLOT(execTypeSetup()), "exec_terminal_embedded");
    actExecStartAndForget->setCheckable(true);
    actExecCollectSeparate->setCheckable(true);
    actExecCollectTogether->setCheckable(true);
    actExecTerminalExternal->setCheckable(true);
    actExecTerminalEmbedded->setCheckable(true);
    QActionGroup *actionGroup = new QActionGroup(krusaderApp);
    actionGroup->setExclusive(true);
    actionGroup->addAction(actExecStartAndForget);
    actionGroup->addAction(actExecCollectSeparate);
    actionGroup->addAction(actExecCollectTogether);
    actionGroup->addAction(actExecTerminalExternal);
    actionGroup->addAction(actExecTerminalEmbedded);
    if (cmdExecMode < (int)(sizeof(execTypeArray) / sizeof(KAction **)) - 1)
        (*execTypeArray[ cmdExecMode ])->setChecked(true);


    NEW_KACTION(actHomeTerminal, i18n("Start &Terminal"), "terminal", 0, SLOTS, SLOT(homeTerminal()), "terminal@home");

    actMountMan = krMtMan.action();
    actMountMan->setShortcut(Qt::ALT + Qt::Key_Slash);
    krusaderApp->actionCollection()->addAction("mountman", actMountMan);

    NEW_KACTION(actFind, i18n("&Search..."), "system-search", Qt::CTRL + Qt::Key_S, SLOTS, SLOT(search()), "find");
    NEW_KACTION(actLocate, i18n("&Locate..."), "edit-find", Qt::SHIFT + Qt::CTRL + Qt::Key_L, SLOTS, SLOT(locate()), "locate");
    NEW_KACTION(actSyncDirs, i18n("Synchronize &Directories..."), "kr_syncdirs", Qt::CTRL + Qt::Key_Y, SLOTS, SLOT(slotSynchronizeDirs()), "sync dirs");
    NEW_KACTION(actDiskUsage, i18n("D&isk Usage..."), "kr_diskusage", Qt::ALT + Qt::SHIFT + Qt::Key_S, SLOTS, SLOT(slotDiskUsage()), "disk usage");
    NEW_KACTION(actQueueManager, i18n("&Queue Manager..."), "document-multiple", Qt::ALT + Qt::SHIFT + Qt::Key_Q, SLOTS, SLOT(slotQueueManager()), "queue manager");
    NEW_KACTION(actKonfigurator, i18n("Configure &Krusader..."), "configure", 0, SLOTS, SLOT(startKonfigurator()), "konfigurator");
    NEW_KACTION(actSavePosition, i18n("Save &Position"), 0, 0, krusaderApp, SLOT(savePosition()), "save position");
    NEW_KACTION(actCompare, i18n("Compare b&y Content..."), "kmultiple", 0, SLOTS, SLOT(compareContent()), "compare");
    NEW_KACTION(actMultiRename, i18n("Multi &Rename..."), "krename", Qt::SHIFT + Qt::Key_F9, SLOTS, SLOT(multiRename()), "multirename");

    KAction *t3, *t4, *t5, *t6, *t7, *t8, *t9, *t10, *t11, *t12, *t13, *t14, *t15, *t16;
    NEW_KACTION(t7, i18n("Bookmark Current"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_D, SLOTS, SLOT(bookmarkCurrent()), "bookmark current");
    NEW_KACTION(actVerticalMode, i18n("Vertical Mode"), "view-split-top-bottom", Qt::ALT + Qt::CTRL + Qt::Key_R, MAIN_VIEW, SLOT(toggleVerticalMode()), "toggle vertical mode");
    NEW_KACTION(actNewTab, i18n("New Tab"), "tab-new", Qt::ALT + Qt::CTRL + Qt::Key_N, SLOTS, SLOT(newTab()), "new tab");
    NEW_KACTION(actDupTab, i18n("Duplicate Current Tab"), "tab_duplicate", Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_N, SLOTS, SLOT(duplicateTab()), "duplicate tab");
    NEW_KACTION(actCloseTab, i18n("Close Current Tab"), "tab-close", Qt::CTRL + Qt::Key_W, SLOTS, SLOT(closeTab()), "close tab");
    NEW_KACTION(actNextTab, i18n("Next Tab"), 0, Qt::SHIFT + Qt::Key_Right, SLOTS, SLOT(nextTab()), "next tab");
    NEW_KACTION(actPreviousTab, i18n("Previous Tab"), 0, Qt::SHIFT + Qt::Key_Left, SLOTS, SLOT(previousTab()), "previous tab");
    NEW_KACTION(actCloseInactiveTabs, i18n("Close Inactive Tabs"), 0, 0, SLOTS, SLOT(closeInactiveTabs()), "close inactive tabs");
    NEW_KACTION(actCloseDuplicatedTabs, i18n("Close Duplicated Tabs"), 0, 0, SLOTS, SLOT(closeDuplicatedTabs()), "close duplicated tabs");
    NEW_KACTION(actLockTab, i18n("Lock Tab"), 0, 0, SLOTS, SLOT(lockTab()), "lock tab");
#if 0
    actUserMenu = new KAction(i18n("User Menu"), ALT + Qt::Key_QuoteLeft, SLOTS,
                              SLOT(userMenu()), actionCollection(), "user menu");
#else
    actUserMenu = new KActionMenu(i18n("User&actions"), krusaderApp);
    krusaderApp->actionCollection()->addAction("useractionmenu", actUserMenu);
#endif
    NEW_KACTION(actManageUseractions, i18n("Manage User Actions..."), 0, 0, SLOTS, SLOT(manageUseractions()), "manage useractions");

    actRemoteEncoding = new KrRemoteEncodingMenu(i18n("Select Remote Charset"), "character-set", krusaderApp->actionCollection());

    NEW_KACTION(actF10, i18n("Quit"), 0, Qt::Key_F10, krusaderApp, SLOT(slotClose()) , "F10_Quit");
    NEW_KACTION(actPopularUrls, i18n("Popular URLs..."), 0, Qt::CTRL + Qt::Key_Z, krusaderApp->popularUrls(), SLOT(showDialog()), "Popular_Urls");
    NEW_KACTION(actSwitchFullScreenTE, i18n("Toggle Fullwidget Terminal Emulator"), 0, Qt::CTRL + Qt::Key_F, MAIN_VIEW, SLOT(switchFullScreenTE()), "switch_fullscreen_te");


    KAction *tmp;
    NEW_KACTION(tmp, i18n("Move Focus Up"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Up, MAIN_VIEW, SLOT(focusUp()), "move_focus_up");
    NEW_KACTION(tmp, i18n("Move Focus Down"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Down, MAIN_VIEW, SLOT(focusDown()), "move_focus_down");

    // and at last we can set the tool-tips
    actKonfigurator->setToolTip(i18n("Setup Krusader the way you like it"));
    actFind->setToolTip(i18n("Search for files"));

    // setup all UserActions
    krUserAction = new UserAction();

#ifdef __KJSEMBED__
    actShowJSConsole = new KAction(i18n("JavaScript Console..."), Qt::ALT + Qt::CTRL + Qt::Key_J, SLOTS, SLOT(jsConsole()), krusaderApp->actionCollection(), "JS_Console");
#endif

}
