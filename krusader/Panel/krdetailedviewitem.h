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

#ifndef QT3_SUPPORT
#define QT3_SUPPORT 1
#define KDE3_SUPPORT 1
#endif

#include "krviewitem.h"
#include <sys/types.h>
#include "../VFS/vfile.h"
#include <k3listview.h>
#include <qpointer.h>
#include <QPixmap>

#define FASTER

class QPixmap;
class KrDetailedView;

class KrDetailedViewItem : public K3ListViewItem, public KrViewItem {
friend class KrDetailedView;
public:
	KrDetailedViewItem(KrDetailedView *parent, Q3ListViewItem *after, vfile *vf);
	inline bool isSelected() const { return K3ListViewItem::isSelected(); }
	inline void setSelected(bool s) { K3ListViewItem::setSelected(s); repaint(); }
	int compare(Q3ListViewItem *i,int col,bool ascending ) const;
	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
	void paintFocus(QPainter *p, const QColorGroup &cg, const QRect &r);
	void repaintItem();
	static void itemHeightChanged(); // force the items to resize when icon/font size change
#ifdef FASTER		
	virtual void setup(); // called when listview needs to know the height of the item
#endif
        virtual QRect itemRect() const { return listView()->itemRect( this ); }
        virtual void redraw() { repaintItem(); repaint(); }
protected:
	// text() was made protected in order to catch every place where text(x) is used
	// to gain unlawful information on the object
	virtual inline QString text(int column) const { return K3ListViewItem::text(column); }

private:
	static const QColor & setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2);
#ifdef FASTER	
	bool initiated;
	static int expHeight;
#endif
};

#endif
