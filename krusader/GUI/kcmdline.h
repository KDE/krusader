/***************************************************************************
                                kcmdline.h
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


#ifndef KCMDLINE_H
#define KCMDLINE_H

// QT includes
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlist.h>
#include <qpopupmenu.h>

// KDE includes
#include <klineedit.h>
#include <kshellcompletion.h>

// This class adds the UP/DOWN keys functionality to KLineEdit
/////////////////////////////////////////////////////////////
class KRLineEdit : public KLineEdit {
	Q_OBJECT
public:
	KRLineEdit(QWidget *parent) : KLineEdit(parent,0) {
	  setAcceptDrops(true);
	  setCompletionMode(KGlobalSettings::CompletionAuto); // auto complete
	}

signals:
	void upkeyPressed();
	void downkeyPressed();
  void returnFocus();    // when user asks to return focus to panel
		
protected:
	void keyPressEvent(QKeyEvent *e) {
		switch (e->key()) {
			case Key_Up :
				if (e->state()==ControlButton)
				  emit returnFocus(); // user asked to return focus to panel
				   else emit upkeyPressed();
				break;			
			case Key_Down :
			  emit downkeyPressed();
			  break;
			default:
				KLineEdit::keyPressEvent(e);
				break;
		}	
	}
};

//class KRLineEdit;

class KCMDLine : public QWidget  {
   Q_OBJECT
public: 
	KCMDLine(QWidget *parent=0, const char *name=0);
	~KCMDLine();
	void setCurrent(const QString &);
	
public slots:
  inline void setFocus() { cmdLine->setFocus(); } // overloaded for KCmdLine
  void slotReturnFocus(); // returns keyboard focus to panel
  void slotRun();
  void slotHistoryUp();
  void slotHistoryDown();
  void setCmdLineText(int id);

private:
	void addToHistory(QString cmd);
  void refreshHistory();
		
private:
  QLabel    			 *path;
  KRLineEdit       *cmdLine;
  QToolButton      *terminal,*history;
  QPopupMenu			 *historyMenu;
  QList<QString>    historyList;
  int               historySize,currentItem;
	KShellCompletion  completion;
};


#endif
