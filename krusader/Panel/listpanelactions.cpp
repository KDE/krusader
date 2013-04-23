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
#include "../filemanagerwindow.h"
#include "../Dialogs/krdialogs.h"
#include "../KViewer/krviewer.h"

#include <QSignalMapper>
#include <QActionGroup>
#include <klocale.h>
#include <kactioncollection.h>


ListPanelActions::ListPanelActions(QObject *parent, FileManagerWindow *mainWindow) :
        ActionsBase(parent, mainWindow)
{
    // set view type
    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), SLOT(setView(int)));
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    QList<KrViewInstance*> views = KrViewFactory::registeredViews();
    for(int i = 0; i < views.count(); i++) {
        KrViewInstance *inst = views[i];
        KAction *action = new KAction(KIcon(inst->icon()), inst->description(), group);
        action->setShortcut(inst->shortcut());
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
        mapper->setMapping(action, inst->id());
        _mainWindow->actions()->addAction("view" + QString::number(i), action);
        setViewActions.insert(inst->id(), action);
    }

    // standard actions
    actHistoryBackward = stdAction(KStandardAction::Back, _func, SLOT(historyBackward()));
    actHistoryForward = stdAction(KStandardAction::Forward, _func, SLOT(historyForward()));
    //FIXME: second shortcut for up: see actDirUp
    //   KStandardAction::up( this, SLOT( dirUp() ), actionCollection )->setShortcut(Qt::Key_Backspace);
    /* Shortcut disabled because of the Terminal Emulator bug. */
    actDirUp = stdAction(KStandardAction::Up, _func, SLOT(dirUp()));
    actHome = stdAction(KStandardAction::Home, _func, SLOT(home()));
    stdAction(KStandardAction::Cut, _func, SLOT(cut()));
    actCopy = stdAction(KStandardAction::Copy, _func, SLOT(copyToClipboard()));
    actPaste = stdAction(KStandardAction::Paste, _func, SLOT(pasteFromClipboard()));

    // Fn keys
    actF2 = action(i18n("Start Terminal Here"), "utilities-terminal", Qt::Key_F2, _func, SLOT(terminal()) , "F2_Terminal");
    actF3 = action(i18n("View File"), 0, Qt::Key_F3, _func, SLOT(view()), "F3_View");
    actF4 = action(i18n("Edit File"), 0, Qt::Key_F4, _func, SLOT(edit()) , "F4_Edit");
    actF5 = action(i18n("Copy to other panel"), 0, Qt::Key_F5, _func, SLOT(copyFiles()) , "F5_Copy");
    actF6 = action(i18n("Move..."), 0, Qt::Key_F6, _func, SLOT(moveFiles()) , "F6_Move");
    actShiftF5 = action(i18n("Copy by queue..."), 0, Qt::SHIFT + Qt::Key_F5, _func, SLOT(copyFilesByQueue()) , "F5_Copy_Queue");
    actShiftF6 = action(i18n("Move by queue..."), 0, Qt::SHIFT + Qt::Key_F6, _func, SLOT(moveFilesByQueue()) , "F6_Move_Queue");
    actF7 = action(i18n("New Directory..."), "folder-new", Qt::Key_F7, _func, SLOT(mkdir()) , "F7_Mkdir");
    actF8 = action(i18n("Delete"), "edit-delete", Qt::Key_F8, _func, SLOT(deleteFiles()) , "F8_Delete");
    actF9 = action(i18n("Rename"), 0, Qt::Key_F9, _func, SLOT(rename()) , "F9_Rename");
    action(i18n("&New Text File..."), "document-new", Qt::SHIFT + Qt::Key_F4, _func, SLOT(editNew()), "edit_new_file");
    action(i18n("F3 View Dialog"), 0, Qt::SHIFT + Qt::Key_F3, _func, SLOT(viewDlg()), "F3_ViewDlg");

    // file operations
    action(i18n("Right-click Menu"), 0, Qt::Key_Menu, _gui, SLOT(rightclickMenu()), "rightclick menu");
    actProperties = action(i18n("&Properties..."), 0, Qt::ALT + Qt::Key_Return, _func, SLOT(properties()), "properties");
    actCompDirs = action(i18n("&Compare Directories"), "view_left_right", Qt::ALT + Qt::SHIFT + Qt::Key_C, _gui, SLOT(compareDirs()), "compare dirs");
    actCalculate = action(i18n("Calculate &Occupied Space"), "accessories-calculator", 0, _func, SLOT(calcSpace()), "calculate");
    actPack = action(i18n("Pac&k..."), "archive-insert", Qt::ALT + Qt::SHIFT + Qt::Key_P, _func, SLOT(pack()), "pack");
    actUnpack = action(i18n("&Unpack..."), "archive-extract", Qt::ALT + Qt::SHIFT + Qt::Key_U, _func, SLOT(unpack()), "unpack");
    actCreateChecksum = action(i18n("Create Checksum..."), "binary", 0, _func, SLOT(createChecksum()), "create checksum");
    actMatchChecksum = action(i18n("Verify Checksum..."), "binary", 0, _func, SLOT(matchChecksum()), "match checksum");
    action(i18n("New Symlink..."), 0, Qt::CTRL + Qt::ALT + Qt::Key_S, _func, SLOT(krlink()), "new symlink");
    actTest = action(i18n("T&est Archive"), "utilities-file-archiver", Qt::ALT + Qt::SHIFT + Qt::Key_E, _func, SLOT(testArchive()), "test archives");

    // navigation
    actRoot = action(i18n("Root"), "go-top", Qt::CTRL + Qt::Key_Backspace, _func, SLOT(root()), "root");
    actCdToOther = action(i18n("Go to Other Panel's Directory"), 0, Qt::CTRL + Qt::Key_Equal, _func, SLOT(cdToOtherPanel()), "cd to other panel");
    action(i18n("&Reload"), "view-refresh", Qt::CTRL + Qt::Key_R, _func, SLOT(refresh()), "std_redisplay");
    actCancelRefresh = action(i18n("Cancel Refresh of View"), "kr_cancel_refresh", 0, _gui, SLOT(inlineRefreshCancel()), "cancel refresh");
    actFTPNewConnect = action(i18n("New Net &Connection..."), "network-connect", Qt::CTRL + Qt::Key_N, _func, SLOT(newFTPconnection()), "ftp new connection");
    actFTPDisconnect = action(i18n("Disconnect &from Net"), "network-disconnect", Qt::SHIFT + Qt::CTRL + Qt::Key_F, _func, SLOT(FTPDisconnect()), "ftp disconnect");
    action(i18n("Sync Panels"), 0, Qt::ALT + Qt::SHIFT + Qt::Key_O, _func, SLOT(syncOtherPanel()), "sync panels");
    actJumpBack = action(i18n("Jump Back"), "kr_jumpback", Qt::CTRL + Qt::Key_J, _gui, SLOT(jumpBack()), "jump_back");
    actSetJumpBack = action(i18n("Set Jump Back Point"), "kr_setjumpback", Qt::CTRL + Qt::SHIFT + Qt::Key_J, _gui, SLOT(setJumpBack()), "set_jump_back");
    actSyncBrowse = action(i18n("S&ynchron Directory Changes"), "kr_syncbrowse_off", Qt::ALT + Qt::SHIFT + Qt::Key_Y, _gui, SLOT(toggleSyncBrowse()), "sync browse");
    actLocationBar = action(i18n("Go to Location Bar"), 0, Qt::CTRL + Qt::Key_L, _gui, SLOT(editLocation()), "location_bar");
    toggleAction(i18n("Toggle Popup Panel"), 0, Qt::ALT + Qt::Key_Down, _gui, SLOT(togglePanelPopup()), "toggle popup panel");
    action(i18n("Bookmarks"), 0, Qt::CTRL + Qt::Key_D, _gui, SLOT(openBookmarks()), "bookmarks");
    action(i18n("Left Bookmarks"), 0, 0, this, SLOT(openLeftBookmarks()), "left bookmarks");
    action(i18n("Right Bookmarks"), 0, 0, this, SLOT(openRightBookmarks()), "right bookmarks");
    action(i18n("History"), 0, Qt::CTRL + Qt::Key_H, _gui, SLOT(openHistory()), "history");
    action(i18n("Left History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Left, this, SLOT(openLeftHistory()), "left history");
    action(i18n("Right History"), 0, Qt::ALT + Qt::CTRL + Qt::Key_Right, this, SLOT(openRightHistory()), "right history");
    action(i18n("Media"), 0, Qt::CTRL + Qt::Key_M, _gui, SLOT(openMedia()), "media");
    action(i18n("Left Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Left, this, SLOT(openLeftMedia()), "left media");
    action(i18n("Right Media"), 0, Qt::CTRL + Qt::SHIFT + Qt::Key_Right, this, SLOT(openRightMedia()), "right media");

    // and at last we can set the tool-tips
    actRoot->setToolTip(i18n("ROOT (/)"));

    actF2->setToolTip("<qt>" + i18n("<p>Open terminal in current directory.</p>"
                                 "<p>The terminal can be defined in Konfigurator, "
                                 "default is <b>konsole</b>.</p>") + "</qt>");
    actF3->setToolTip(i18n("Open file in viewer."));
    actF4->setToolTip("<qt>" + i18n("<p>Edit file.</p>"
                                 "<p>The editor can be defined in Konfigurator, "
                                 "default is <b>internal editor</b>.</p>") + "</qt>");
    actF5->setToolTip(i18n("Copy file from one panel to the other."));
    actF6->setToolTip(i18n("Move file from one panel to the other."));
    actF7->setToolTip(i18n("Create directory in current panel."));
    actF8->setToolTip(i18n("Delete file, directory, etc."));
    actF9->setToolTip(i18n("Rename file, directory, etc."));
}

void ListPanelActions::activePanelChanged()
{
    _gui.reconnect(activePanel()->gui);
    _func.reconnect(activePanel()->func);
}

void ListPanelActions::guiUpdated()
{
    QList<QAction*> actions;
    foreach(QAction *action, setViewActions.values())
        actions << action;
    static_cast<FileManagerWindow*>(_mainWindow)->plugActionList("view_actionlist", actions);
}

inline KrPanel *ListPanelActions::activePanel()
{
    return static_cast<FileManagerWindow*>(_mainWindow)->activePanel();
}

inline KrPanel *ListPanelActions::leftPanel()
{
    return static_cast<FileManagerWindow*>(_mainWindow)->leftPanel();
}

inline KrPanel *ListPanelActions::rightPanel()
{
    return static_cast<FileManagerWindow*>(_mainWindow)->rightPanel();
}

inline ListPanelFunc *ListPanelActions::activeFunc()
{
    return activePanel()->func;
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
