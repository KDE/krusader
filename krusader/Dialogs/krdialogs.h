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
#include <kanimwidget.h>
#include <kurlrequesterdlg.h>
#include <kdatepicker.h>
// QT includes
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qsemimodal.h>
#include <qcheckbox.h>

// the most basic dialog in krusader.
class KRDialog : public KDialog {
	Q_OBJECT
public:
	KRDialog(QWidget *parent = 0, QString text = 0,bool modal=true, bool roomForIcon=false);

protected:
	QGridLayout *layout;		
	QLabel 			*message;
};

class KChooseDir : KURLRequesterDlg {
	Q_OBJECT	
public:
	KChooseDir(QWidget *parent, QString text,QString url, QString cwd="");
public slots:
	void result();
public:
	static QString dest;
};

class KRAbout : public KRDialog {
  Q_OBJECT
public:
  KRAbout();
};

class KRGetDate : public KDialog {
  Q_OBJECT
public:
  KRGetDate(QDate date=QDate::currentDate());
  QDate getDate();

private slots:
  void setDate(QDate);

private:
  KDatePicker *dateWidget;
  QDate chosenDate, originalDate;
};

#endif
