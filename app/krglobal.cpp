/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krglobal.h"

#include "krusader.h"
#include "krusaderview.h"

#include <KActionCollection>

KConfig *KrGlobal::config = nullptr;
KMountMan *KrGlobal::mountMan = nullptr;
KrArcHandler *KrGlobal::arcMan = nullptr;
KrBookmarkHandler *KrGlobal::bookman = nullptr;
KrSlots *KrGlobal::slot = nullptr;
KrusaderView *KrGlobal::mainView = nullptr;
QWidget *KrGlobal::mainWindow = nullptr;
UserAction *KrGlobal::userAction = nullptr;
JobMan *KrGlobal::jobMan = nullptr;
// ListPanel *KrGlobal::activePanel = 0;
QKeySequence KrGlobal::copyShortcut;

const int KrGlobal::sConfigVersion;
int KrGlobal::sCurrentConfigVersion;

bool KrGlobal::isWaylandPlatform = false;
bool KrGlobal::isX11Platform = false;

KrPanel *KrGlobal::activePanel()
{
    return mainView->activePanel();
}
