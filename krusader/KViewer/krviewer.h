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
#include <qptrlist.h>
#include <kparts/mainwindow.h>
#include <ktempfile.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <qguardedptr.h>
#include <ktabwidget.h>


/**
  *@author Shie Erlich & Rafi Yanai
  */

class QPopupMenu;
class PanelViewerBase;

class KrViewer : public KParts::MainWindow {
	Q_OBJECT
public:
	virtual ~KrViewer();
	
	enum Mode{Generic,Text,Hex};

	static void view( KURL url );
	static void view( KURL url, Mode mode, bool new_window );
	static void edit( KURL url, Mode mode=Text, bool new_window=false  );


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
	void tabURLChanged( PanelViewerBase * pvb, const KURL &url );
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
	KrViewer( QWidget *parent = 0, const char *name = 0 );
	void addTab(PanelViewerBase* pvb, QString msg,QString iconName, KParts::Part* part);
	PanelViewerBase * getPanelViewerBase( KParts::Part* part);
	void updateActions( PanelViewerBase * base );
	
	static KrViewer* getViewer(bool new_window);	

	KParts::PartManager manager;
	QPopupMenu* viewerMenu;
	KTempFile tmpFile;
	KTabWidget tabBar;
	int returnFocusToKrusader;
	int detachActionIndex;

	KAction *printAction;
	KAction *copyAction;

	static QPtrList<KrViewer> viewers; // the first viewer is the active one
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
