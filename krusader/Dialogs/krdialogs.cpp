/***************************************************************************
                                krdialogs.cpp
                             -------------------
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// Krusader includes
#include "krdialogs.h"
// QT includes
#include <qmessagebox.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qfontmetrics.h>
// KDE includes
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
// Krusader includes
#include "../krusader.h"
#include "../resources.h"

KRDialog::KRDialog(QWidget *parent, QString text,bool modal, bool roomForIcon) :
					KDialog(parent,0,modal,WStyle_DialogBorder) {
	layout=new QGridLayout(this,1,1,5); 	// 1,1 makes the grid auto-resize when needed
	message=new QLabel(text,this);
	layout->addMultiCellWidget(message,0,0,0+(int)roomForIcon,10); // why 10 ?? check KQuestion::KQuestion
	layout->activate();
	setMaximumSize(minimumSize());
}	

QString KChooseDir::dest;

KChooseDir::KChooseDir(QWidget *, QString text,QString url) :
	KURLRequesterDlg(url,text,krApp,""){

	dest = QString::null;
 	connect(this,SIGNAL( okClicked() ),this,SLOT( result() ));
  exec();
}

void KChooseDir::result(){
	dest = urlRequester()->lineEdit()->text();//url();
}

KRAbout::KRAbout() : KRDialog(0,0,true,false) {
  QLabel *pic=new QLabel(this);
  QString about = KGlobal::dirs()->findResource("appdata","about.png");
  pic->setPixmap(about);
  layout->addMultiCellWidget(pic,0,0,0,10);
  layout->activate();
  setCaption(i18n("Information"));
  setGeometry(krApp->x()+krApp->width()/2-width()/2,krApp->y()+krApp->height()/2-height()/2,width(),height());
}

KRGetDate::KRGetDate(QDate date) : KDialog(0,0,true,WStyle_DialogBorder) {
  dateWidget = new KDatePicker(this, date);
  dateWidget->resize(dateWidget->sizeHint());
  setMinimumSize(dateWidget->sizeHint());
  setMaximumSize(dateWidget->sizeHint());
  resize(minimumSize());
  connect(dateWidget, SIGNAL(dateSelected(QDate)), this, SLOT(setDate(QDate)));
  connect(dateWidget, SIGNAL(dateEntered(QDate)), this, SLOT(setDate(QDate)));

  // keep the original date - incase ESC is pressed
  originalDate  = date;
}

QDate KRGetDate::getDate() {
  if (exec() == QDialog::Rejected) chosenDate.setYMD(0,0,0);
  hide();
  return chosenDate;
}

void KRGetDate::setDate(QDate date) {
  chosenDate = date;
  accept();
}

#include "krdialogs.moc"
