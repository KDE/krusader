/***************************************************************************
                                krusaderview.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#ifndef KRUSADERVIEW_H
#define KRUSADERVIEW_H

// KDE includes
#include <klistview.h>
#include <klocale.h>
#include <kaccel.h>
#include <kapplication.h>
#include <kparts/part.h>

// QT includes
#include <qlayout.h>
#include <qsplitter.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qhbox.h>

class PanelManager;
class ListPanel;

// forward declaration
class KFnKeys;
class KCMDLine;

class KrusaderView : public QWidget  {
   Q_OBJECT

public:
	KrusaderView(QWidget *parent=0);
	~KrusaderView(){}
  void start(QStringList leftTabs, int leftActiveTab, QStringList rightTabs, int rightActiveTab);
  void cmdLineFocus();  // command line receive's keyboard focus
  void cmdLineUnFocus();// return focus from command line to active panel
  inline PanelManager *activeManager() const { return (activePanel==left ? leftMng : rightMng); }
  	
public slots:
  void slotCurrentChanged(QString p);
	void slotSetActivePanel(ListPanel *p);
  void slotTerminalEmulator(bool);
	// manage the function keys to the CURRENT vfs
	//////////////////////////////////////////////
  // Tab - switch focus
  void panelSwitch();
  void toggleVerticalMode();
  
  void profiles( QString profileName = QString::null );  
  void loadPanelProfiles( QString group );
  void savePanelProfiles( QString group );

protected slots:
  void killTerminalEmulator();

public:
  ListPanel  *activePanel;
  ListPanel  *left,*right;								// the actual panels
  PanelManager *leftMng, *rightMng;       // saving them for panel swaps
  KFnKeys			*fnKeys;										// function keys
  KCMDLine    *cmdLine;                   // command line widget
  QHBox       *terminal_dock;             // docking widget for terminal emulator
  KParts::ReadOnlyPart *konsole_part;     // the actual part pointer
  QSplitter		*horiz_splitter, *vert_splitter;

private:
  QGridLayout *mainLayout, *terminal_layout;
};

#endif
