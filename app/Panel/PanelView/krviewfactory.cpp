/*
    SPDX-FileCopyrightText: 2000-2007 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2007 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2007 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewfactory.h"

#include "krinterbriefview.h"
#include "krinterdetailedview.h"

#include <stdio.h>

#include <KLocalizedString>

KrViewInstance::KrViewInstance(int id, const QString &name, const QString &desc, const QString &iconName, const QKeySequence &shortcut)
    : m_id(id)
    , m_name(name)
    , m_description(desc)
    , m_iconName(iconName)
    , m_shortcut(shortcut)
{
}

template<typename T>
class KrViewInstanceImpl : public KrViewInstance
{
public:
    KrViewInstanceImpl(int id, const QString &name, const QString &desc, const QString &icon, const QKeySequence &shortcut)
        : KrViewInstance(id, name, desc, icon, shortcut)
    {
    }

    KrView *create(QWidget *w, KConfig *cfg) override
    {
        return new T(w, *this, cfg);
    }
};

KrViewFactory::KrViewFactory()
    : m_defaultViewId(-1)
{
}

// static initialization, on first use idiom
KrViewFactory &KrViewFactory::self()
{
    static KrViewFactory *factory = nullptr;
    if (!factory) {
        factory = new KrViewFactory();
        factory->init();
    }
    return *factory;
}

void KrViewFactory::init()
{
    registerView(
        new KrViewInstanceImpl<KrInterDetailedView>(0, "KrInterDetailedView", i18n("&Detailed View"), "view-list-details", Qt::ALT | Qt::SHIFT | Qt::Key_D));

    registerView(new KrViewInstanceImpl<KrInterBriefView>(1, "KrInterBriefView", i18n("&Brief View"), "view-list-icons", Qt::ALT | Qt::SHIFT | Qt::Key_B));
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
    for (KrViewInstance *inst : std::as_const(m_registeredViews)) {
        if (inst->id() == id)
            return inst;
    }

    for (KrViewInstance *inst_dflt : std::as_const(m_registeredViews)) {
        if (inst_dflt->id() == m_defaultViewId)
            return inst_dflt;
    }

    fprintf(stderr, "Internal Error: no views registered!\n");
    exit(-1);
}
