/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#include "krglobal.h"

#include "krusader.h"
#include "krusaderview.h"

#include <KXmlGui/KActionCollection>

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

KrPanel *KrGlobal::activePanel()
{
    return mainView->activePanel();
}
