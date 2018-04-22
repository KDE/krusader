/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KRGLOBAL_H
#define KRGLOBAL_H

// QtGui
#include <QKeySequence>

#include <KConfigCore/KConfigGroup>

class KConfig;
class KMountMan;
class KrBookmarkHandler;
class KRslots;
class KrusaderView;
class UserAction;
class JobMan;
class QWidget;
class KrPanel;

// global references to frequently used objects

class KrGlobal
{
public:
    static KConfig *config;    // allow everyone to access the config
    static KMountMan *mountMan;  // krusader's Mount Manager
    static KrBookmarkHandler *bookman;
    static KRslots *slot;
    static KrusaderView *mainView;  // The GUI
    static QWidget *mainWindow;
    static UserAction *userAction;
    static JobMan *jobMan;
//     static ListPanel  *activePanel;
    static KrPanel *activePanel();

    //HACK - used by [ListerTextArea|KrSearchDialog|LocateDlg]:keyPressEvent()
    static QKeySequence copyShortcut;

//     static void enableAction(const char *name, bool enable);
//     static QAction *getAction(const char *name);

    /** Version of saved configuration. Use this to detect configuration updates. */
    static const int sConfigVersion = 1;
    static int sCurrentConfigVersion;
};

#define krConfig     KrGlobal::config
#define krMtMan      (*(KrGlobal::mountMan))
#define krBookMan    KrGlobal::bookman
#define SLOTS        KrGlobal::slot
#define MAIN_VIEW    KrGlobal::mainView
#define krMainWindow KrGlobal::mainWindow
#define krUserAction KrGlobal::userAction
#define krJobMan     KrGlobal::jobMan

#define ACTIVE_PANEL (KrGlobal::activePanel())

#define ACTIVE_MNG   (MAIN_VIEW->activeManager())
#define ACTIVE_FUNC  (ACTIVE_PANEL->func)
#define OTHER_MNG  (MAIN_VIEW->inactiveManager())
#define OTHER_PANEL (ACTIVE_PANEL->otherPanel())
#define OTHER_FUNC (OTHER_PANEL->func)
#define LEFT_PANEL (MAIN_VIEW->leftPanel())
#define LEFT_FUNC  (LEFT_PANEL->func)
#define LEFT_MNG  (MAIN_VIEW->leftManager())
#define RIGHT_PANEL  (MAIN_VIEW->rightPanel())
#define RIGHT_FUNC (RIGHT_PANEL->func)
#define RIGHT_MNG  (MAIN_VIEW->rightManager())

// #define krEnableAction(name, enable) (KrGlobal::enableAction(#name, (enable)))
// #define krGetAction(name) (KrGlobal::getAction(#name))

#endif
