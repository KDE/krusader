/***************************************************************************
                            krdetailedviewitem.h
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

#ifndef KRDETAILEDVIEWITEM_H
#define KRDETAILEDVIEWITEM_H

#include "krviewitem.h"
#include "../VFS/vfile.h"
#include <klistview.h>
#include <qobject.h>

class QPixmap;
class KrDetailedView;

class KrDetailedViewItem : public QObject, public KListViewItem, public KrViewItem {
  Q_OBJECT

friend class KrDetailedView;
public:
  KrDetailedViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf);
  QString name() const;
  QString description() const; // for status bar
  inline bool isDir() const { return (_vf ? _vf->vfile_isDir() : false); }
  inline bool isExecutable() const { return (_vf ? _vf->vfile_isExecutable() : false); }
  inline unsigned long size() const { return (_vf ? _vf->vfile_getSize() : 0); }
  inline QString dateTime() const { return (_vf ? _vf->vfile_getDateTime() : QString::null); }
  inline QString mime() const { return (_vf ? _vf->vfile_getMime() : QString::null); }
  inline QString symlinkDest() const {
   //return (_vf ? : _vf->vfile_getSymDest() : QString::null);
   if (_vf) return _vf->vfile_getSymDest();
   else return QString::null;
  }
  inline bool isSymLink() const { return (_vf ? _vf->vfile_isSymLink() : false); }
  bool isSelected() const { return (_vf ? KListViewItem::isSelected() : false); }
  void setSelected(bool s) { KListViewItem::setSelected(s); }
	/*void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment){}*/
  QPixmap& icon();
  int compare(QListViewItem *i,int col,bool ascending ) const;
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
  void repaintItem();

signals:
  void renameCancelled(KrDetailedViewItem *);

protected:
  // text() was made protected in order to catch every place where text(x) is used
  // to gain unlawful information on the object
  virtual QString text(int column) const { return KListViewItem::text(column); }

private:
  // used INTERNALLY when calculation of dir size changes the displayed size of the item
  inline void setSize(unsigned long size) { _vf->vfile_setSize(size); }

  vfile *_vf;
  KrDetailedView *_view;
};

#endif
