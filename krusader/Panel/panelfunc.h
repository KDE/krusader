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
#include <qobject.h>
#include <qptrstack.h>
#include <qthread.h>

/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */
/* Dialog calculating showing the number of files and directories and its total size
   in a dialog. If wanted, the dialog appears after 3 seconds of calculation, to
   avoid a short appearence if the result was found quickly. Computes teh result in
   a different thread.
 */
class KrCalcSpaceDialog : public KDialogBase{
	Q_OBJECT
	/* Thread which does the actual calculation. Deletes itself, if no longer
	   needed. Creator must call finished(), if the thread is no longer needed.
	*/
	class CalcThread : public QThread{
		KIO::filesize_t m_totalSize;
		unsigned long m_totalFiles;
		unsigned long m_totalDirs;
		const QStringList m_names;
		vfs * m_files;
		KrCalcSpaceDialog * m_parent;
		QMutex m_synchronizeUsageAccess;
		bool m_threadInUse; // true: caller needs the thread
		bool m_stop;
		void cleanUp(); // Deletes this, if possible
	public:
		KIO::filesize_t getTotalSize() const {return m_totalSize;} // the result
		unsigned long getTotalFiles() const {return m_totalFiles;} // the result
		unsigned long getTotalDirs() const {return m_totalDirs;} // the result
		const QStringList & getNames() const {return m_names;} // list of directories to calculate
		CalcThread(KrCalcSpaceDialog * parent, vfs * files, const QStringList & names);
		void deleteInstance(); // thread is no longer needed.
		void run(); // start calculation
		void stop(); // stop it. Thread continues until vfs_calcSpace returns
	} * m_thread;
	friend class CalcThread;
	class QTimer * m_pollTimer;
	QLabel * m_label;
	bool m_autoClose; // true: wait 3 sec. before showing the dialog. Close it, when done
	bool m_canceled; // true: cancel was pressed
	int m_timerCounter; // internal counter. The timer runs faster as the rehresh (see comment there)
	void calculationFinished(); // called if the calulation is done
	void showResult(); // show the current result in teh dialog
protected slots:
	void timer(); // poll timer was fired
	void slotCancel(); // cancel was pressed
public:
	// autoclose: wait 3 sec. before showing the dialog. Close it, when done
	KrCalcSpaceDialog(QWidget *parent, vfs * files, const QStringList & names, bool autoclose);
	~KrCalcSpaceDialog();
	KIO::filesize_t getTotalSize() const {return m_thread->getTotalSize();} // the result
	unsigned long getTotalFiles() const {return m_thread->getTotalFiles();} // the result
	unsigned long getTotalDirs() const {return m_thread->getTotalDirs();} // the result
	bool wasCanceled() const {return m_canceled;} // cancel was pressed; result is probably wrong
public slots:
	void exec(); // start calculation
};
/* End of patch by Heiner <h.eichmann@gmx.de> */

class ListPanelFunc : public QObject{
friend class ListPanel;
	Q_OBJECT
public slots:
	void execute(QString&);
	void openUrl(const QString& path);
	void refresh(){ refresh(panel->virtualPath); } // re-read the files
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
	void moveFiles();
	void pack();
	void unpack();
	void testArchive();
	void copyFiles();
	void deleteFiles();
	void calcSpace(); // calculate the occupied space and show it in a dialog

	// calculate the occupied space. A dialog appears, if calculation lasts more than 3 seconds
	// and disappears, if the calculation is done. Returns true, if the result is ok and false
	// otherwise (Cancel was pressed).
	bool calcSpace(QStringList & names,KIO::filesize_t & totalSize,unsigned long & totalFiles,unsigned long & totalDirs);
	void FTPDisconnect();
	void newFTPconnection(QString host=QString::null);
	void changeVFS(QString type, QString origin);
	inline ListPanelFunc* otherFunc(){ return panel->otherPanel->func; }

protected:
	void refresh(const QString path);

	ListPanel	     *panel;    // our ListPanel
	QStringList    backStack; // Path stack for the "back" button
	bool           inRefresh; // true when we are in refresh()
	QPtrStack<vfs> vfsStack; // the vfs stack.
};

#endif
