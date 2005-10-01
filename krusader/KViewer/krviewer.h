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
#include <qintdict.h>
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
	enum Mode{Generic,Text,Hex};

	static void view( KURL url, Mode mode=Generic, bool new_window=false );
	static void edit( KURL url, Mode mode=Text, bool new_window=false  );


public slots:
	void keyPressEvent( QKeyEvent *e );
	void createGUI( KParts::Part* );

	void viewGeneric();
	void viewText();
	void viewHex();
	void editText();

	void tabChanged(QWidget* w);
	void tabCloseRequest(QWidget *w);
	void tabCloseRequest();
	
	void checkModified();

protected:
	virtual bool queryClose();
	virtual bool queryExit();

private:
	KrViewer( QWidget *parent = 0, const char *name = 0 );
	~KrViewer();
	void addTab(PanelViewerBase* pvb, QString msg, KParts::Part* part);
	static KrViewer* getViewer(bool new_window);	

	KParts::PartManager manager;
	QPopupMenu* viewerMenu;
	KTempFile tmpFile;
	KTabWidget tabBar;
	QIconSet icon;
	QIntDict<PanelViewerBase> viewerDict;

	static KrViewer* viewer;
};

#endif
