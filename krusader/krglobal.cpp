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

#include "krglobal.h"

#include "krusader.h"
#include "krusaderview.h"

// QtCore
#include <QtGlobal>

#include <KXmlGui/KActionCollection>

KConfig *KrGlobal::config = 0;
KMountMan *KrGlobal::mountMan = 0;
KrBookmarkHandler *KrGlobal::bookman = 0;
KRslots* KrGlobal::slot = 0;
KrusaderView *KrGlobal::mainView = 0;
QWidget *KrGlobal::mainWindow = 0;
UserAction *KrGlobal::userAction = 0;
JobMan *KrGlobal::jobMan = 0;
// ListPanel *KrGlobal::activePanel = 0;
QKeySequence KrGlobal::copyShortcut;

const int KrGlobal::sConfigVersion;
int KrGlobal::sCurrentConfigVersion;

KrPanel *KrGlobal::activePanel()
{
    return mainView->activePanel();
}

// void KrGlobal::enableAction(const char *name, bool enable)
// {
//     getAction(name)->setEnabled(enable);
// }
// 
// QAction* KrGlobal::getAction(const char *name)
// {
//     QAction *act = krApp->actionCollection()->action(name);
//     if(!act)
//         qFatal("no such action: %s", name);
//     return act;
// }
