/***************************************************************************
                            krbriefviewitem.h
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

#ifndef KRBRIEFVIEWITEM_H
#define KRBRIEFVIEWITEM_H

#include "krviewitem.h"
#include <sys/types.h>
#include "../VFS/vfile.h"
#include <kiconview.h>
#include <qguardedptr.h>

class QPixmap;
class KrBriefView;

class KrBriefViewItem : public KIconViewItem, public KrViewItem {
friend class KrBriefView;
friend class KrCalcSpaceDialog;
public:
	KrBriefViewItem(KrBriefView *parent, QIconViewItem *after, vfile *vf);
	inline bool isSelected() const { return KIconViewItem::isSelected(); }
	inline void setSelected(bool s) { KIconViewItem::setSelected(s); }
	int compare(QIconViewItem *i) const;
	void paintItem(QPainter *p, const QColorGroup &cg);
	void repaintItem() {}
	static void itemHeightChanged(); // force the items to resize when icon/font size change
	// TODO: virtual void setup(); // called when iconview needs to know the height of the item

private:
	bool initiated;
	static int expHeight;
	// TODO:
	static const QColor & setColorIfContrastIsSufficient(const QColor & /* background */, const QColor & /* color1 */, const QColor & /* color2 */ ) {static QColor col; return col;}
	
	
//	static int expHeight;
};

#endif
