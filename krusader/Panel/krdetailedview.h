/***************************************************************************
                              krdetailedview.h
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
#ifndef KRDETAILEDVIEW_H
#define KRDETAILEDVIEW_H

#include <klistview.h>
#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <qwidget.h>
#include "krview.h"

class KrDetailedView : public KrView {
   Q_OBJECT
#define GROUP       "KrDetailedView"
#define MAX_COLUMNS 9
friend class KrViewItem;

public:
  enum ColumnType { Name=0x1, Extention=0x2, Mime=0x3, Size=0x4, DateTime=0x5,
                    Permissions=0x6, KrPermissions=0x7, Owner=0x8, Group=0x9, Unused=0xA };

	KrDetailedView(QWidget *parent=0, KConfig *cfg = krConfig, const char *name=0);
	~KrDetailedView();
  int column(ColumnType type);
  virtual void addItems(vfs *v, bool addUpDir = true);
  virtual inline QString nameToMakeCurrent() const { return _nameToMakeCurrent; }
  virtual inline void setNameToMakeCurrent(const QString name) { _nameToMakeCurrent = name; }
  virtual void ensureItemVisible(const QListViewItem *item) { _listview->ensureItemVisible(item); }
/////////////////////////////////////////////////////////////////////
  virtual void getSelectedItems(QStringList* names){}
  virtual QString getCurrentItem() const {}
  virtual void setCurrentItem(const QString& name) {}
  virtual void select(const QString& filter = "*") {}
  virtual void unselect(const QString& filter = "*") {}
  virtual void invertSelection() {}
  virtual void clear() {}
  virtual void setSortMode(SortSpec mode) {}
  virtual SortSpec sortMode() const { return _sortMode; }
  virtual void sort() {}
  virtual void setFilter(FilterSpec filter) {}
  virtual FilterSpec filter() const { return _filter; }
  virtual uint count() const {}
  virtual uint numFiles() const {}
  virtual uint numDirs() const {}
  virtual void saveSettings() {}

protected:
  void addColumn(ColumnType type);

private:
  KListView *_listview;
  KSqueezedTextLabel *_status, *_totals;
  ColumnType _columns[MAX_COLUMNS];
  static QString ColumnName[MAX_COLUMNS];
  QString _nameToMakeCurrent;
  bool _withIcons;
};

#endif /* KRDETAILEDVIEW_H */
