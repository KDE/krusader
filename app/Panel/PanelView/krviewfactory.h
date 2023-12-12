/*
    SPDX-FileCopyrightText: 2000-2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    inline int id() const
    {
        return m_id;
    }
    inline QString name() const
    {
        return m_name;
    }
    inline QString description() const
    {
        return m_description;
    }
    inline QString iconName() const
    {
        return m_iconName;
    }
    inline QKeySequence shortcut() const
    {
        return m_shortcut;
    }

    virtual KrView *create(QWidget *w, KConfig *cfg) = 0;

protected:
    KrViewInstance(int id, const QString &name, const QString &desc, const QString &iconName, const QKeySequence &shortcut);
    virtual ~KrViewInstance()
    {
    }

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
    static const QList<KrViewInstance *> &registeredViews()
    {
        return self().m_registeredViews;
    }
    static int defaultViewId()
    {
        return self().m_defaultViewId;
    }

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
