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

#include <sys/types.h>
#include <kio/global.h>
#include "../VFS/vfile.h"
#include "krview.h"
#include <QPixmap>

class QString;
class QPixmap;

class KrViewItem {
	friend class KrView;
	friend class KrCalcSpaceDialog;

public:
	virtual const QString& name(bool withExtension=true) const;
	virtual inline bool hasExtension() const { return _hasExtension; }
	virtual inline const QString& extension() const { return _extension; }
	virtual QString dateTime() const;
	virtual QString description() const;
	virtual bool isSelected() const = 0;
	virtual void setSelected( bool s ) = 0;
	virtual QPixmap icon();
	virtual QRect itemRect() const = 0;
	
	KrViewItem(vfile *vf, const KrViewProperties* properties);
	virtual ~KrViewItem() { if (dummyVfile) delete _vf; }
		
	// DON'T USE THOSE OUTSIDE THE VIEWS!!!
	inline const vfile* getVfile() const { return _vf; }
	inline vfile* getMutableVfile() { return _vf; }
	inline bool isDummy() const { return dummyVfile; }
	inline bool isHidden() const { return _hidden; }

protected:
	// used INTERNALLY when calculation of dir size changes the displayed size of the item
	inline void setSize(KIO::filesize_t size) { _vf->vfile_setSize(size); }
	
	vfile* _vf;			// each view item holds a pointer to a corrosponding vfile for fast access	
	bool dummyVfile;	// used in case our item represents the ".." (updir) item
	const KrViewProperties* _viewProperties;
	bool _hasExtension;
	bool _hidden;
	QString _name;
	QString _extension;
};

#endif
