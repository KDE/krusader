/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRGLOBAL_H
#define KRGLOBAL_H

// QtGui
#include <QKeySequence>

#include <KConfigGroup>

class KConfig;
class KMountMan;
class KrArcHandler;
class KrBookmarkHandler;
class KrSlots;
class KrusaderView;
class UserAction;
class JobMan;
class QWidget;
class KrPanel;

// global references to frequently used objects

class KrGlobal
{
public:
    static KConfig *config; // allow everyone to access the config
    static KMountMan *mountMan; // krusader's Mount Manager
    static KrArcHandler *arcMan; //! Manages archives in several parts of the code
    static KrBookmarkHandler *bookman;
    static KrSlots *slot;
    static KrusaderView *mainView; // The GUI
    static QWidget *mainWindow;
    static UserAction *userAction;
    static JobMan *jobMan;
    static KrPanel *activePanel();

    // HACK - used by [ListerTextArea|KrSearchDialog|LocateDlg]:keyPressEvent()
    static QKeySequence copyShortcut;

    /** Version of saved configuration. Use this to detect configuration updates. */
    static const int sConfigVersion = 1;
    static int sCurrentConfigVersion;

    static bool isX11Platform; // running on X11
    static bool isWaylandPlatform; // running on Wayland
};

#define krConfig KrGlobal::config
#define krMtMan (*(KrGlobal::mountMan))
#define krArcMan (*(KrGlobal::arcMan))
#define krBookMan KrGlobal::bookman
#define SLOTS KrGlobal::slot
#define MAIN_VIEW KrGlobal::mainView
#define krMainWindow KrGlobal::mainWindow
#define krUserAction KrGlobal::userAction
#define krJobMan KrGlobal::jobMan

#define ACTIVE_PANEL (KrGlobal::activePanel())

#define ACTIVE_MNG (MAIN_VIEW->activeManager())
#define ACTIVE_FUNC (ACTIVE_PANEL->func)
#define OTHER_MNG (MAIN_VIEW->inactiveManager())
#define OTHER_PANEL (ACTIVE_PANEL->otherPanel())
#define OTHER_FUNC (OTHER_PANEL->func)
#define LEFT_PANEL (MAIN_VIEW->leftPanel())
#define LEFT_FUNC (LEFT_PANEL->func)
#define LEFT_MNG (MAIN_VIEW->leftManager())
#define RIGHT_PANEL (MAIN_VIEW->rightPanel())
#define RIGHT_FUNC (RIGHT_PANEL->func)
#define RIGHT_MNG (MAIN_VIEW->rightManager())

// #define krEnableAction(name, enable) (KrGlobal::enableAction(#name, (enable)))
// #define krGetAction(name) (KrGlobal::getAction(#name))

#endif
