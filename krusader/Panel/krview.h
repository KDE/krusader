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

class KrView : public QWidget {
  Q_OBJECT

public:
  enum SortSpec { Name=0x1, Size=0x2, Type=0x4, Modified=0x8, Permissions=0x10,
                  KrPermissions=0x20, Owner=0x40, Group=0x80, Descending=0x100,
                  DirsFirst=0x200, IgnoreCase=0x400 };
  enum FilterSpec { Dirs=0x1, Files=0x2, All=0x3, Custom=0x4, ApplyToDirs=0x8 };

protected:
  KrView(QWidget *parent, KConfig *cfg = krConfig, const char *name = 0);

public:
  virtual void addItems(vfs* v, bool addUpDir = true) = 0;
  virtual void getSelectedItems(QStringList* names) = 0;
  virtual QString getCurrentItem() const = 0;
  virtual void setCurrentItem(const QString& name) = 0;
  virtual void select(const QString& filter = "*") = 0;
  virtual void unselect(const QString& filter = "*") = 0;
  virtual void invertSelection() = 0;
  virtual void clear() = 0;
  virtual void updateView() { QWidget::repaint(); }
  virtual void setSortMode(SortSpec mode);
  virtual SortSpec sortMode() const;
  virtual void sort() = 0;
  virtual void setFilter(FilterSpec filter);
  virtual FilterSpec filter() const;
  virtual uint count() const = 0;
  virtual uint numFiles() const = 0;
  virtual uint numDirs() const = 0;
  virtual void ensureItemVisible(const QListViewItem *item) = 0;
  virtual void saveSettings() = 0;
  virtual QString nameToMakeCurrent() const = 0;
  virtual void setNameToMakeCurrent(const QString name) = 0;
  static QPixmap getIcon(vfile *vf);

  // todo: do we need firstItem(), nextItem() ??
  // todo: what about selection modes ???
  // todo: do we need a way to scroll the list, or does QT do it ?

protected:
  virtual void keyPressEvent(QKeyEvent*e);

protected:
  KConfig *_config;
  SortSpec _sortMode;
  FilterSpec _filter;
  QString _filterMask;
};

#endif /* KRVIEW_H */
