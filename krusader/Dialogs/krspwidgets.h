/***************************************************************************
                                krspwidgets.h
                             -------------------
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
#ifndef KRSPWIDGETS_H
#define KRSPWIDGETS_H

#include <qstrlist.h>
#include <kurl.h>
#include "krmaskchoice.h"
#include "newftpgui.h"
#include "../VFS/krquery.h"
#include <klineedit.h>
#include <kpassivepopup.h>

class newFTPGUI;

class KRMaskChoiceSub;

class KRSpWidgets {
  friend class KRMaskChoiceSub;
  
public: 
	KRSpWidgets();

  static KRQuery getMask( QString caption, bool nameOnly=false ); // get file-mask for (un)selecting files
  static KURL newFTP();

private:
  static QStrList maskList;  // used by KRMaskChoiceSub 
};

/////////////////////////// newFTPSub ///////////////////////////////////////
class newFTPSub : public newFTPGUI {
public:
  newFTPSub();

protected:
  void reject();
  void accept();
};

/////////////////////////// KRMaskChoiceSub /////////////////////////////////
// Inherits KRMaskChoice's generated code to fully implement the functions //
/////////////////////////////////////////////////////////////////////////////
class KRMaskChoiceSub : public KRMaskChoice {
public:
  KRMaskChoiceSub();

public slots:
  void addSelection();
  void deleteSelection();
  void clearSelections();
  void acceptFromList(QListBoxItem *i);
    
protected:
  void reject();
  void accept();  
};

/////////////////////////// QuickNavLineEdit //////////////////////////
// same as line edit, but hold ctrl while pointing to it... and see! //
///////////////////////////////////////////////////////////////////////

class QuickNavLineEdit: public KLineEdit {
public:
	QuickNavLineEdit(const QString &string, QWidget *parent, const char *name=0);
 	QuickNavLineEdit(QWidget *parent=0, const char *name=0);
 	virtual ~QuickNavLineEdit() {}
	static int findCharFromPos(const QString &, const QFontMetrics &, int pos);
protected:
	void mouseMoveEvent( QMouseEvent *m);
	void leaveEvent( QEvent * );
	void mousePressEvent( QMouseEvent *m );
	inline void clearAll() { _numOfSelectedChars = 0; if (_pop) delete _pop; _dummyDisplayed=false; }
	void init();
	
private:
	int _numOfSelectedChars;
	bool _dummyDisplayed;
	QGuardedPtr<KPassivePopup> _pop;
};

#endif
