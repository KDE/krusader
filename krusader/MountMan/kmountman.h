/***************************************************************************
                             kmountman.h
                          -------------------
 begin                : Thu May 4 2000
 copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
 e-mail               : krusader@users.sourceforge.net
 web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
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
#ifndef KMOUNTMAN_H
#define KMOUNTMAN_H

// QT includes
#include <qobject.h>
#include <qstring.h>

// KDE includes
#include <kdeversion.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kmountpoint.h>

// krusader includes
#include <stdlib.h>
#include <math.h>

class KMountMan : public QObject {
   Q_OBJECT
   friend class KMountManGUI;

public:
   enum mntStatus {DOESNT_EXIST, NOT_MOUNTED, MOUNTED};

   inline bool operational() {
      return Operational;
   } // check this 1st
   
	void mainWindow();                        // opens up the GUI
   void mount( QString mntPoint );           // this is probably what you need for mount
   void unmount( QString mntPoint );         // this is probably what you need for unmount
   mntStatus getStatus( QString mntPoint );  // return the status of a mntPoint (if any)
   void autoMount( QString path );           // just call it before refreshing into a dir
   static void eject( QString mntPoint );
   bool ejectable( QString path );
   QString convertSize( KIO::filesize_t size );
	bool invalidFilesystem(QString type);
	bool nonmountFilesystem(QString type);

   KMountMan();
   ~KMountMan();

public slots:
   void performAction( int idx );
   void quickList();

protected slots:
	void jobResult(KIO::Job *job);
	
protected:
	// used internally
	static KMountPoint *findInListByMntPoint(KMountPoint::List &lst, QString value); 
   void toggleMount( QString mntPoint ); 
		
private:
   QString *_actions;

private:
   bool Operational;   // if false, something went terribly wrong on startup
   KMountManGUI *mountManGui;
	QStringList invalid_fs;
	QStringList nonmount_fs;
};

#endif
