/***************************************************************************
                                krviewitem.h
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
#ifndef KRVIEWITEM_H
#define KRVIEWITEM_H

#include <klistview.h>
#include "../VFS/vfile.h"

class KrDetailedView;

class KrViewItem : public KListViewItem  {
public:
  //enum cmpColor {none = 0, exclusive = 1, newer = 2, older = 4, identical = 8 };

  KrViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf);
  KrViewItem(KListView *parent ): KListViewItem( parent ), _vf(0L) {}
  inline QString name() const { return _vf->vfile_getName(); }

  QString key(int column, bool ascending) const;
  //inline int getCompareModeID() const { return compareModeID; }
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);

protected:
  // text() was made protected in order to catch every place where text(x) is used
  // to gain unlawful information on the object
  virtual QString text(int column) const { return KListViewItem::text(column); }

private:
  //int compareModeID;
  vfile *_vf;
  KrDetailedView *_view;
};

#endif
