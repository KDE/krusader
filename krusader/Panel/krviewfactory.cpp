/***************************************************************************
                  krviewfactory.cpp
                 -------------------
copyright            : (C) 2000-2007 by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "krviewfactory.h"

#include "krinterdetailedview.h"
#include "krinterbriefview.h"

extern KrViewInstance interDetailedView;    // hold reference for linking
extern KrViewInstance interBriefView;    // hold reference for linking

KrViewInstance::KrViewInstance(int id, QString desc, QKeySequence shortcut, KrViewFactoryFunction fun, KrViewItemHeightChange fun2) :
        m_id(id), m_description(desc), m_shortcut(shortcut), m_factoryfun(fun), m_ihchangefun(fun2)
{
}

KrView * KrViewFactory::createView(int id, QWidget * widget, bool & left, KConfig *cfg)
{
    return (*(viewInstance(id)->factoryFunction()))(widget, left, cfg);
}

void KrViewFactory::itemHeightChanged(int id)
{
    (*(viewInstance(id)->itemHChangeFunction()))();
}

// static initialization, on first use idiom
KrViewFactory & KrViewFactory::self()
{
    static KrViewFactory * factory = new KrViewFactory();
    return *factory;
}

void KrViewFactory::registerView(KrViewInstance * inst)
{
    int position = 0;

    while (position < self().m_registeredViews.count()) {
        if (self().m_registeredViews[ position ]->id() > inst->id())
            break;
        position++;
    }

    self().m_registeredViews.insert(self().m_registeredViews.begin() + position, inst);
    if (self().m_defaultViewId == -1 || inst->id() < self().m_defaultViewId)
        self().m_defaultViewId = inst->id();
}

KrViewInstance * KrViewFactory::viewInstance(int id)
{
    foreach(KrViewInstance * inst, self().m_registeredViews)
    if (inst->id() == id)
        return inst;

    foreach(KrViewInstance * inst_dflt, self().m_registeredViews)
    if (inst_dflt->id() == self().m_defaultViewId)
        return inst_dflt;

    fprintf(stderr, "Internal Error: no views registered!\n");
    exit(-1);
}

void KrViewFactory::init()
{
    KrViewFactory::registerView(&interDetailedView);
    KrViewFactory::registerView(&interBriefView);
}
