/***************************************************************************
                          krpreviewpopup.h  -  description
                             -------------------
    begin                : Sun Dec 29 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
    web site             : http://krusader.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRPREVIEWPOPUP_H
#define KRPREVIEWPOPUP_H

#include <qpopupmenu.h>
#include <qpixmap.h>
#include <kfileitem.h>
#include <kurl.h>

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrPreviewPopup : public QPopupMenu {
	Q_OBJECT
public: 
	KrPreviewPopup();
	~KrPreviewPopup();

	void setUrls(const KURL::List* urls);
public slots:
	void addPreview(const KFileItem* file,const QPixmap& preview);
	void view(int id);

protected:
	KFileItemList files;
	int id;
	bool noPreview;
	KURL::List availablePreviews;
};

#endif
