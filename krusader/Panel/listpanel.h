/***************************************************************************
                                listpanel.h
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


#ifndef KVFSPANEL_H
#define KVFSPANEL_H

#include <kpropsdlg.h>
#include <kfileitem.h>
#include <kurl.h>
#include "kfilelist.h"
#include "krlistitem.h"
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qdir.h>
#include <qpixmapcache.h>
#include <qiconset.h>
#include <qstack.h>
#include <qtextbrowser.h>
#include <keditcl.h>
#include <qguardedptr.h>
#include <ksqueezedtextlabel.h>

class KFileList;
class vfs;
class vfile;
class KRdirWatch;
class PanelFunc;
class statsCollector;

class ListPanel : public QWidget {
friend class ListPanelFunc;
	Q_OBJECT
public:
	enum FilterSpec{ALL,EXEC,CUSTOM};

 	// constructor create the panel, but DOESN'T fill it with data, use start()
  ListPanel(QWidget *parent, const bool mirrored, const char *name=0);
 ~ListPanel(){}
  void start(bool left);                        
  inline QString getPath(){return virtualPath;} 
  QString getCurrentName();                     
  // gets a list of all the selected file names
  void getSelectedNames(QStringList* fileNames) { fileList->getSelectedNames(fileNames); }
	inline void setOther(ListPanel *panel) { otherPanel=panel; }

public slots:
  void gotStats(QString); // displays filesystem status
	void popRightClickMenu();
  void popRightClickMenu(KListView*,QListViewItem*,const QPoint&);
  void select(bool, bool);      
  void invertSelection();      
  virtual void slotFocusOnMe(); // give this Panel the focus (the path bar)

  ///////////////////////// service functions - called internally ////////////////////////
  // extract the filename from a qlistviewitem
  QString getFilename(QListViewItem *item) { return fileList->getFilename(item); }
  void setNameToMakeCurrent(QString n) { nameToMakeCurrent = n; } // internal
  void popBookmarks();                      
 	void prepareToDelete();                   // internal use only

public slots:
	void resizeEvent( QResizeEvent *e );      // <rewrite> check what this does -- move to filelist ?
	void slotBookmarkChosen(int id);
  void slotRefreshBookmarks();              // re-reads the bookmark list from disk
	void slotUpdateTotals();                  // update file totals
	void slotUpdate();			                  // when the vfs finish to update...
	void slotStartUpdate();                   // internal
	void slotEndUpdate();                     // internal
	void slotChangeStatus(QListViewItem *i);  // update status bar
	void slotGetStats(QString path);          // get the disk-free stats
  void slotSelectionChanged(QListViewItem *item);
	void setFilter(FilterSpec f);             

////////////////////////////// drag n' drop handlers ///////////////////////////////////
protected:
  void dragEnterEvent ( QDragEnterEvent * );// internal
  void dragLeaveEvent ( QDragLeaveEvent * );// internal
  void dragMoveEvent  ( QDragMoveEvent * ); // internal
  void dropEvent      ( QDropEvent * );     // internal
  QListViewItem* itemOn(QPoint p);          // internal
public slots:
  void startDragging  (int);                // internal

signals:
	void signalStatus(QString msg);       // emmited when we need to update the status bar
	void cmdLineUpdate(QString p);	      // emitted when we need to update the command line
  void activePanelChanged(ListPanel *p);// emitted when the user changes panels
  void finishedDragging();              // currently

public:
  ListPanelFunc	*func;
	ListPanel		*otherPanel;
	QString 		 virtualPath;
	QString      realPath;
	QToolButton *bookmarkList;
	KFileList		*fileList;
	int          colorMask;
	bool         compareMode;

protected:
	QPixmap 	     getIcon(vfile* vf, KRListItem::cmpColor color);

  QString        nameToMakeCurrent;                // internal
	bool           scroll;
  FilterSpec	   filter;
	QString			   filterMask;	
	QPixmap        currDragPix;
	QListViewItem *currDragItem;
	QPopupMenu    *bookmarks;
	QGuardedPtr<QObject> statsAgent;
	bool				   focused;
	KSqueezedTextLabel *status,*totals;
	QPushButton   *origin;
	QGridLayout   *layout;
};

#endif
