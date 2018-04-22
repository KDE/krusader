/*****************************************************************************
 * Copyright (C) 2000-2008 Csaba Karai <krusader@users.sourceforge.net>      *
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

#ifndef KRVIEWFACTORY_H
#define KRVIEWFACTORY_H

// QtCore
#include <QList>
#include <QString>
// QtGui
#include <QKeySequence>
// QtWidgets
#include <QWidget>

class KrView;
class KConfig;

/** Abstract container for KrView implementation classes. Created internally by KrViewFactory. */
class KrViewInstance
{
    friend class KrView;

public:
    inline int id() const { return m_id; }
    inline QString name() const { return m_name; }
    inline QString description() const { return m_description; }
    inline QString iconName() const { return m_iconName; }
    inline QKeySequence shortcut() const { return m_shortcut; }

    virtual KrView *create(QWidget *w, KConfig *cfg) = 0;

protected:
    KrViewInstance(int id, const QString &name, const QString &desc, const QString &iconName,
                   const QKeySequence &shortcut);
    virtual ~KrViewInstance() {}

private:
    const int m_id;
    const QString m_name;
    const QString m_description;
    const QString m_iconName;
    const QKeySequence m_shortcut;

    QList<KrView *> m_objects; // direct access in KrView
};

/** Factory for KrView implementations. This is a hidden singleton. */
class KrViewFactory
{
    friend class KrViewInstance;

public:
    static KrView *createView(int id, QWidget *widget, KConfig *cfg);
    static const QList<KrViewInstance *> &registeredViews() { return self().m_registeredViews; }
    static int defaultViewId() { return self().m_defaultViewId; }

private:
    KrViewFactory();
    void init();
    void registerView(KrViewInstance *);
    KrViewInstance *viewInstance(int id);

    static KrViewFactory &self();

    QList<KrViewInstance *> m_registeredViews;
    int m_defaultViewId;
};

#endif /* __KRVIEWFACTORY_H__ */
