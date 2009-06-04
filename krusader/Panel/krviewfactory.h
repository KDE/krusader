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

#include <QString>
#include <QKeySequence>
#include <QWidget>
#include <QList>

class KrView;
class KConfig;

typedef KrView * (*KrViewFactoryFunction)( QWidget *, bool &, KConfig * );
typedef void     (*KrViewItemHeightChange)();

class KrViewInstance {
public:
	KrViewInstance( int id, QString desc, QKeySequence shortcut, KrViewFactoryFunction fun, KrViewItemHeightChange fun2 );
	
	inline int                     id()                    { return m_id; }
	inline QString                 description()           { return m_description; }
	inline QKeySequence            shortcut()              { return m_shortcut; }
	inline KrViewFactoryFunction   factoryFunction()       { return m_factoryfun; }
	inline KrViewItemHeightChange  itemHChangeFunction()   { return m_ihchangefun; }
	
protected:
	int                            m_id;
	QString                        m_description;
	QKeySequence                   m_shortcut;
	KrViewFactoryFunction          m_factoryfun;
	KrViewItemHeightChange         m_ihchangefun;
};

class KrViewFactory {
	friend class KrViewInstance;
public:
	static KrView *                createView( int id, QWidget * widget, bool & left, KConfig *cfg );
	static void                    itemHeightChanged( int id );
	static KrViewInstance *        viewInstance( int id );
	static QList<KrViewInstance *> registeredViews()       { return self().m_registeredViews; }
	static int                     defaultViewId()         { return self().m_defaultViewId; }
	static void                    init();

private:
	KrViewFactory() : m_defaultViewId( -1 ) {}
	
	static KrViewFactory &         self();
	static void                    registerView( KrViewInstance * );

	QList<KrViewInstance *>        m_registeredViews;
	int                            m_defaultViewId;
};

#endif /* __KRVIEWFACTORY_H__ */
