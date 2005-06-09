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
   virtual QString name() const = 0;
   virtual QString dateTime() const = 0;
   virtual QString description() const = 0;
   virtual bool isSelected() const = 0;
   virtual void setSelected( bool s ) = 0;
   virtual QPixmap icon() = 0;
   
	KrViewItem(vfile *vf, const KrViewProperties* properties): _vf(vf), dummyVfile(false), _viewProperties(properties) {}
   virtual ~KrViewItem() { if (dummyVfile) delete _vf; }
		
   // DON'T USE THOSE OUTSIDE THE VIEWS!!!
   virtual inline vfile* getVfile() const { return _vf; }
   virtual inline vfile* getMutableVfile() { return _vf; }

protected:
	vfile* _vf;			// each view item holds a pointer to a corrosponding vfile for fast access	
	bool dummyVfile;	// used in case our item represents the ".." (updir) item
	const KrViewProperties* _viewProperties;
};

#endif
