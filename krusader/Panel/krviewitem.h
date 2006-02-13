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

class QString;
class QPixmap;

class KrViewItem {
	friend class KrView;

public:
   virtual inline const QString& name() const { return _vf->vfile_getName(); }
   virtual const QString dateTime() const;
   virtual const QString description() const;
   virtual bool isSelected() const = 0;
   virtual void setSelected( bool s ) = 0;
   virtual QPixmap icon();
   
	KrViewItem(vfile *vf, const KrViewProperties* properties): _vf(vf), dummyVfile(false), _viewProperties(properties) {}
   virtual ~KrViewItem() { if (dummyVfile) delete _vf; }
		
   // DON'T USE THOSE OUTSIDE THE VIEWS!!!
   virtual inline const vfile* getVfile() const { return _vf; }
   virtual inline vfile* getMutableVfile() { return _vf; }

protected:
	// used INTERNALLY when calculation of dir size changes the displayed size of the item
	inline void setSize(KIO::filesize_t size) { _vf->vfile_setSize(size); }

	vfile* _vf;			// each view item holds a pointer to a corrosponding vfile for fast access	
	bool dummyVfile;	// used in case our item represents the ".." (updir) item
	const KrViewProperties* _viewProperties;
};

#endif
