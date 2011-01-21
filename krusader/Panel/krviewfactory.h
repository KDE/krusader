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

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QKeySequence>
#include <QtGui/QWidget>

class KrView;
class KConfig;

class KrViewInstance
{
    friend class KrView;
public:

    KrViewInstance(int id, QString name, QString desc, QString icon, QKeySequence shortcut);

    inline int                     id()                    {
        return m_id;
    }
    inline QString                 name()                  {
        return m_name;
    }
    inline QString                 description()           {
        return m_description;
    }
    inline QString                 icon()                  {
        return m_icon;
    }
    inline QKeySequence            shortcut()              {
        return m_shortcut;
    }

    virtual KrView *create(QWidget *w, const bool &left, KConfig *cfg) = 0;

protected:
    int                            m_id;
    QString                        m_name;
    QString                        m_description;
    QString                        m_icon;
    QKeySequence                   m_shortcut;
    QList<KrView*>                 m_objects;
};


template< typename T >
class KrViewInstanceImpl: public KrViewInstance
{
public:
    KrViewInstanceImpl(int id, QString name, QString desc, QString icon, QKeySequence shortcut) :
        KrViewInstance(id, name, desc, icon, shortcut) {}

    virtual KrView *create(QWidget *w, const bool &left, KConfig *cfg) {
        return new T(w, *this, left, cfg);
    }
};

class KrViewFactory
{
    friend class KrViewInstance;
public:
    static KrView *                createView(int id, QWidget * widget, const bool &left, KConfig *cfg);
    static KrViewInstance *        viewInstance(int id);
    static const QList<KrViewInstance*>& registeredViews() {
        return self().m_registeredViews;
    }
    static int                     defaultViewId()         {
        return self().m_defaultViewId;
    }
    static void                    init();

private:
    KrViewFactory() : m_defaultViewId(-1) {}

    static KrViewFactory &         self();
    static void                    registerView(KrViewInstance *);

    QList<KrViewInstance *>        m_registeredViews;
    int                            m_defaultViewId;
};

#endif /* __KRVIEWFACTORY_H__ */
