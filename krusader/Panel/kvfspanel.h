/***************************************************************************
                                kvfspanel.h
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

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// KVFSPanel - Father of all panels /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class KVFSPanel : public QWidget  {
	Q_OBJECT
friend class ListPanelFunc;
friend class TreePanelFunc;
public:
	enum FilterSpec{ALL,EXEC,CUSTOM};
	
	KVFSPanel(QWidget *parent,const char *name ) : QWidget(parent,name), colorMask(255),
	                                               compareMode(false) {}
	virtual ~KVFSPanel(){}
	virtual void start(bool){}            // refresh the panel for the 1st time
	virtual QString getPath(){return "";} // returns the path the panel's in
  virtual void popBookmarks()=0;        // shows the bookmarks pop-up menu
  virtual QString getCurrentName()=0;   // returns the current file in the panel

public slots:
	virtual void openUrl( QString path, QString = QString::null){ refresh(path); }
	virtual void refresh(){ refresh(virtualPath); } // re-read the files
  virtual void refresh(const QString){} // re-read path
	virtual void setFilter(FilterSpec){}  // determinte which file types to show
  virtual void slotFocusOnMe();		      // give this VFS the focus (the path bar)
	virtual void select(bool, bool){}     // select/unselect files
  virtual void invertSelection(){}      // invert files selection
  void popRightClickMenu();
  virtual void popRightClickMenu(QListViewItem*, QPoint){}
  virtual void popRightClickMenu(KListView*,QListViewItem*, const QPoint&){}
   	
	///////////////////////// service functions - called internally ////////////////////////
	// service function for 1st time initiation
	inline void setOther(KVFSPanel *panel) { otherPanel=panel; }
	QString shortPath();	
	virtual void cleanUp(){}
	////////////////////////////// drag n' drop handlers ///////////////////////////////////
public slots:
  virtual void startDragging(int){}
protected:
  virtual void dragEnterEvent ( QDragEnterEvent * ){}
  virtual void dragLeaveEvent ( QDragLeaveEvent * ){}
  virtual void dragMoveEvent  ( QDragMoveEvent * ){}
  virtual void dropEvent      ( QDropEvent * ){}

signals:
	void signalStatus(QString msg);       // emmited when we need to update the status bar
	void cmdLineUpdate(QString p);	      // emitted when we need to update the command line
  void activePanelChanged(KVFSPanel *p);// emitted when the user changes panels
  void finishedDragging();              // currently

// these are made non-private to allow easy communications between panels
public:
  class PanelFunc	*func;	
	KVFSPanel		*otherPanel;
	QString 		 virtualPath;
	QString      realPath;
	QToolButton *bookmarkList;
	QString 		 type;
	KFileList		*fileList;
	vfs					*files;
	int          colorMask;
	bool         compareMode;
		
protected:
	bool				 focused;
	KSqueezedTextLabel *status,*totals;
	QPushButton *origin;
	QGridLayout *layout;
};


//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Tree Panel /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class TreePanel : public KVFSPanel{
friend class TreePanelFunc;
  Q_OBJECT
public:
	// this constructor creates the TREE panel, also needs to be STARTed
  TreePanel(QWidget *parent,const char *name=0);
 ~TreePanel(){}

	void start(bool left);        // see doc in KVFSPanel
  int readTree(QString path,QListViewItem *father); // reads the tree from the place marked by PATH
 	QString getPath();            // see doc in KVFSPanel
 	void popBookmarks(){}         // see doc in KVFSPanel
  QString getCurrentName();     // see doc in KVFSPanel
  // gets a list of all the selected file names
  void getSelectedNames(QStringList* fileNames) { return fileList->getSelectedNames(fileNames); }
	void slotRecreateTree(QListViewItem *item = 0);

public slots:
	void refresh(){ refresh(virtualPath); } // re-read the files
  void refresh(const QString path); // re-read path
  void popRightClickMenu(KListView*,QListViewItem*,const QPoint&);
  void slotFocusOnMe(); // see doc in KVFSPanel
  void gotStats(QString); // displays filesystem status

	///////////////////////// service functions - called internally ////////////////////////
  QString returnPath(QListViewItem *item); // converts a TREE-item to a full path
  // extract the filename from a qlistviewitem
  QString getFilename(QListViewItem *item) { return fileList->getFilename(item); }
  inline  void slotStartUpdate(){refresh();}     // internal
	void slotExpandTree(QListViewItem *item);      // expand a branch from 'item'
  void slotCollapsedTree(QListViewItem *item);   // collapse a branch from 'item'
  void slotSelectionChanged(QListViewItem *item);
  void slotChangeFromTree(QListViewItem *item){  // refreshes the other panel with filename of 'item'
    otherPanel->refresh(otherPanel->realPath=returnPath(item));
    slotFocusOnMe();
  }
  void slotGetStats(QString path);
	////////////////////////////// drag n' drop handlers ///////////////////////////////////
protected:
  void dragEnterEvent ( QDragEnterEvent * );  // internal
  void dragLeaveEvent ( QDragLeaveEvent * );  // internal
  void dragMoveEvent  ( QDragMoveEvent * );   // internal
  void dropEvent      ( QDropEvent * );       // internal
  QListViewItem* itemOn(QPoint p);            // internal
public slots:
  void startDragging(int);

protected:
	QPixmap      currDragPix;
	QListViewItem *currDragItem;
	QStringList openItems;
	KRdirWatch *watcher;
  QGuardedPtr<QObject> statsAgent;
};

//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// List Panel /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class ListPanel : public KVFSPanel{
friend class ListPanelFunc;
	Q_OBJECT
public:
 	// constructor create the panel, but DOESN'T fill it with data, use start()
  ListPanel(QWidget *parent, const bool mirrored, const char *name=0);
 ~ListPanel(){}
  void start(bool left);                        // see doc in KVFSPanel
  inline QString getPath(){return virtualPath;} // see doc in KVFSPanel
  QString getCurrentName();                     // see doc in KVFSPanel
  // gets a list of all the selected file names
  void getSelectedNames(QStringList* fileNames) { fileList->getSelectedNames(fileNames); }

public slots:
  inline void refresh(){ refresh(virtualPath); } // re-read the files
  void refresh(const QString path);
  void openUrl( QString path, QString file=QString::null );
  void gotStats(QString); // displays filesystem status
  void popRightClickMenu(KListView*,QListViewItem*,const QPoint&);
  void select(bool, bool);      // see doc in KVFSPanel
  void invertSelection();       // see doc in KVFSPanel
  virtual void slotFocusOnMe(); // give this VFS the focus (the path bar)

///////////////////////// service functions - called internally ////////////////////////
  // extract the filename from a qlistviewitem
  QString getFilename(QListViewItem *item) { return fileList->getFilename(item); }
  void setNameToMakeCurrent(QString n) { nameToMakeCurrent = n; } // internal
  void popBookmarks();                      // see doc in KVFSPanel
  void cleanUp();
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
	void setFilter(FilterSpec f);             // see doc in KVFSPanel

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
	void signalStatus(QString msg);

protected:
	QPixmap 	     getIcon(vfile* vf, KRListItem::cmpColor color);

protected:
  QString        nameToMakeCurrent;                // internal
	bool				   inRefresh;
	bool           scroll;
  FilterSpec	   filter;
	QString			   filterMask;	
	QPixmap        currDragPix;
	QListViewItem *currDragItem;
	QPopupMenu    *bookmarks;
	QStack<vfs>   *vfsStack;
	QGuardedPtr<QObject> statsAgent;
};


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// QuickView Panel /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class QuickViewPanel : public KVFSPanel{
  Q_OBJECT
public:
	QuickViewPanel(QWidget *parent,const char *name=0);
 ~QuickViewPanel(){}
  void start(bool);     // see doc in KVFSPanel
	void slotFocusOnMe(); // see doc in KVFSPanel
  // this function is inactive in this context
  QString getCurrentName() { return QString::null; }

///////////////////////// service functions - called internally ////////////////////////
	inline QString getPath(){return otherPanel->getPath();}
  void popBookmarks() {}

public slots:
	void view(QListViewItem * item);
	void viewGeneric(QString fileName);
  void viewHtml(QString fileName);
  void viewImage(QString fileName);
	void viewDefault();

protected:
	QWidget 			* workspace;
	QGridLayout 	* wsLayout;
	
	QString 				about;
	KEdit   		  * textViewer;
	QTextBrowser 	* htmlViewer;
  QWidget				* imageViewer;
	bool inWork;
	bool defaultIsOn;	
};

#endif
