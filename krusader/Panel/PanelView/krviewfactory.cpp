/*****************************************************************************
 * Copyright (C) 2000-2007 Shie Erlich <krusader@users.sourceforge.net>      *
 * Copyright (C) 2000-2007 Rafi Yanai <krusader@users.sourceforge.net>       *
 * Copyright (C) 2000-2007 Csaba Karai <krusader@users.sourceforge.net>      *
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

#include "krviewfactory.h"

#include "krinterdetailedview.h"
#include "krinterbriefview.h"

#include <stdio.h>

#include <KI18n/KLocalizedString>

KrViewInstance::KrViewInstance(int id, const QString &name, const QString &desc,
                               const QString &iconName, const QKeySequence &shortcut)
    : m_id(id), m_name(name), m_description(desc), m_iconName(iconName), m_shortcut(shortcut)
{
}

template <typename T>
class KrViewInstanceImpl : public KrViewInstance
{
  public:
    KrViewInstanceImpl(int id, const QString &name, const QString &desc, const QString &icon,
                       const QKeySequence &shortcut)
        : KrViewInstance(id, name, desc, icon, shortcut) {}

    virtual KrView *create(QWidget *w, KConfig *cfg) Q_DECL_OVERRIDE {
        return new T(w, *this, cfg);
    }
};

KrViewFactory::KrViewFactory() : m_defaultViewId(-1) {}

// static initialization, on first use idiom
KrViewFactory &KrViewFactory::self()
{
    static KrViewFactory *factory = 0;
    if (!factory) {
        factory = new KrViewFactory();
        factory->init();
    }
    return *factory;
}

void KrViewFactory::init()
{
    registerView(new KrViewInstanceImpl<KrInterDetailedView> (0, "KrInterDetailedView",
        i18n("&Detailed View"), "view-list-details", Qt::ALT + Qt::SHIFT + Qt::Key_D));

    registerView(new KrViewInstanceImpl<KrInterBriefView> (1, "KrInterBriefView",
        i18n("&Brief View"), "view-list-icons", Qt::ALT + Qt::SHIFT + Qt::Key_B));
}

KrView *KrViewFactory::createView(int id, QWidget *widget, KConfig *cfg)
{
    return self().viewInstance(id)->create(widget, cfg);
}

void KrViewFactory::registerView(KrViewInstance *inst)
{
    int position = 0;

    while (position < m_registeredViews.count()) {
        if (m_registeredViews[position]->id() > inst->id())
            break;
        position++;
    }

    m_registeredViews.insert(m_registeredViews.begin() + position, inst);
    if (m_defaultViewId == -1 || inst->id() < m_defaultViewId)
        m_defaultViewId = inst->id();
}

KrViewInstance *KrViewFactory::viewInstance(int id)
{
    foreach (KrViewInstance *inst, m_registeredViews) {
        if (inst->id() == id)
            return inst;
    }

    foreach (KrViewInstance *inst_dflt, m_registeredViews) {
        if (inst_dflt->id() == m_defaultViewId)
            return inst_dflt;
    }

    fprintf(stderr, "Internal Error: no views registered!\n");
    exit(-1);
}
