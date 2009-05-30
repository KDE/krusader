/***************************************************************************
                                panelfunc.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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


#ifndef PANELFUNC_H
#define PANELFUNC_H
#include "listpanel.h"
#include "krviewitem.h"
#include <QtCore/QObject>
#include <QtCore/QTimer>

class ListPanelFunc : public QObject{
friend class ListPanel;
	Q_OBJECT
public slots:
	inline vfile* getVFile(KrViewItem *item) { return files()->vfs_search(item->name()); }
	inline vfile* getVFile(const QString& name) { return files()->vfs_search(name); }
	void execute(const QString&);
	void goInside(const QString&);
	void openUrl(const KUrl& path, const QString& nameToMakeCurrent = QString());
	void openUrl(const QString& path, const QString& nameToMakeCurrent = QString());
	void popErronousUrl();
	void immediateOpenUrl( const KUrl& path, bool disableLock = false);
	void doOpenUrl();
	void refresh();
	void rename(const QString &oldname, const QString &newname);

public:
	ListPanelFunc(class ListPanel *parent);
	~ListPanelFunc();

	vfs* files();  // return a pointer to the vfs

	void refreshActions();
	void redirectLink();
	void krlink(bool sym);
	void goBack();
	void dirUp();
	void properties();
	void terminal();
	void editFile();
	void view();
	void rename();
	void mkdir();
	void moveFiles( bool enqueue=false );
	void pack();
	void unpack();
	void testArchive();
	void copyFiles( bool enqueue=false );
	void deleteFiles(bool reallyDelete=false);
	void calcSpace(); // calculate the occupied space and show it in a dialog
	void createChecksum();
	void matchChecksum();
	void copyToClipboard( bool move=false );
	void pasteFromClipboard();

	// calculate the occupied space. A dialog appears, if calculation lasts more than 3 seconds
	// and disappears, if the calculation is done. Returns true, if the result is ok and false
	// otherwise (Cancel was pressed).
	bool calcSpace(const QStringList & items,KIO::filesize_t & totalSize,unsigned long & totalFiles,unsigned long & totalDirs);
	void FTPDisconnect();
	void newFTPconnection();
	inline ListPanelFunc* otherFunc(){ return panel->otherPanel->func; }

private:
	KUrl getVirtualBaseURL();

protected:
	ListPanel*           panel;     // our ListPanel
	QList<KUrl>          urlStack;  // Path stack for the "go-previous" button
	bool                 inRefresh; // true when we are in refresh()
	vfs*                 vfsP;      // pointer to vfs.
	QTimer               delayTimer;
	KUrl                 delayURL;
	bool                 delayLock;
	QString              nameToMakeCurrent;
	bool                 canGoBack();
};

#endif
