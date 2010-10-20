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

    // navigation
    void openLeftBookmarks();
    void openRightBookmarks();
    void openLeftHistory();
    void openRightHistory();
    void openLeftMedia();
    void openRightMedia();

    void activePanelChanged(ListPanel *panel);
    void guiUpdated();

public:
    KAction *actF2, *actF3, *actF4, *actF5, *actF6, *actF7, *actF8, *actF9;
    KAction *actShiftF5, *actShiftF6;
    KAction *actProperties, *actPack, *actUnpack, *actTest,  *actCompDirs, *actCalculate, *actSync;
    KAction *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect;
    KAction *actLocationBar, *actJumpBack, *actSetJumpBack;
    KAction *actCreateChecksum, *actMatchChecksum;
    KAction *actCopy, *actPaste;
    KAction *actHistoryBackward, *actHistoryForward, *actDirUp, *actRoot;
    KAction *actSyncBrowse, *actCancelRefresh;

    QHash<int/*id*/, KAction*> setViewActions;

protected:
    KrPanel *activePanel();
    KrPanel *leftPanel();
    KrPanel *rightPanel();
    ListPanelFunc *activeFunc();

    ActionGroup _gui, _func;
};

#endif // __LISTPANELACTIONS_H__
