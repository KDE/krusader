/***************************************************************************
                                 krdialogs.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#ifndef KCHOSEDIR_H
#define KCHOSEDIR_H

// KDE includes
#include <kdialog.h>
#include <kanimatedbutton.h>
#include <kurlrequesterdlg.h>
#include <kdatepicker.h>
#include <kdialog.h>
// QT includes
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <q3progressdialog.h>
#include <qcheckbox.h>
#include <qcombobox.h>

/** \class KChooseDir
 * Used for asking the user for a folder.
 * example:
 * \code
 * KUrl u = KChooseDir::getDir("target folder", "/suggested/path", ACTIVE_PANEL->virtualPath());
 * if (u.isEmpty()) { 
 *   // user canceled (either by pressing cancel, or esc
 * } else {
 *   // do you thing here: you've got a safe url to use
 * }
 * \endcode
 */
class KChooseDir {
public:
	/**
	 * \param text - description of the info requested from the user
	 * \param url - a suggested url to appear in the box as a default choice
	 * \param cwd - a path which is the current working directory (usually ACTIVE_PANEL->virtualPath()).
	 *              this is used for completion of partial urls
	 */
	static KUrl getDir(QString text,const KUrl& url, const KUrl& cwd);
	static KUrl getDir(QString text,const KUrl& url, const KUrl& cwd, bool & preserveAttrs );
	static KUrl getDir(QString text,const KUrl& url, const KUrl& cwd, bool & preserveAttrs, KUrl &baseURL );
};

class KUrlRequesterDlgForCopy : public KDialog {
  Q_OBJECT
public:
	KUrlRequesterDlgForCopy( const QString& url, const QString& text, bool presAttrs,
				QWidget *parent, const char *name, bool modal=true, KUrl baseURL = KUrl() );
	KUrlRequesterDlgForCopy();

	KUrl selectedURL() const;
	KUrl baseURL() const;
	bool preserveAttrs();
	bool copyDirStructure();
        
	KUrlRequester *urlRequester();
private slots:
	void slotClear();
	void slotTextChanged(const QString &);
	void slotDirStructCBChanged();
private:
	KUrlRequester *urlRequester_;
	QComboBox *baseUrlCombo;
	QCheckBox *preserveAttrsCB;
	QCheckBox *copyDirStructureCB;
};

class KRGetDate : public KDialog {
  Q_OBJECT
public:
  KRGetDate(QDate date=QDate::currentDate(), QWidget *parent = 0, const char *name = 0);
  QDate getDate();

private slots:
  void setDate(QDate);

private:
  KDatePicker *dateWidget;
  QDate chosenDate, originalDate;
};

#endif
