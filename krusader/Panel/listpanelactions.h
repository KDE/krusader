/***************************************************************************
                                listpanelactions.h
                           -------------------
    begin                : Thu May 4 2000
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LISTPANELACTIONS_H__
#define __LISTPANELACTIONS_H__

#include "../actionsbase.h"

#include <kaction.h>


class FileManagerWindow;
class KrPanel;
class ListPanel;
class ListPanelFunc;

class ListPanelActions : public ActionsBase
{
    Q_OBJECT
public:
    ListPanelActions(QObject *parent, FileManagerWindow *mainWindow);

public slots:
    // set view type
    void setView(int id);

    // F2
    void terminal();
    // F3
    void view();
    // Shift F3
    void viewDlg();
    // F4
    void edit();
    // Shift F4
    void editDlg();
    // F5
    void copyFiles();
    // F6
    void moveFiles();
    // SHIFT + F5
    void copyFilesByQueue();
    // SHIFT + F6
    void moveFilesByQueue();
    // F7
    void mkdir();
    // F8
    void deleteFiles(bool reallyDelete = false);
    // F9
    void rename();

    //filter
    void allFilter();
    //void execFilter();
    void customFilter();

    // selection
    void markAll();
    void unmarkAll();
    void markGroup();
    void markGroup(const QString &, bool select);
    void unmarkGroup();
    void invert();

    // file operations
    void cut();
    void copy();
    void paste();
    void rightclickMenu();
    void properties();
    void compareDirs();
    void slotPack();
    void slotUnpack();
    void createChecksum();
    void matchChecksum();
    void newSymlink();
    void testArchive();
    void calcSpace();

    // navigation
    void historyBackward();
    void historyForward();
    void dirUp();
    void home();
    void root();
    void refresh();
    void FTPDisconnect();
    void newFTPconnection();
    void syncPanels();
    void slotSetJumpBack();
    void openLeftBookmarks();
    void openRightBookmarks();
    void openLeftHistory();
    void openRightHistory();
    void openLeftMedia();
    void openRightMedia();

    void activePanelChanged(ListPanel *panel);

public:
    static ListPanelActions *self;

    static KAction *actF2, *actF3, *actF4, *actF5, *actF6, *actF7, *actF8, *actF9;
    static KAction *actShiftF5, *actShiftF6;
    static KAction *actProperties, *actPack, *actUnpack, *actTest,  *actCompDirs, *actCalculate, *actSync;
    static KAction *actSelect, *actUnselect, *actSelectAll, *actUnselectAll, *actInvert;
    static KAction *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect;
    static KAction *actExecFilter, *actAllFilter, *actCustomFilter;
    static KAction *actLocationBar, *actJumpBack, *actSetJumpBack;
    static KAction *actCreateChecksum, *actMatchChecksum;
    static KAction *actCopy, *actPaste;
    static KAction *actHistoryBackward, *actHistoryForward, *actDirUp, *actRoot;
    static KAction *actSyncBrowse, *actCancelRefresh;

    QHash<int/*id*/, KAction*> setViewActions;

protected:
    KrPanel *activePanel();
    KrPanel *leftPanel();
    KrPanel *rightPanel();
    ListPanelFunc *activeFunc();

    ActionGroup _guiActions, _funcActions;
};

#endif // __LISTPANELACTIONS_H__
