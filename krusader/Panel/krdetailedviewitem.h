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
#include <sys/types.h>
#include "../VFS/vfile.h"
#include <klistview.h>
#include <qguardedptr.h>

class QPixmap;
class KrDetailedView;

class KrDetailedViewItem : public KListViewItem, public KrViewItem {

friend class KrDetailedView;
friend class KrCalcSpaceDialog;
public:
	KrDetailedViewItem(KrDetailedView *parent, QListViewItem *after, vfile *vf);
	inline QString name() const { return _vf->vfile_getName(); }
	QString description() const; // for status bar
	inline bool isDir() const { return _vf->vfile_isDir(); }
	inline bool isExecutable() const { return _vf->vfile_isExecutable(); }
	inline KIO::filesize_t size() const { return _vf->vfile_getSize(); }
	QString dateTime() const;
	inline time_t getTime_t() const { return _vf->vfile_getTime_t(); }
	inline QString mime() const { return _vf->vfile_getMime(); }
	inline QString symlinkDest() const { return _vf->vfile_getSymDest(); }
	inline bool isSymLink() const { return _vf->vfile_isSymLink(); }
	inline bool isSelected() const { return KListViewItem::isSelected(); }
	inline void setSelected(bool s) { KListViewItem::setSelected(s); }
	QPixmap icon();
	int compare(QListViewItem *i,int col,bool ascending ) const;
	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
	void repaintItem();

protected:
	// text() was made protected in order to catch every place where text(x) is used
	// to gain unlawful information on the object
	virtual inline QString text(int column) const { return KListViewItem::text(column); }

private:
	// used INTERNALLY when calculation of dir size changes the displayed size of the item
	inline void setSize(KIO::filesize_t size) { _vf->vfile_setSize(size); }
	static const QColor & setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2);
	
	KrDetailedView *_view; // <==
  
	// values are cached for faster comparisions
	bool caseSensitiveSort;
	int nameColumn;
	int sizeColumn;
	int dateTimeColumn;
	int mimeColumn;
	int krPermColumn;
	int permColumn;
	int ownerColumn;
	int groupColumn;
	int extColumn;
	bool humanReadableSize;
};

#endif
