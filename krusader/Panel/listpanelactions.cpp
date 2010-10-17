/***************************************************************************
                          listpanelactions.cpp
                       -------------------
copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
copyright            : (C) 2010 by Jan Lepper
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

#include "listpanelactions.h"

#include "listpanel.h"
#include "panelfunc.h"
#include "krviewfactory.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Dialogs/krdialogs.h"
#include "../KViewer/krviewer.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <ktoggleaction.h>


ListPanelActions *ListPanelActions::self = 0;

KAction *ListPanelActions::actCompDirs = 0;
KAction *ListPanelActions::actSync = 0;
KAction *ListPanelActions::actProperties = 0;
KAction *ListPanelActions::actPack = 0;
KAction *ListPanelActions::actUnpack = 0;
KAction *ListPanelActions::actTest = 0;
KAction *ListPanelActions::actCopy = 0;
KAction *ListPanelActions::actPaste = 0;
KAction *ListPanelActions::actCalculate = 0;
KAction *ListPanelActions::actCreateChecksum = 0;
KAction *ListPanelActions::actMatchChecksum = 0;
KAction *ListPanelActions::actSelect = 0;
KAction *ListPanelActions::actSelectAll = 0;
KAction *ListPanelActions::actUnselect = 0;
KAction *ListPanelActions::actUnselectAll = 0;
KAction *ListPanelActions::actInvert = 0;
KAction *ListPanelActions::actFTPConnect = 0;
KAction *ListPanelActions::actFTPNewConnect = 0;
KAction *ListPanelActions::actFTPDisconnect = 0;
KAction *ListPanelActions::actAllFilter = 0;
// KAction *ListPanelActions::actExecFilter = 0;
KAction *ListPanelActions::actCustomFilter = 0;
KAction *ListPanelActions::actRoot = 0;
KAction *ListPanelActions::actDirUp = 0;
KAction *ListPanelActions::actSyncBrowse = 0;
KAction *ListPanelActions::actF2 = 0;
KAction *ListPanelActions::actF3 = 0;
KAction *ListPanelActions::actF4 = 0;
KAction *ListPanelActions::actF5 = 0;
KAction *ListPanelActions::actF6 = 0;
KAction *ListPanelActions::actF7 = 0;
KAction *ListPanelActions::actF8 = 0;
KAction *ListPanelActions::actF9 = 0;
KAction *ListPanelActions::actShiftF5 = 0;
KAction *ListPanelActions::actShiftF6 = 0;
KAction *ListPanelActions::actLocationBar = 0;
KAction *ListPanelActions::actJumpBack = 0;
KAction *ListPanelActions::actSetJumpBack = 0;
KAction *ListPanelActions::actView0 = 0;
KAction *ListPanelActions::actView1 = 0;
KAction *ListPanelActions::actView2 = 0;
KAction *ListPanelActions::actView3 = 0;
KAction *ListPanelActions::actView4 = 0;
KAction *ListPanelActions::actView5 = 0;
KAction *ListPanelActions::actHistoryBackward = 0;
KAction *ListPanelActions::actHistoryForward = 0;
KAction *ListPanelActions::actCancelRefresh = 0;



KAction *createAction(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, QObject *parent, KActionCollection *actionCollection)
{
    KAction *a;
    if (icon == 0)
        a = new KAction(text, parent);
    else
        a = new KAction(KIcon(icon), text, parent);
    a->setShortcut(shortcut);
    parent->connect(a, SIGNAL(triggered(bool)), recv, slot);
    actionCollection->addAction(name, a);
    return a;
}

KToggleAction *createToggleAction(QString text, QString icon, QKeySequence shortcut,
                                 QObject *recv, const char *slot, QString name, QObject *parent, KActionCollection *actionCollection)
{
    KToggleAction *a;
    if (icon == 0)
        a = new KToggleAction(text, parent);
    else
        a = new KToggleAction(KIcon(icon), text, parent);
    a->setShortcut(shortcut);
    parent->connect(a, SIGNAL(triggered(bool)), recv, slot);
    actionCollection->addAction(name, a);
    return a;
}


ListPanelActions::ListPanelActions(QObject *parent) : QObject(parent)
{
    if(self)
        abort();
    self = this;

#define NEW_KACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krApp, actionCollection);

#define NEW_KTOGGLEACTION(VAR, TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME) \
    VAR = createToggleAction(TEXT, ICON_NAME, SHORTCUT, RECV_OBJ, SLOT_NAME, NAME, krApp, actionCollection);

    KActionCollection *actionCollection = krApp->actionCollection();

    // set view type
    QList<KrViewInstance *> views = KrViewFactory::registeredViews();
    foreach(KrViewInstance * inst, views) {
        int id = inst->id();

        switch (id) {
        case 0:
            NEW_KACTION(actView0, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView0()), "view0");
            break;
        case 1:
            NEW_KACTION(actView1, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView1()), "view1");
            break;
        case 2:
            NEW_KACTION(actView2, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView2()), "view2");
            break;
        case 3:
            NEW_KACTION(actView3, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView3()), "view3");
            break;
        case 4:
            NEW_KACTION(actView4, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView4()), "view4");
            break;
        case 5:
            NEW_KACTION(actView5, inst->description(), inst->icon(), inst->shortcut(), this, SLOT(setView5()), "view5");
            break;
        default:
            break;
        }
    }

    // standard actions
    actHistoryBackward = KStandardAction::back(this, SLOT(historyBackward()), parent);
    actionCollection->addAction("history_backward", actHistoryBackward);
    actHistoryForward = KStandardAction::forward(this, SLOT(historyForward()), parent);
    actionCollection->addAction("history_forward", actHistoryForward);
    KStandardAction::home(this, SLOT(home()), actionCollection/*, "std_home"*/)->setText(i18n("Home")); /*->setShortcut(Qt::Key_QuoteLeft);*/
    KStandardAction::cut(this, SLOT(cut()), actionCollection)->setText(i18n("Cut to Clipboard"));
    (actCopy = KStandardAction::copy(this, SLOT(copy()), actionCollection))->setText(i18n("Copy to Clipboard"));
    (actPaste = KStandardAction::paste(this, SLOT(paste()), actionCollection))->setText(i18n("Paste from Clipboard"));

    KAction *tmp;

    // Fn keys
    NEW_KACTION(actF2, i18n("Start Terminal Here"), "terminal", Qt::Key_F2, this, SLOT(terminal()) , "F2_Terminal");
    NEW_KACTION(actF3, i18n("View File"), 0, Qt::Key_F3, this, SLOT(view()), "F3_View");
    NEW_KACTION(actF4, i18n("Edit File"), 0, Qt::Key_F4, this, SLOT(edit()) , "F4_Edit");
    NEW_KACTION(actF5, i18n("Copy..."), 0, Qt::Key_F5, this, SLOT(copyFiles()) , "F5_Copy");
    NEW_KACTION(actF6, i18n("Move..."), 0, Qt::Key_F6, this, SLOT(moveFiles()) , "F6_Move");
    NEW_KACTION(actShiftF5, i18n("Copy by queue..."), 0, Qt::SHIFT + Qt::Key_F5, this, SLOT(copyFilesByQueue()) , "F5_Copy_Queue");
    NEW_KACTION(actShiftF6, i18n("Move by queue..."), 0, Qt::SHIFT + Qt::Key_F6, this, SLOT(moveFilesByQueue()) , "F6_Move_Queue");
    NEW_KACTION(actF7, i18n("New Directory..."), "folder-new", Qt::Key_F7, this, SLOT(mkdir()) , "F7_Mkdir");
    NEW_KACTION(actF8, i18n("Delete"), "edit-delete", Qt::Key_F8, this, SLOT(deleteFiles()) , "F8_Delete");
    NEW_KACTION(actF9, i18n("Rename"), 0, Qt::Key_F9, this, SLOT(rename()) , "F9_Rename");
    NEW_KACTION(tmp, i18n("&New Text File..."), "document-new", Qt::SHIFT + Qt::Key_F4, this, SLOT(editDlg()), "edit_new_file");
    NEW_KACTION(tmp, i18n("F3 View Dialog"), 0, Qt::SHIFT + Qt::Key_F3, this, SLOT(viewDlg()), "F3_ViewDlg");

    // filter
    NEW_KACTION(actAllFilter, i18n("&All Files"), 0, Qt::SHIFT + Qt::Key_F10, this, SLOT(allFilter()), "all files");
    //actExecFilter = new KAction( i18n( "&Executables" ), SHIFT + Qt::Key_F11,
    //                             SLOTS, SLOT( execFilter() ), actionCollection(), "exec files" );
    NEW_KACTION(actCustomFilter, i18n("&Custom"), 0, Qt::SHIFT + Qt::Key_F12, this, SLOT(customFilter()), "custom files");

    // selection
    NEW_KACTION(actSelect, i18n("Select &Group..."), "kr_select", Qt::CTRL + Qt::Key_Plus, this, SLOT(markGroup()), "select group");
    NEW_KACTION(actSelectAll, i18n("&Select All"), "kr_selectall", Qt::ALT + Qt::Key_Plus, this, SLOT(markAll()), "select all");
    NEW_KACTION(actUnselect, i18n("&Unselect Group..."), "kr_unselect", Qt::CTRL + Qt::Key_Minus, this, SLOT(unmarkGroup()), "unselect group");
    NEW_KACTION(actUnselectAll, i18n("U&nselect All"), "kr_unselectall", Qt::ALT + Qt::Key_Minus, this, SLOT(unmarkAll()), "unselect all");
    NEW_KACTION(actInvert, i18n("&Invert Selection"), "kr_invert", Qt::ALT + Qt::Key_Asterisk, this, SLOT(invert()), "invert");

    // file operations
    NEW_KACTION(tmp, i18n("Right-click Menu"), 0, Qt::Key_Menu, this, SLOT(rightclickMenu()), "rightclick menu");
    NEW_KACTION(actProperties, i18n("&Properties..."), 0, Qt::ALT + Qt::Key_Enter, this, SLOT(properties()), "properties");
    NEW_KACTION(actCompDirs, i18n("&Compare Directories"), "view_left_right", Qt::ALT + Qt::SHIFT + Qt::Key_C, this, SLOT(compareDirs()), "compare dirs");
    NEW_KACTION(actCalculate, i18n("Calculate &Occupied Space"), "kcalc", 0, this, SLOT(calcSpace()), "calculate");
    NEW_KACTION(actPack, i18n("Pac&k..."), "archive-insert", Qt::ALT + Qt::SHIFT + Qt::Key_P, this, SLOT(slotPack()), "pack");
    NEW_KACTION(actUnpack, i18n("&Unpack..."), "archive-extract", Qt::ALT + Qt::SHIFT + Qt::Key_U, this, SLOT(slotUnpack()), "unpack");
    NEW_KACTION(actCreateChecksum, i18n("Create Checksum..."), "binary", 0, this, SLOT(createChecksum()), "create checksum");
    NEW_KACTION(actMatchChecksum, i18n("Verify Checksum..."), "binary", 0, this, SLOT(matchChecksum()), "match checksum");
    NEW_KACTION(tmp, i18n("New Symlink..."), 0, Qt::CTRL + Qt::ALT + Qt::Key_S, this, SLOT(newSymlink()), "new symlink");
    NEW_KACTION(actTest, i18n("T&est Archive"), "utilities-file-archiver", Qt::ALT + Qt::SHIFT + Qt::Key_E, this, SLOT(testArchive()), "test archives");

    // navigation
    NEW_KACTION(actRoot, i18n("Root"), "go-top", Qt::CTRL + Qt::Key_Backspace, this, SLOT(root()), "root");
//PORTME: second shortcut for up: see actDirUp
    //   KStandardAction::up( this, SLOT( dirUp() ), actionCollection )->setShortcut(Qt::Key_Backspace);
    /* Shortcut disabled because of the Terminal Emulator bug. */
    NEW_KACTION(actDirUp, i18n("Up"), "go-up", Qt::CTRL + Qt::Key_PageUp /*Qt::Key_Backspace*/, this, SLOT(dirUp()), "dirUp");
    KAction *reloadAct;
    NEW_KACTION(reloadAct, i18n("&Reload"), "view-refresh", Qt::CTRL + Qt::Key_R, this, SLOT(refresh()), "std_redisplay");
    NEW_KACTION(actCancelRefresh, i18n("Cancel Refresh of View"), "kr_cancel_refresh", 0, this, SLOT(cancelRefresh()), "cancel refresh");
    NEW_KACTION(actFTPNewConnect, i18n("New Net &Connection..."), "network-connect", Qt::CTRL + Qt::Key_N, this, SLOT(newFTPconnection()), "ftp new connection");
    NEW_KACTION(actFTPDisconnect, i18n("Disconnect &from Net"), "network-disconnect", Qt::SHIFT + Qt::CTRL + Qt::Key_F, this, SLOT(FTPDisconnect()), "ftp disconnect");
    NEW_KACTION(tmp, i18n("Sync Panels"), 0, Qt::ALT + Qt::SHIFT + Qt::Key_O, this, SLOT(syncPanels()), "sync panels");
    NEW_KACTION(actJumpBack, i18n("Jump Back"), "kr_jumpback", Qt::CTRL + Qt::Key_J, this, SLOT(slotJumpBack()), "jump_back");
    NEW_KACTION(actSetJumpBack, i18n("Set Jump Back Point"), "kr_setjumpback", Qt::CTRL + Qt::SHIFT + Qt::Key_J, this, SLOT(slotSetJumpBack()), "set_jump_back");
    NEW_KACTION(actSyncBrowse, i18n("S&ynchron Directory Changes"), "kr_syncbrowse_off", Qt::ALT + Qt::SHIFT + Qt::Key_Y, this, SLOT(slotSyncBrowse()), "sync browse");
    NEW_KACTION(actLocationBar, i18n("Go to Location Bar"), 0, Qt::CTRL + Qt::Key_L, this, SLOT(slotLocationBar()), "location_bar");
    NEW_KTOGGLEACTION(tmp, i18n("Toggle Popup Panel"), 0, Qt::ALT + Qt::Key_Down, this, SLOT(togglePopupPanel()), "toggle popup panel");
    NEW_KACTION(tmp, i18n("Bookmarks"), 0, Qt::CTRL + Qt::Key_D, this, SLOT(openBookmarks()), "bookmarks");
    NEW_KACTION(tmp, i18n("Left Bookmarks"), 0, 0, this, SLOT(openLeftBookmarks()), "left bookmarks");
    NEW_KACTION(tmp, i18n("Right Bookmarks"), 0, 0, this, SLOT(openRightBookmarks()), "right bookmarks");
    NEW_KACTION(tmp, i18n("History"), 0, Qt::CTRL + Qt::Key_H, this, SLOT(openHistory()), "history");
    NEW_KACTION(tmp, i18n("Left History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Left, this, SLOT(openLeftHistory()), "left history");
    NEW_KACTION(tmp, i18n("Right History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Right, this, SLOT(openRightHistory()), "right history");
    NEW_KACTION(tmp, i18n("Media"), 0, Qt::CTRL + Qt::Key_M, this, SLOT(openMedia()), "media");
    NEW_KACTION(tmp, i18n("Left Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Left, this, SLOT(openLeftMedia()), "left media");
    NEW_KACTION(tmp, i18n("Right Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Right, this, SLOT(openRightMedia()), "right media");

    // and at last we can set the tool-tips
    actSelect->setToolTip(i18n("Select files using a filter"));
    actSelectAll->setToolTip(i18n("Select all files in the current directory"));
    actUnselectAll->setToolTip(i18n("Unselect all selected files"));
    actRoot->setToolTip(i18n("ROOT (/)"));
}

// set view type

void ListPanelActions::setView0()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 0)
        ACTIVE_PANEL->gui->changeType(0);
}

void ListPanelActions::setView1()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 1)
        ACTIVE_PANEL->gui->changeType(1);
}

void ListPanelActions::setView2()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 2)
        ACTIVE_PANEL->gui->changeType(2);
}

void ListPanelActions::setView3()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 3)
        ACTIVE_PANEL->gui->changeType(3);
}

void ListPanelActions::setView4()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 4)
        ACTIVE_PANEL->gui->changeType(4);
}

void ListPanelActions::setView5()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->gui->getType() != 5)
        ACTIVE_PANEL->gui->changeType(5);
}

// fn keys

// F2
void ListPanelActions::terminal()
{
    ACTIVE_FUNC->terminal();
}
// F3
void ListPanelActions::view()
{
    ACTIVE_FUNC->view();
}
// F4
void ListPanelActions::edit()
{
    ACTIVE_FUNC->editFile();
}
// F5
void ListPanelActions::copyFiles()
{
    ACTIVE_FUNC->copyFiles();
}
// F6
void ListPanelActions::moveFiles()
{
    ACTIVE_FUNC->moveFiles();
}
// SHIFT + F5
void ListPanelActions::copyFilesByQueue()
{
    ACTIVE_FUNC->copyFiles(true);
}
// SHIFT + F6
void ListPanelActions::moveFilesByQueue()
{
    ACTIVE_FUNC->moveFiles(true);
}
// F7
void ListPanelActions::mkdir()
{
    ACTIVE_FUNC->mkdir();
}
// F8
void ListPanelActions::deleteFiles(bool reallyDelete)
{
    ACTIVE_FUNC->deleteFiles(reallyDelete);
}
// F9
void ListPanelActions::rename()
{
    ACTIVE_FUNC->rename();
}
// Shift F3
void ListPanelActions::viewDlg()
{
    // ask the user for a url to view
    KUrl dest = KChooseDir::getDir(i18n("Enter a URL to view:"), ACTIVE_PANEL->virtualPath(), ACTIVE_PANEL->virtualPath());
    if (dest.isEmpty()) return ;   // the user canceled

    KrViewer::view(dest);   // view the file
//  }
    // nothing more to it!
}
// Shift F4
void ListPanelActions::editDlg()
{
    ACTIVE_FUNC->editNewFile();
}

// filter

void ListPanelActions::allFilter()
{
    ACTIVE_PANEL->gui->setFilter(KrViewProperties::All);
}
#if 0
void ListPanelActions::execFilter()
{
    ACTIVE_PANEL->gui->setFilter(KrViewProperties::All);
}
#endif
void ListPanelActions::customFilter()
{
    ACTIVE_PANEL->gui->setFilter(KrViewProperties::Custom);
}

// selection

void ListPanelActions::markAll()
{
    ACTIVE_PANEL->gui->select(true, true);
}

void ListPanelActions::unmarkAll()
{
    ACTIVE_PANEL->gui->select(false, true);
}

void ListPanelActions::markGroup()
{
    ACTIVE_PANEL->gui->select(true, false);
}

void ListPanelActions::markGroup(const QString& mask, bool select)
{
    ACTIVE_PANEL->gui->select(KRQuery(mask), select);
}

void ListPanelActions::unmarkGroup()
{
    ACTIVE_PANEL->gui->select(false, false);
}

void ListPanelActions::invert()
{
    ACTIVE_PANEL->gui->invertSelection();
}

// file operations

void ListPanelActions::cut()
{
    ACTIVE_FUNC->copyToClipboard(true);
}

void ListPanelActions::copy()
{
    ACTIVE_FUNC->copyToClipboard(false);
}

void ListPanelActions::paste()
{
    ACTIVE_FUNC->pasteFromClipboard();
}

void ListPanelActions::createChecksum()
{
    ACTIVE_FUNC->createChecksum();
}

void ListPanelActions::matchChecksum()
{
    ACTIVE_FUNC->matchChecksum();
}

void ListPanelActions::compareDirs()
{
    ACTIVE_PANEL->gui->compareDirs();
    ACTIVE_PANEL->otherPanel()->gui->compareDirs();
}

void ListPanelActions::properties()
{
    ACTIVE_FUNC->properties();
}

void ListPanelActions::rightclickMenu()
{
    ACTIVE_PANEL->gui->rightclickMenu();
}

void ListPanelActions::slotPack()
{
    ACTIVE_FUNC->pack();
}

void ListPanelActions::slotUnpack()
{
    ACTIVE_FUNC->unpack();
}

void ListPanelActions::testArchive()
{
    ACTIVE_FUNC->testArchive();
}

void ListPanelActions::calcSpace()
{
    ACTIVE_FUNC->calcSpace();
}

void ListPanelActions::newSymlink()
{
    ACTIVE_PANEL->func->krlink(true);
}

// navigation

void ListPanelActions::historyBackward()
{
    ACTIVE_FUNC->historyBackward();
}

void ListPanelActions::historyForward()
{
    ACTIVE_FUNC->historyForward();
}

void ListPanelActions::dirUp()
{
    ACTIVE_FUNC->dirUp();
}

void ListPanelActions::home()
{
    ACTIVE_FUNC->openUrl(QDir::homePath());
}

void ListPanelActions::root()
{
    ACTIVE_FUNC->openUrl(KUrl("/"));
}

void ListPanelActions::refresh()
{
    ACTIVE_FUNC->refresh();
}

void ListPanelActions::cancelRefresh()
{
    ACTIVE_PANEL->gui->inlineRefreshCancel();
}

void ListPanelActions::FTPDisconnect()
{
    ACTIVE_FUNC->FTPDisconnect();
}

void ListPanelActions::newFTPconnection()
{
    ACTIVE_FUNC->newFTPconnection();
}

void ListPanelActions::slotJumpBack()
{
    ACTIVE_PANEL->gui->jumpBack();
}

void ListPanelActions::slotSetJumpBack()
{
    ACTIVE_PANEL->gui->setJumpBack(ACTIVE_PANEL->virtualPath());
}

void ListPanelActions::slotLocationBar()
{
    ACTIVE_PANEL->gui->editLocation();
}

void ListPanelActions::togglePopupPanel()
{
    ACTIVE_PANEL->gui->togglePanelPopup();
}

void ListPanelActions::slotSyncBrowse()
{
    ACTIVE_PANEL->gui->toggleSyncBrowse();
}

void ListPanelActions::syncPanels()
{
    OTHER_FUNC->openUrl(ACTIVE_PANEL->virtualPath());
}

void ListPanelActions::openBookmarks()
{
    ACTIVE_PANEL->gui->openBookmarks();
}

void ListPanelActions::openLeftBookmarks()
{
    LEFT_PANEL->gui->openBookmarks();
}

void ListPanelActions::openRightBookmarks()
{
    RIGHT_PANEL->gui->openBookmarks();
}

void ListPanelActions::openHistory()
{
    ACTIVE_PANEL->gui->openHistory();
}

void ListPanelActions::openLeftHistory()
{
    LEFT_PANEL->gui->openHistory();
}

void ListPanelActions::openRightHistory()
{
    RIGHT_PANEL->gui->openHistory();
}

void ListPanelActions::openMedia()
{
    ACTIVE_PANEL->gui->openMedia();
}

void ListPanelActions::openLeftMedia()
{
    LEFT_PANEL->gui->openMedia();
}

void ListPanelActions::openRightMedia()
{
    RIGHT_PANEL->gui->openMedia();
}
