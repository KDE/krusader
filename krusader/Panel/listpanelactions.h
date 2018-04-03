/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef LISTPANELACTIONS_H
#define LISTPANELACTIONS_H

#include "../actionsbase.h"

// QtWidgets
#include <QAction>

class KrMainWindow;
class KrPanel;
class ListPanelFunc;

class ListPanelActions : public ActionsBase
{
    Q_OBJECT
public:
    ListPanelActions(QObject *parent, KrMainWindow *mainWindow);

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

    void activePanelChanged();
    void guiUpdated();

public:
    QAction *actRenameF2, *actViewFileF3, *actEditFileF4, *actCopyF5, *actMoveF6, *actNewFolderF7,
        *actDeleteF8, *actTerminalF9;
    QAction *actCopyDelayedF5, *actMoveDelayedShiftF6;
    QAction *actProperties, *actPack, *actUnpack, *actTest,  *actCompDirs, *actCalculate, *actSync;
    QAction *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect;
    QAction *actLocationBar, *actSearchBar, *actJumpBack, *actSetJumpBack;
    QAction *actCreateChecksum, *actMatchChecksum;
    QAction *actCopy, *actCut, *actPaste;
    QAction *actHistoryBackward, *actHistoryForward, *actDirUp, *actRoot, *actHome, *actCdToOther;
    QAction *actSyncBrowse, *actCancelRefresh;

    QHash<int/*id*/, QAction *> setViewActions;

protected:
    KrPanel *activePanel();
    KrPanel *leftPanel();
    KrPanel *rightPanel();

    ActionGroup _gui, _func;
};

#endif // LISTPANELACTIONS_H
