/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "listpanelactions.h"

#include "../Dialogs/krdialogs.h"
#include "../KViewer/krviewer.h"
#include "../icon.h"
#include "../krmainwindow.h"
#include "PanelView/krviewfactory.h"
#include "listpanel.h"
#include "panelfunc.h"

// QtWidgets
#include <QActionGroup>

#include <KActionCollection>
#include <KLocalizedString>

ListPanelActions::ListPanelActions(QObject *parent, KrMainWindow *mainWindow)
    : ActionsBase(parent, mainWindow)
{
    // set view type
    auto *group = new QActionGroup(this);
    group->setExclusive(true);
    QList<KrViewInstance *> views = KrViewFactory::registeredViews();
    for (int i = 0; i < views.count(); i++) {
        KrViewInstance *inst = views[i];
        QAction *action = new QAction(Icon(inst->iconName()), inst->description(), group);
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=] {
            setView(inst->id());
        });
        _mainWindow->actions()->addAction("view" + QString::number(i), action);
        _mainWindow->actions()->setDefaultShortcut(action, inst->shortcut());
        setViewActions.insert(inst->id(), action);
    }

    // standard actions
    actHistoryBackward = stdAction(KStandardAction::Back, _func, SLOT(historyBackward()));
    actHistoryForward = stdAction(KStandardAction::Forward, _func, SLOT(historyForward()));
    // FIXME: second shortcut for up: see actDirUp
    //    KStandardAction::up( this, SLOT(dirUp()), actionCollection )->setShortcut(Qt::Key_Backspace);
    /* Shortcut disabled because of the Terminal Emulator bug. */
    actDirUp = stdAction(KStandardAction::Up, _func, SLOT(dirUp()));
    actHome = stdAction(KStandardAction::Home, _func, SLOT(home()));

    actCut = stdAction(KStandardAction::Cut, _func, SLOT(cut()));
    actCut->setText(i18n("Cut to Clipboard"));
    // removing alternative shift+del shortcut, it is used for the alternative delete files action
    QList<QKeySequence> cutShortcuts = actCut->shortcuts();
    cutShortcuts.removeAll(QKeySequence(Qt::SHIFT | Qt::Key_Delete));
    _mainWindow->actions()->setDefaultShortcuts(actCut, cutShortcuts);
    actCopy = stdAction(KStandardAction::Copy, _func, SLOT(copyToClipboard()));
    actCopy->setText(i18n("Copy to Clipboard"));
    actPaste = stdAction(KStandardAction::Paste, _func, SLOT(pasteFromClipboard()));
    actPaste->setText(i18n("Paste from Clipboard"));

    // Fn keys
    actRenameF2 = action(i18n("Rename"), "edit-rename", Qt::Key_F2, _func, SLOT(rename()), "F2_Rename");
    actViewFileF3 = action(i18n("View File"), nullptr, Qt::Key_F3, _func, SLOT(view()), "F3_View");
    actEditFileF4 = action(i18n("Edit File"), nullptr, Qt::Key_F4, _func, SLOT(editFile()), "F4_Edit");
    actNewFileShiftF4 = action(i18n("&New Text File..."), "document-new", Qt::SHIFT | Qt::Key_F4, _func, SLOT(askEditFile()), "edit_new_file");
    actCopyF5 = action(i18n("Copy to other panel"), nullptr, Qt::Key_F5, _func, SLOT(copyFiles()), "F5_Copy");
    actMoveF6 = action(i18n("Move to other panel"), nullptr, Qt::Key_F6, _func, SLOT(moveFiles()), "F6_Move");
    actCopyDelayedF5 = action(i18n("Copy delayed..."), nullptr, Qt::SHIFT | Qt::Key_F5, _func, SLOT(copyFilesDelayed()), "F5_Copy_Queue");
    actMoveDelayedShiftF6 = action(i18n("Move delayed..."), nullptr, Qt::SHIFT | Qt::Key_F6, _func, SLOT(moveFilesDelayed()), "F6_Move_Queue");
    actNewFolderF7 = action(i18n("New Folder..."), "folder-new", Qt::Key_F7, _func, SLOT(mkdir()), "F7_Mkdir");
    actDeleteF8 = action(i18n("Delete"), "edit-delete", Qt::Key_F8, _func, SLOT(defaultDeleteFiles()), "F8_Delete");
    actTerminalF9 = action(i18n("Start Terminal Here"), "utilities-terminal", Qt::Key_F9, _func, SLOT(terminal()), "F9_Terminal");
    action(i18n("F3 View Dialog"), nullptr, Qt::SHIFT | Qt::Key_F3, _func, SLOT(viewDlg()), "F3_ViewDlg");

    // file operations
    action(i18n("Right-click Menu"), nullptr, Qt::Key_Menu, _gui, SLOT(rightclickMenu()), "rightclick menu");
    actProperties = action(i18n("&Properties..."), "document-properties", Qt::ALT | Qt::Key_Return, _func, SLOT(properties()), "properties");
    actCompDirs = action(i18n("&Compare Folders"), "kr_comparedirs", Qt::ALT | Qt::SHIFT | Qt::Key_C, _gui, SLOT(compareDirs()), "compare dirs");
    actCalculate = action(i18n("Calculate &Occupied Space"), "accessories-calculator", 0, _func, SLOT(calcSpace()), "calculate");
    actPack = action(i18n("Pac&k..."), "archive-insert", Qt::ALT | Qt::SHIFT | Qt::Key_P, _func, SLOT(pack()), "pack");
    actUnpack = action(i18n("&Unpack..."), "archive-extract", Qt::ALT | Qt::SHIFT | Qt::Key_U, _func, SLOT(unpack()), "unpack");
    actCreateChecksum = action(i18n("Create Checksum..."), "document-edit-sign", 0, _func, SLOT(createChecksum()), "create checksum");
    actMatchChecksum = action(i18n("Verify Checksum..."), "document-edit-decrypt-verify", 0, _func, SLOT(matchChecksum()), "match checksum");
    action(i18n("New Symlink..."), nullptr, Qt::CTRL | Qt::ALT | Qt::Key_S, _func, SLOT(krlink()), "new symlink");
    actTest = action(i18n("T&est Archive"), "archive-extract", Qt::ALT | Qt::SHIFT | Qt::Key_E, _func, SLOT(testArchive()), "test archives");
    action(i18n("Alternative Delete"), "edit-delete", Qt::Key_Delete | Qt::CTRL, _func, SLOT(alternativeDeleteFiles()), "alternative delete");

    // navigation
    actRoot = action(i18n("Root"), "folder-red", Qt::CTRL | Qt::Key_Backspace, _func, SLOT(root()), "root");
    actCdToOther = action(i18n("Go to Other Panel's Folder"), nullptr, Qt::CTRL | Qt::Key_Equal, _func, SLOT(cdToOtherPanel()), "cd to other panel");
    action(i18n("&Reload"), "view-refresh", Qt::CTRL | Qt::Key_R, _func, SLOT(refresh()), "std_redisplay");
    actCancelRefresh = action(i18n("Cancel Refresh of View"), "dialog-cancel", 0, _gui, SLOT(cancelProgress()), "cancel refresh");
    actFTPNewConnect = action(i18n("New Net &Connection..."), "network-connect", Qt::CTRL | Qt::Key_N, _func, SLOT(newFTPconnection()), "ftp new connection");
    actFTPDisconnect =
        action(i18n("Disconnect &from Net"), "network-disconnect", Qt::SHIFT | Qt::CTRL | Qt::Key_D, _func, SLOT(FTPDisconnect()), "ftp disconnect");
    action(i18n("Sync Panels"), nullptr, Qt::ALT | Qt::SHIFT | Qt::Key_O, _func, SLOT(syncOtherPanel()), "sync panels");
    actJumpBack = action(i18n("Jump Back"), "go-jump", Qt::CTRL | Qt::Key_J, _gui, SLOT(jumpBack()), "jump_back");
    actSetJumpBack = action(i18n("Set Jump Back Point"), "go-jump-definition", Qt::CTRL | Qt::SHIFT | Qt::Key_J, _gui, SLOT(setJumpBack()), "set_jump_back");
    actSyncBrowse =
        action(i18n("S&ynchron Folder Changes"), "kr_syncbrowse_off", Qt::ALT | Qt::SHIFT | Qt::Key_Y, _gui, SLOT(toggleSyncBrowse()), "sync browse");
    actLocationBar = action(i18n("Go to Location Bar"), nullptr, Qt::CTRL | Qt::Key_L, _gui, SLOT(editLocation()), "location_bar");
    toggleAction(i18n("Toggle Sidebar"), nullptr, Qt::ALT | Qt::Key_Down, _gui, SLOT(toggleSidebar()), "toggle sidebar");
    action(i18n("Bookmarks"), nullptr, Qt::CTRL | Qt::Key_D, _gui, SLOT(openBookmarks()), "bookmarks");
    action(i18n("Left Bookmarks"), nullptr, 0, this, SLOT(openLeftBookmarks()), "left bookmarks");
    action(i18n("Right Bookmarks"), nullptr, 0, this, SLOT(openRightBookmarks()), "right bookmarks");
    action(i18n("History"), nullptr, Qt::CTRL | Qt::Key_H, _gui, SLOT(openHistory()), "history");
    action(i18n("Left History"), nullptr, Qt::ALT | Qt::CTRL | Qt::Key_Left, this, SLOT(openLeftHistory()), "left history");
    action(i18n("Right History"), nullptr, Qt::ALT | Qt::CTRL | Qt::Key_Right, this, SLOT(openRightHistory()), "right history");
    action(i18n("Media"), nullptr, Qt::CTRL | Qt::Key_M, _gui, SLOT(openMedia()), "media");
    action(i18n("Left Media"), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_Left, this, SLOT(openLeftMedia()), "left media");
    action(i18n("Right Media"), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_Right, this, SLOT(openRightMedia()), "right media");

    // quick search bar
    action(i18n("Find in folder..."), nullptr, Qt::CTRL | Qt::Key_F, _gui, SLOT(showSearchBar()), "search bar");
    action(i18n("Select in folder..."), nullptr, Qt::CTRL | Qt::SHIFT | Qt::Key_S, _gui, SLOT(showSearchBarSelection()), "search bar selection");
    action(i18n("Filter in folder..."), nullptr, Qt::CTRL | Qt::Key_I, _gui, SLOT(showSearchBarFilter()), "search bar filter");

    // and at last we can set the tool-tips
    actRoot->setToolTip(i18n("ROOT (/)"));

    actRenameF2->setToolTip(i18n("Rename file, folder, etc."));
    actViewFileF3->setToolTip(i18n("Open file in viewer."));
    actEditFileF4->setToolTip(
        i18n("<qt><p>Edit file.</p>"
             "<p>The editor can be defined in Konfigurator, "
             "default is <b>internal editor</b>.</p></qt>"));
    actCopyF5->setToolTip(i18n("Copy file from one panel to the other."));
    actMoveF6->setToolTip(i18n("Move file from one panel to the other."));
    actNewFolderF7->setToolTip(i18n("Create folder in current panel."));
    actDeleteF8->setToolTip(i18n("Delete file, folder, etc."));
    actTerminalF9->setToolTip(
        i18n("<qt><p>Open terminal in current folder.</p>"
             "<p>The terminal can be defined in Konfigurator, "
             "default is <b>konsole</b>.</p></qt>"));
}

void ListPanelActions::activePanelChanged()
{
    _gui.reconnect(activePanel()->gui);
    _func.reconnect(activePanel()->func);
}

void ListPanelActions::guiUpdated()
{
    QList<QAction *> actions;
    for (QAction *action : std::as_const(setViewActions))
        actions << action;
    static_cast<KrMainWindow *>(_mainWindow)->plugActionList("view_actionlist", actions);
}

inline KrPanel *ListPanelActions::activePanel()
{
    return static_cast<KrMainWindow *>(_mainWindow)->activePanel();
}

inline KrPanel *ListPanelActions::leftPanel()
{
    return static_cast<KrMainWindow *>(_mainWindow)->leftPanel();
}

inline KrPanel *ListPanelActions::rightPanel()
{
    return static_cast<KrMainWindow *>(_mainWindow)->rightPanel();
}

// set view type

void ListPanelActions::setView(int id)
{
    activePanel()->gui->changeType(id);
}

// navigation

void ListPanelActions::openLeftBookmarks()
{
    leftPanel()->gui->openBookmarks();
}

void ListPanelActions::openRightBookmarks()
{
    rightPanel()->gui->openBookmarks();
}

void ListPanelActions::openLeftHistory()
{
    leftPanel()->gui->openHistory();
}

void ListPanelActions::openRightHistory()
{
    rightPanel()->gui->openHistory();
}

void ListPanelActions::openLeftMedia()
{
    leftPanel()->gui->openMedia();
}

void ListPanelActions::openRightMedia()
{
    rightPanel()->gui->openMedia();
}
