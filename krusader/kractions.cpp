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

// QtWidgets
#include <QMenu>
#include <QAction>

#include <KI18n/KLocalizedString>
#include <KXmlGui/KActionCollection>
#include <KWidgetsAddons/KToggleAction>
#include <KWidgetsAddons/KToolBarPopupAction>
#include <KXmlGui/KToggleToolBarAction>
#include <KXmlGui/KXMLGUIFactory>

#include "defaults.h"
#include "krusader.h"
#include "krusaderview.h"
#include "krslots.h"
#include "krtrashhandler.h"
#include "Dialogs/popularurls.h"
#include "GUI/krremoteencodingmenu.h"
#include "JobMan/jobman.h"
#include "MountMan/kmountman.h"
#include "Panel/PanelView/krviewfactory.h"
#include "UserAction/useraction.h"

QAction *KrActions::actCompare = 0;
QAction *KrActions::actDiskUsage = 0;
QAction *KrActions::actHomeTerminal = 0;
QAction *KrActions::actRemoteEncoding = 0;
QAction *KrActions::actProfiles = 0;
QAction *KrActions::actMultiRename = 0;
QAction *KrActions::actMountMan = 0;
QAction *KrActions::actNewTool = 0;
QAction *KrActions::actKonfigurator = 0;
QAction *KrActions::actToolsSetup = 0;
QAction *KrActions::actSwapPanels = 0;
QAction *KrActions::actSwapSides = 0;
QAction *KrActions::actFind = 0;
QAction *KrActions::actLocate = 0;
QAction *KrActions::actSwitchFullScreenTE = 0;
QAction *KrActions::actAddBookmark = 0;
QAction *KrActions::actSavePosition = 0;
QAction *KrActions::actSelectColorMask = 0;
QAction *KrActions::actOpenLeftBm = 0;
QAction *KrActions::actOpenRightBm = 0;
QAction *KrActions::actCmdlinePopup = 0;
QAction *KrActions::actSplit = 0;
QAction *KrActions::actCombine = 0;
QAction *KrActions::actUserMenu = 0;
QAction *KrActions::actManageUseractions = 0;
#ifdef SYNCHRONIZER_ENABLED
QAction *KrActions::actSyncDirs = 0;
#endif
QAction *KrActions::actF10Quit = 0;
QAction *KrActions::actEmptyTrash = 0;
QAction *KrActions::actTrashBin = 0;
QAction *KrActions::actPopularUrls = 0;

KToggleAction *KrActions::actToggleTerminal = 0;
QAction  *KrActions::actVerticalMode = 0;
QAction  *KrActions::actSelectNewerAndSingle = 0;
QAction  *KrActions::actSelectSingle = 0;
QAction  *KrActions::actSelectNewer = 0;
QAction  *KrActions::actSelectDifferentAndSingle = 0;
QAction  *KrActions::actSelectDifferent = 0;
QAction  **KrActions::compareArray[] = {&actSelectNewerAndSingle, &actSelectNewer, &actSelectSingle,
                                       &actSelectDifferentAndSingle, &actSelectDifferent, 0
                                      };
QAction *KrActions::actExecStartAndForget = 0;
QAction *KrActions::actExecCollectSeparate = 0;
QAction *KrActions::actExecCollectTogether = 0;
QAction *KrActions::actExecTerminalExternal = 0;
QAction *KrActions::actExecTerminalEmbedded = 0;
QAction **KrActions::execTypeArray[] = {&actExecStartAndForget, &actExecCollectSeparate, &actExecCollectTogether,
                                       &actExecTerminalExternal, &actExecTerminalEmbedded, 0
                                      };
KToggleAction *KrActions::actToggleFnkeys = 0;
KToggleAction *KrActions::actToggleCmdline = 0;
KToggleAction *KrActions::actShowStatusBar = 0;
KToggleAction *KrActions::actToggleHidden = 0;
KToggleAction *KrActions::actCompareDirs = 0;

QAction *KrActions::actJobProgress = 0;
QAction *KrActions::actJobControl = 0;
QAction *KrActions::actJobMode = 0;
QAction *KrActions::actJobUndo = 0;

#ifdef __KJSEMBED__
    static QAction *actShowJSConsole;
#endif


QAction *createAction(QString text, QString icon, QKeySequence shortcut,
                      QObject *recv, const char *slot, QString name, Krusader *krusaderApp)
{
    QAction *a;
    if (icon.isEmpty())
        a = new QAction(text, krusaderApp);
    else
        a = new QAction(QIcon::fromTheme(icon), text, krusaderApp);
    krusaderApp->connect(a, SIGNAL(triggered(bool)), recv, slot);
    krusaderApp->actionCollection()->addAction(name, a);
    krusaderApp->actionCollection()->setDefaultShortcut(a, shortcut);
    return a;
}

QAction *createAction(QString text, QString icon, QList<QKeySequence> shortcuts,
                      QObject *recv, const char *slot, QString name, Krusader *krusaderApp)
{
    QAction *a;
    if (icon.isEmpty())
        a = new QAction(text, krusaderApp);
    else
        a = new QAction(QIcon::fromTheme(icon), text, krusaderApp);
    krusaderApp->connect(a, SIGNAL(triggered(bool)), recv, slot);
    krusaderApp->actionCollection()->addAction(name, a);
    krusaderApp->actionCollection()->setDefaultShortcuts(a, shortcuts);
    return a;
}


KToggleAction *createToggleAction(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, Krusader *krusaderApp)
{
    KToggleAction *a;
    if (icon == 0)
        a = new KToggleAction(text, krusaderApp);
    else
        a = new KToggleAction(QIcon::fromTheme(icon), text, krusaderApp);
    krusaderApp->connect(a, SIGNAL(triggered(bool)), recv, slot);
    krusaderApp->actionCollection()->addAction(name, a);
    krusaderApp->actionCollection()->setDefaultShortcut(a, shortcut);
    return a;
}


void KrActions::setupActions(Krusader *krusaderApp)
{

#define NEW_KACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krusaderApp);

#define NEW_KTOGGLEACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createToggleAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krusaderApp);


    // first come the TODO actions
    //actSync =       0;//new QAction(i18n("S&ynchronize Dirs"),                         0, krusaderApp, 0, actionCollection(), "sync dirs");
    //actNewTool =    0;//new QAction(i18n("&Add a new tool"),                          0, krusaderApp, 0, actionCollection(), "add tool");
    //actToolsSetup = 0;//new QAction(i18n("&Tools Menu Setup"),                        0, 0, krusaderApp, 0, actionCollection(), "tools setup");
    //KStandardAction::print(SLOTS, 0,actionCollection(),"std_print");
    //KStandardAction::showMenubar( SLOTS, SLOT(showMenubar()), actionCollection(), "std_menubar" );

    /* Shortcut disabled because of the Terminal Emulator bug. */
    KConfigGroup group(krConfig, "Private");
    int compareMode = group.readEntry("Compare Mode", 0);
    int cmdExecMode =  group.readEntry("Command Execution Mode", 0);
    QAction *tmp;
    Q_UNUSED(tmp);

    NEW_KACTION(tmp, i18n("Tab-Switch panel"), 0, Qt::Key_Tab, MAIN_VIEW, SLOT(panelSwitch()), "tab");

    KToggleToolBarAction *actShowToolBar = new KToggleToolBarAction(krusaderApp->toolBar(), i18n("Show Main Toolbar"), krusaderApp);
    krusaderApp->actionCollection()->addAction(KStandardAction::name(KStandardAction::ShowToolbar), actShowToolBar);

    KToggleToolBarAction *actShowJobToolBar = new KToggleToolBarAction(krusaderApp->toolBar("jobToolBar"), i18n("Show Job Toolbar"), krusaderApp);
    krusaderApp->actionCollection()->addAction("toggle show jobbar", actShowJobToolBar);

    KToggleToolBarAction *actShowActionsToolBar = new KToggleToolBarAction(krusaderApp->toolBar("actionsToolBar"), i18n("Show Actions Toolbar"), krusaderApp);
    krusaderApp->actionCollection()->addAction("toggle actions toolbar", actShowActionsToolBar);

    actShowStatusBar = KStandardAction::showStatusbar(SLOTS, SLOT(updateStatusbarVisibility()), krusaderApp->actionCollection());
    KStandardAction::quit(krusaderApp, SLOT(quit()), krusaderApp->actionCollection());
    KStandardAction::configureToolbars(krusaderApp, SLOT(configureToolbars()), krusaderApp->actionCollection());
    KStandardAction::keyBindings(krusaderApp->guiFactory(), SLOT(configureShortcuts()), krusaderApp->actionCollection());

    // the toggle actions
    NEW_KTOGGLEACTION(actToggleFnkeys, i18n("Show &FN Keys Bar"), 0, 0, SLOTS,  SLOT(toggleFnkeys()), "toggle fn bar");

    NEW_KTOGGLEACTION(actToggleCmdline, i18n("Show &Command Line"), 0, 0, SLOTS, SLOT(toggleCmdline()), "toggle command line");

    NEW_KTOGGLEACTION(actToggleTerminal, i18n("Show &Embedded Terminal"), 0, Qt::ALT + Qt::CTRL + Qt::Key_T, SLOTS, SLOT(toggleTerminal()), "toggle terminal emulator");


    NEW_KTOGGLEACTION(actToggleHidden, i18n("Show &Hidden Files"), 0, Qt::ALT + Qt::Key_Period, SLOTS, SLOT(showHiddenFiles(bool)), "toggle hidden files");

    NEW_KACTION(actSwapPanels, i18n("S&wap Panels"), 0, Qt::CTRL + Qt::Key_U, SLOTS, SLOT(swapPanels()), "swap panels");

    NEW_KACTION(actEmptyTrash, i18n("Empty Trash"), "trash-empty", 0, SLOTS, SLOT(emptyTrash()), "emptytrash");

    NEW_KACTION(actTrashBin, i18n("Trash Popup Menu"), KrTrashHandler::trashIcon(), 0, SLOTS, SLOT(trashPopupMenu()), "trashbin");

    NEW_KACTION(actSwapSides, i18n("Sw&ap Sides"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_U, SLOTS, SLOT(toggleSwapSides()), "toggle swap sides");
    actToggleHidden->setChecked(KConfigGroup(krConfig, "Look&Feel").readEntry("Show Hidden", _ShowHidden));

    // and then the DONE actions
    NEW_KACTION(actCmdlinePopup, i18n("popup cmdline"), 0, Qt::CTRL + Qt::Key_Slash, SLOTS, SLOT(cmdlinePopup()), "cmdline popup");


    NEW_KACTION(tmp, i18n("Start &Root Mode Krusader"), "krusader_root", Qt::ALT + Qt::SHIFT + Qt::Key_K, SLOTS, SLOT(rootKrusader()), "root krusader");
    NEW_KACTION(actProfiles, i18n("Pro&files"), "user-identity", Qt::ALT + Qt::SHIFT + Qt::Key_L, MAIN_VIEW, SLOT(profiles()), "profile");
    NEW_KACTION(actSplit, i18n("Sp&lit File..."), "split", Qt::CTRL + Qt::Key_P, SLOTS, SLOT(slotSplit()), "split");
    NEW_KACTION(actCombine, i18n("Com&bine Files..."), "kr_combine", 0, SLOTS, SLOT(slotCombine()), "combine");
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
    if (compareMode < (int)(sizeof(compareArray) / sizeof(QAction **)) - 1)
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
    if (cmdExecMode < (int)(sizeof(execTypeArray) / sizeof(QAction **)) - 1)
        (*execTypeArray[ cmdExecMode ])->setChecked(true);


    NEW_KACTION(actHomeTerminal, i18n("Start &Terminal"), "utilities-terminal", 0, SLOTS, SLOT(homeTerminal()), "terminal@home");

    actMountMan = krMtMan.action();
    krusaderApp->actionCollection()->addAction("mountman", actMountMan);
    krusaderApp->actionCollection()->setDefaultShortcut(actMountMan, Qt::ALT + Qt::Key_Slash);

    NEW_KACTION(actFind, i18n("&Search..."), "system-search", Qt::CTRL + Qt::Key_S, SLOTS, SLOT(search()), "find");
    NEW_KACTION(actLocate, i18n("&Locate..."), "edit-find", Qt::SHIFT + Qt::CTRL + Qt::Key_L, SLOTS, SLOT(locate()), "locate");
#ifdef SYNCHRONIZER_ENABLED
    NEW_KACTION(actSyncDirs, i18n("Synchronize Fol&ders..."), "folder-sync", Qt::CTRL + Qt::Key_Y, SLOTS, SLOT(slotSynchronizeDirs()), "sync dirs");
#endif
    NEW_KACTION(actDiskUsage, i18n("D&isk Usage..."), "kr_diskusage", Qt::ALT + Qt::SHIFT + Qt::Key_S, SLOTS, SLOT(slotDiskUsage()), "disk usage");
    NEW_KACTION(actKonfigurator, i18n("Configure &Krusader..."), "configure", 0, SLOTS, SLOT(startKonfigurator()), "konfigurator");
    NEW_KACTION(actSavePosition, i18n("Save &Position"), 0, 0, krusaderApp, SLOT(savePosition()), "save position");
    NEW_KACTION(actCompare, i18n("Compare b&y Content..."), "kr_comparedirs", 0, SLOTS, SLOT(compareContent()), "compare");
    NEW_KACTION(actMultiRename, i18n("Multi &Rename..."), "edit-rename", Qt::SHIFT + Qt::Key_F9, SLOTS, SLOT(multiRename()), "multirename");

    NEW_KACTION(actAddBookmark, i18n("Add Bookmark"), "bookmark-new", KStandardShortcut::addBookmark(), SLOTS, SLOT(addBookmark()), "add bookmark");
    NEW_KACTION(actVerticalMode, i18n("Vertical Mode"), "view-split-top-bottom", Qt::ALT + Qt::CTRL + Qt::Key_R, MAIN_VIEW, SLOT(toggleVerticalMode()), "toggle vertical mode");
    actUserMenu = new KActionMenu(i18n("User&actions"), krusaderApp);
    krusaderApp->actionCollection()->addAction("useractionmenu", actUserMenu);
    NEW_KACTION(actManageUseractions, i18n("Manage User Actions..."), 0, 0, SLOTS, SLOT(manageUseractions()), "manage useractions");

    actRemoteEncoding = new KrRemoteEncodingMenu(i18n("Select Remote Charset"), "character-set", krusaderApp->actionCollection());

    NEW_KACTION(actF10Quit, i18n("Quit"), 0, Qt::Key_F10, krusaderApp, SLOT(quit()) , "F10_Quit");
    actF10Quit->setToolTip(i18n("Quit Krusader."));

    NEW_KACTION(actPopularUrls, i18n("Popular URLs..."), 0, Qt::CTRL + Qt::Key_Z, krusaderApp->popularUrls(), SLOT(showDialog()), "Popular_Urls");
    NEW_KACTION(actSwitchFullScreenTE, i18n("Toggle Fullscreen Embedded Terminal"), 0,
                Qt::CTRL + Qt::ALT + Qt::Key_F, MAIN_VIEW, SLOT(toggleFullScreenTerminalEmulator()),
                "switch_fullscreen_te");

    NEW_KACTION(tmp, i18n("Move Focus Up"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Up, MAIN_VIEW, SLOT(focusUp()), "move_focus_up");
    NEW_KACTION(tmp, i18n("Move Focus Down"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Down, MAIN_VIEW, SLOT(focusDown()), "move_focus_down");

    // job manager actions
    actJobControl = krJobMan->controlAction();
    krusaderApp->actionCollection()->addAction("job control", actJobControl);
    krusaderApp->actionCollection()->setDefaultShortcut(actJobControl, Qt::CTRL + Qt::ALT + Qt::Key_P);
    actJobProgress = krJobMan->progressAction();
    krusaderApp->actionCollection()->addAction("job progress", actJobProgress);
    actJobMode = krJobMan->modeAction();
    krusaderApp->actionCollection()->addAction("job mode", actJobMode);
    actJobUndo = krJobMan->undoAction();
    krusaderApp->actionCollection()->addAction("job undo", actJobUndo);
    krusaderApp->actionCollection()->setDefaultShortcut(actJobUndo, Qt::CTRL + Qt::ALT + Qt::Key_Z);

    // and at last we can set the tool-tips
    actKonfigurator->setToolTip(i18n("Setup Krusader the way you like it"));
    actFind->setToolTip(i18n("Search for files"));

    // setup all UserActions
    krUserAction = new UserAction();

#ifdef __KJSEMBED__
    actShowJSConsole = new QAction(i18n("JavaScript Console..."), Qt::ALT + Qt::CTRL + Qt::Key_J, SLOTS, SLOT(jsConsole()), krusaderApp->actionCollection(), "JS_Console");
#endif

}
