/***************************************************************************
                                 temp_vfs.h
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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



#ifndef TEMP_VFS_H
#define TEMP_VFS_H

#include "normal_vfs.h"

class temp_vfs : public normal_vfs  {
public: 
	temp_vfs( QString origin, QString type, QWidget* panel, bool writeable);
	~temp_vfs();
	QString vfs_workingDir();
  bool vfs_isWritable() { return false; } // temp vfs is not writable !

public slots:
	// actually reads files and stats
	bool vfs_refresh(const KUrl& origin);

protected:
	void handleAceArj(QString origin, QString type);
	void handleRpm(QString origin);
	void handleIso(QString origin);
  QString tmpDir;

};

#endif
