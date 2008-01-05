/***************************************************************************
                         krviewer.h  -  description
                            -------------------
   begin                : Thu Apr 18 2002
   copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
   email                : 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRVIEWER_H
#define KRVIEWER_H

#include <qwidget.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QEvent>
#include <QList>
#include <QKeyEvent>
#include <Q3PopupMenu>
#include <QFocusEvent>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <qpointer.h>
#include <ktabwidget.h>
#include <ktemporaryfile.h>
#include "../krusader.h"


/**
  *@author Shie Erlich & Rafi Yanai
  */

class Q3PopupMenu;
class PanelViewerBase;

class KrViewer : public KParts::MainWindow {
	Q_OBJECT
public:
	virtual ~KrViewer();
	
	enum Mode{Generic,Text,Hex};

	static void view( KUrl url, QWidget * parent = krApp );
	static void view( KUrl url, Mode mode, bool new_window, QWidget * parent = krApp );
	static void edit( KUrl url, QWidget * parent );
	static void edit( KUrl url, Mode mode=Text, int new_window=-1, QWidget * parent = krApp );
	
	virtual bool eventFilter ( QObject * watched, QEvent * e );

public slots:
	void keyPressEvent( QKeyEvent *e );
	void createGUI( KParts::Part* );

	void viewGeneric();
	void viewText();
	void viewHex();
	void editText();

	void print();
	void copy();

	void tabChanged(QWidget* w);
	void tabURLChanged( PanelViewerBase * pvb, const KUrl &url );
	void tabCloseRequest(QWidget *w);
	void tabCloseRequest();

	void nextTab();
	void prevTab();
	void detachTab();
	
	void checkModified();

protected:
	virtual bool queryClose();
	virtual bool queryExit();
	virtual void windowActivationChange ( bool oldActive );

	virtual void focusInEvent( QFocusEvent * ){ if( viewers.remove( this ) ) viewers.prepend( this ); } // move to first

private:
	KrViewer( QWidget *parent = 0 );
	void addTab(PanelViewerBase* pvb, QString msg,QString iconName, KParts::Part* part);
	PanelViewerBase * getPanelViewerBase( KParts::Part* part);
	void updateActions( PanelViewerBase * base );
	
	static KrViewer* getViewer(bool new_window);	

	KParts::PartManager manager;
	QMenu* viewerMenu;
	KTemporaryFile tmpFile;
	KTabWidget tabBar;
	QPointer<QWidget> returnFocusTo;
	PanelViewerBase * returnFocusTab;
	
	QAction *detachAction;

	KAction *printAction;
	KAction *copyAction;

	QAction *tabClose;
	QAction *closeAct;

	static Q3PtrList<KrViewer> viewers; // the first viewer is the active one
	QList<int>    reservedKeys;   // the reserved key sequences
	QList<QAction *> reservedKeyActions; // the IDs of the reserved keys
};

class Invoker : public QObject {
	Q_OBJECT
	
public:
	Invoker( QObject *recv, const char * slot ) {
		connect( this, SIGNAL( invokeSignal() ), recv, slot );
	}
	
	void invoke() {
		emit invokeSignal();
	}
	
signals:
	void invokeSignal();
};

#endif
