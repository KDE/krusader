/***************************************************************************
                   krviewfactory.h
                 -------------------
copyright            : (C) 2000-2008 by Csaba Karai
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

                                         H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
    inline QString icon() const { return m_icon; }
    inline QKeySequence shortcut() const { return m_shortcut; }

    virtual KrView *create(QWidget *w, KConfig *cfg) = 0;

protected:
    KrViewInstance(int id, const QString &name, const QString &desc, const QString &icon,
                   const QKeySequence &shortcut);
    virtual ~KrViewInstance() {}

private:
    const int m_id;
    const QString m_name;
    const QString m_description;
    const QString m_icon;
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
