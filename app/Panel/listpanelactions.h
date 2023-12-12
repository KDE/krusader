/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    QAction *actRenameF2, *actViewFileF3, *actEditFileF4, *actCopyF5, *actMoveF6, *actNewFolderF7, *actDeleteF8, *actTerminalF9;
    QAction *actNewFileShiftF4, *actCopyDelayedF5, *actMoveDelayedShiftF6;
    QAction *actProperties, *actPack, *actUnpack, *actTest, *actCompDirs, *actCalculate, *actSync;
    QAction *actFTPConnect, *actFTPNewConnect, *actFTPDisconnect;
    QAction *actLocationBar, *actJumpBack, *actSetJumpBack;
    QAction *actCreateChecksum, *actMatchChecksum;
    QAction *actCopy, *actCut, *actPaste;
    QAction *actHistoryBackward, *actHistoryForward, *actDirUp, *actRoot, *actHome, *actCdToOther;
    QAction *actSyncBrowse, *actCancelRefresh;

    QHash<int /*id*/, QAction *> setViewActions;

protected:
    KrPanel *activePanel();
    KrPanel *leftPanel();
    KrPanel *rightPanel();

    ActionGroup _gui, _func;
};

#endif // LISTPANELACTIONS_H
