/***************************************************************************
                                krview.h
                             -------------------
    copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#ifndef KRVIEW_H
#define KRVIEW_H

#include <qpixmap.h>
#include <qvariant.h>
#include <qdict.h>
#include "../krusader.h"
#include "../VFS/vfile.h"
#include "../VFS/vfs.h"
#include "../VFS/krquery.h"

#include <kdebug.h>

class KrView;
class KrViewItem;
typedef QValueList<KrViewItem*> KrViewItemList;

// KrViewProperties
// This class is an interface class between KrView and KrViewItem
// In order for KrViewItem to be as independant as possible, KrView holds 
// an instance of this class, and fills it with the correct data. A reference
// to this should be given to each KrViewItem, which then queries it for 
// information regarding how things should be displayed in the current view.
//
// Every property that the item needs to know about the view must be here!
class KrViewProperties {
public:
	enum SortSpec { Name=0x1, Ext=0x2, Size=0x4, Type=0x8, Modified=0x10, Permissions=0x20,
						KrPermissions=0x40, Owner=0x80, Group=0x100, Descending=0x200,
						DirsFirst=0x400, IgnoreCase=0x800 };
	enum FilterSpec { Dirs=0x1, Files=0x2, All=0x3, Custom=0x4, ApplyToDirs=0x8 };
	
	bool displayIcons;	// true if icons should be displayed in this view
	SortSpec sortMode;	// sort specifications
	FilterSpec filter;	// what items to show (all, custom, exec)
	KRQuery filterMask;	// what items to show (*.cpp, *.h etc)
	bool localeAwareCompareIsCaseSensitive; // mostly, it is not! depends on LC_COLLATE
	bool humanReadableSize;		// display size as KB, MB or just as a long number
};

// operator can handle two ways of doing things:
// 1. if the view is a widget (inherits krview and klistview for example)
// 2. if the view HAS A widget (a krview-son has a member of klistview)
// this is done by specifying the view and the widget in the constructor,
// even if they are actually the same object (specify it twice in that case)
class KrViewOperator: public QObject {
	Q_OBJECT
public:
	KrViewOperator(KrView *view, QWidget *widget);
	~KrViewOperator();
	KrView *view() const { return _view; }
	QWidget *widget() const { return _widget; }
	void startDrag();
	
	void emitSelectionChanged() { emit selectionChanged(); }
	void emitGotDrop(QDropEvent *e) { emit gotDrop(e); }
	void emitLetsDrag(QStringList items, QPixmap icon ) { emit letsDrag(items, icon); }
	void emitItemDescription(QString &desc) { emit itemDescription(desc); }
	void emitContextMenu(const QPoint &point) { emit contextMenu(point); }
	void emitEmptyContextMenu(const QPoint &point) { emit emptyContextMenu(point); }
   void emitRenameItem(const QString &oldName, const QString &newName) { emit renameItem(oldName, newName); }
   void emitExecuted( QString &name ) { emit executed(name); }
   void emitNeedFocus() { emit needFocus(); }
	
signals:
	void selectionChanged();
	void gotDrop( QDropEvent *e );
	void letsDrag( QStringList items, QPixmap icon );
	void itemDescription( QString &desc );
   void contextMenu( const QPoint &point );
   void emptyContextMenu( const QPoint& point );
   void renameItem( const QString &oldName, const QString &newName );
   void executed( QString &name );
   void needFocus();

	
protected:
	// never delete those
	KrView *_view;
	QWidget *_widget;
};

/****************************************************************************
 * READ THIS FIRST: Using the view
 *
 * You always hold a pointer to KrView, thus you can only use functions declared
 * in this class. If you need something else, either this class is missing something
 * or you are ;-) Using a true name (like dynamic_cast<KrDetailedViewItem*>) should be
 * needed only when doing new(), or connect() - see example in listpanel.cpp
 *
 * The functions you'd usually want:
 * 1) getSelectedItems - returns all selected items, or (if none) the current item.
 *    it never returns anything which includes the "..", and thus can return an empty list!
 * 2) getSelectedKrViewItems - the same as (1), but returns a QValueList with KrViewItems
 * 3) getCurrentItem, setCurrentItem - work with QString
 * 4) getFirst, getNext, getPrev, getCurrentKrViewItem - all work with KrViewItems, and
 *    used to iterate through a list of items. note that getNext and getPrev accept a pointer
 *    to the current item (used in detailedview for safe iterating), thus your loop should be:
 *       for (KrViewItem *it = view->getFirst(); it!=0; it = view->getNext(it)) { blah; }
 * 5) nameToMakeCurrent(), setNameToMakeCurrent() - work with QString
 *
 * IMPORTANT NOTE: every one who subclasses this must call initProperties() in the constructor !!!
 */ 
class KrView {
public:
  // instantiating a new view
  // 1. new KrView
  // 2. view->init()
  // notes: constructor does as little as possible, setup() does the rest. esp, note that
  // if you need something from operator or properties, move it into setup()
  virtual void init();
protected:
  virtual void initProperties() { qFatal("Please implement your own initProperties() method"); }
  virtual void initOperator() { qFatal("Please implement your own initOperator() method"); }
  virtual void setup() { qFatal("Please implement your own setup() method"); }
  
  ///////////////////////////////////////////////////////
  // Every view must implement the following functions //
  ///////////////////////////////////////////////////////
public:
  virtual KrViewItem *getFirst() = 0;
  virtual KrViewItem *getNext(KrViewItem *current) = 0;
  virtual KrViewItem *getPrev(KrViewItem *current) = 0;
  virtual KrViewItem *getCurrentKrViewItem() = 0;
  virtual KrViewItem *getKrViewItemAt(const QPoint &vp) = 0;
  virtual KrViewItem *findItemByName(const QString &name) = 0;
  virtual void addItems(vfs* v, bool addUpDir = true) = 0; // kill me, kill me now
  virtual QString getCurrentItem() const = 0;
  virtual void setCurrentItem(const QString& name) = 0;
  virtual void makeItemVisible(const KrViewItem *item) = 0;
  virtual void clear();
  virtual void updateView() = 0;
  virtual void updateItem(KrViewItem* item) = 0;
  virtual void sort() = 0;
  virtual void saveSettings() = 0;
  virtual void restoreSettings() = 0;
  virtual void prepareForActive() = 0;
  virtual void prepareForPassive() = 0;
  virtual QString nameInKConfig() = 0;
  virtual void renameCurrentItem(); // Rename current item. returns immediatly

protected:
	virtual KrViewItem *preAddItem(vfile *vf) = 0;
	virtual bool preDelItem(KrViewItem *item) = 0;

public:
  //////////////////////////////////////////////////////
  // the following functions are already implemented, //
  // and normally - should NOT be re-implemented.     //
  //////////////////////////////////////////////////////
  virtual void addItem(vfile *vf);
  virtual void updateItem(vfile *vf);
  virtual void delItem(const QString &name);
  virtual uint numSelected() const { return _numSelected; }
  virtual KIO::filesize_t selectedSize() const { return _selectedSize; }
  virtual uint numFiles() const { return _count-_numDirs; }
  virtual uint numDirs() const { return _numDirs; }
  virtual uint count() const { return _count; }
  virtual KIO::filesize_t countSize() const { return _countSize; }
  virtual void getSelectedItems(QStringList* names);
  virtual void getItemsByMask(QString mask, QStringList* names, bool dirs = true, bool files = true);
  virtual void getSelectedKrViewItems(KrViewItemList *items);
  virtual void selectAllIncludingDirs() { changeSelection( KRQuery( "*" ), true, true); }
  virtual void select( const KRQuery& filter = KRQuery( "*" ) ) { changeSelection(filter, true); }
  virtual void unselect(const KRQuery& filter = KRQuery( "*" ) ) { changeSelection(filter, false); }
  virtual void invertSelection();
  virtual QString nameToMakeCurrent() const { return _nameToMakeCurrent; }
  virtual void setNameToMakeCurrent(const QString name) { _nameToMakeCurrent = name; }
  virtual QString firstUnmarkedBelowCurrent();
  virtual QString statistics();
  virtual const KrViewProperties* properties() const { return _properties; }
  virtual KrViewOperator* op() const { return _operator; }

  /////////////////////////////////////////////////////////////
  // the following functions have a default and minimalistic //
  // implementation, and may be re-implemented if needed     //
  /////////////////////////////////////////////////////////////
  virtual void setSortMode(KrViewProperties::SortSpec mode) { _properties->sortMode = mode; }
  virtual KrViewProperties::SortSpec sortMode() const { return _properties->sortMode; }
  virtual void setFilter(KrViewProperties::FilterSpec filter) { _properties->filter = filter; }
  virtual KrViewProperties::FilterSpec filter() const { return _properties->filter; }
  virtual void setFilterMask( KRQuery mask ) { _properties->filterMask = mask; }
  virtual const KRQuery& filterMask() const { return _properties->filterMask; }
  inline QWidget *widget() { return _widget; }
  inline void setWidget(QWidget *w) { _widget = w; }

  // todo: what about selection modes ???
  virtual ~KrView();
protected:
  KrView(KConfig *cfg = krConfig);
  static QPixmap getIcon(vfile *vf);
  void changeSelection(const KRQuery& filter, bool select, bool includeDirs = false);


protected:
  KConfig *_config;
  QWidget *_widget;
  QString _nameToMakeCurrent;
  uint _numSelected, _count, _numDirs;
  KIO::filesize_t _countSize, _selectedSize;
  bool _left;
  KrViewProperties *_properties;
  KrViewOperator *_operator;
  QDict<KrViewItem> _dict;
};

#endif /* KRVIEW_H */
