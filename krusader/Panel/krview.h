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
#include "../krusader.h"
#include "../VFS/vfile.h"
#include "../VFS/vfs.h"
#include "krviewitem.h"

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
 */

/////////////////////////////////////////////////////////////////
// the following settings are available for ALL kinds of views
//
// With Icons
#define _WithIcons          true
// View Background
#define _ViewBackground     new QColor("white")
// Single Click Selects
#define _SingleClickSelects false

typedef QValueList<KrViewItem*> KrViewItemList;
class KrView {
public:
  enum SortSpec { Name=0x1, Ext=0x2, Size=0x4, Type=0x8, Modified=0x10, Permissions=0x20,
                  KrPermissions=0x40, Owner=0x80, Group=0x100, Descending=0x200,
                  DirsFirst=0x400, IgnoreCase=0x800 };
  enum FilterSpec { Dirs=0x1, Files=0x2, All=0x3, Custom=0x4, ApplyToDirs=0x8 };


public:
  ///////////////////////////////////////////////////////
  // Every view must implement the following functions //
  ///////////////////////////////////////////////////////
  virtual KrViewItem *getFirst() = 0;
  virtual KrViewItem *getNext(KrViewItem *current) = 0;
  virtual KrViewItem *getPrev(KrViewItem *current) = 0;
  virtual KrViewItem *getCurrentKrViewItem() = 0;
  virtual KrViewItem *getKrViewItemAt(const QPoint &vp) = 0;
  virtual void addItems(vfs* v, bool addUpDir = true) = 0;
  virtual QString getCurrentItem() const = 0;
  virtual void setCurrentItem(const QString& name) = 0;
  virtual void makeItemVisible(const KrViewItem *item) = 0;
  virtual void clear() = 0;
  virtual void updateView() = 0;
  virtual void sort() = 0;
  virtual void saveSettings() = 0;
  virtual void restoreSettings() = 0;
  virtual void prepareForActive() = 0;
  virtual void prepareForPassive() = 0;
  virtual QString nameInKConfig() = 0;
  virtual void renameCurrentItem() = 0; // Rename current item. returns immediatly
  // also, the following must be implemented (but must be remarked here)
  // signals:
  //   void letsDrag(QStringList items, QPixmap icon);
  //   void gotDrop(QDropEvent *);
  //   void selectionChanged();  

  //////////////////////////////////////////////////////
  // the following functions are already implemented, //
  // and normally - should NOT be re-implemented.     //
  //////////////////////////////////////////////////////
  virtual uint numSelected() const { return _numSelected; }
  virtual KIO::filesize_t selectedSize() const { return _selectedSize; }
  virtual uint numFiles() const { return _count-_numDirs; }
  virtual uint numDirs() const { return _numDirs; }
  virtual uint count() const { return _count; }
  virtual KIO::filesize_t countSize() const { return _countSize; }
  virtual void getSelectedItems(QStringList* names);
  virtual void getSelectedKrViewItems(KrViewItemList *items);
  virtual void select(const QString& filter = "*") { changeSelection(filter, true); }
  virtual void unselect(const QString& filter = "*") { changeSelection(filter, false); }
  virtual void invertSelection();
  virtual inline QString nameToMakeCurrent() const { return _nameToMakeCurrent; }
  virtual inline void setNameToMakeCurrent(const QString name) { _nameToMakeCurrent = name; }
  virtual QString firstUnmarkedAboveCurrent();
  virtual QString statistics();

  /////////////////////////////////////////////////////////////
  // the following functions have a default and minimalistic //
  // implementation, and may be re-implemented if needed     //
  /////////////////////////////////////////////////////////////
  virtual void setSortMode(SortSpec mode) { _sortMode = mode; }
  virtual SortSpec sortMode() const { return _sortMode; }
  virtual void setFilter(FilterSpec filter) { _filter = filter; }
  virtual FilterSpec filter() const { return _filter; }
  inline QWidget *widget() { return _widget; }
  inline void setWidget(QWidget *w) { _widget = w; }

  /////////////////remove/////////////////
  virtual QString itemToFilename(QListViewItem *it) = 0;


  // todo: what about selection modes ???
protected:
  KrView(KConfig *cfg = krConfig);
  virtual ~KrView() {}
  static QPixmap getIcon(vfile *vf);
  void changeSelection(const QString& filter, bool select);


protected:
  KConfig *_config;
  SortSpec _sortMode;
  FilterSpec _filter;
  QString _filterMask;
  QWidget *_widget;
  QString _nameToMakeCurrent;
  uint _numSelected, _count, _numDirs;
  KIO::filesize_t _countSize, _selectedSize;
  bool _left;
};

#endif /* KRVIEW_H */
