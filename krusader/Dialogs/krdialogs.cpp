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
#include <kurlcompletion.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <qcheckbox.h>
// Krusader includes
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfs.h"
#include "../defaults.h"

KURL KChooseDir::getDir(QString text,const KURL& url, const KURL& cwd) {
	KURLRequesterDlg *dlg = new KURLRequesterDlg(url.prettyURL(1),text,krApp,"");
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == QDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			u = cwd;
			u.addPath(u.url());
		}
	}
	delete dlg;
	return u;
}

KRGetDate::KRGetDate(QDate date, QWidget *parent, const char *name) : KDialog(parent, name,true,WStyle_DialogBorder) {
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

int UserSelectionModeDlg::createCustomMode(QWidget *parent) {
	UserSelectionModeDlg dlg(parent);
	int result = dlg.exec();
	if (result == QDialog::Accepted) {
		{
			#define WRITE(KEY, CB)	krConfig->writeEntry(KEY, dlg.CB->isChecked())
			//KConfigGroupSaver(krConfig, "Custom Selection Mode");
			krConfig->setGroup("Custom Selection Mode");
			WRITE("QT Selection", qtSelection);
			WRITE("Left Selects", leftButtonSelects);
			WRITE("Left Preserves", leftButtonPreserves);
			WRITE("ShiftCtrl Left Selects", shiftCtrlLeftSelects);
			WRITE("Right Selects", rightButtonSelects);
			WRITE("Right Preserves", rightButtonPreserves);
			WRITE("ShiftCtrl Right Selects", shiftCtrlRightSelects);
			WRITE("Space Moves Down", spaceMovesDown);
			WRITE("Space Calc Space", spaceCalcSpace);
			WRITE("Insert Moves Down", insertMovesDown);
			WRITE("Immediate Context Menu", contextMenuImmediate);
			#undef WRITE
		}
	}
	
	return result;
}

UserSelectionModeDlg::UserSelectionModeDlg(QWidget *parent):
	KDialogBase(Plain, i18n("Custom Selection Mode"), Ok | Cancel, KDialogBase::Ok, parent) {
	QGridLayout *layout = new QGridLayout( plainPage(), 0, KDialog::spacingHint() );
	
	qtSelection = new QCheckBox(i18n("Based on QT selection mode"), plainPage());
	leftButtonSelects = new QCheckBox(i18n("Left mouse button selects"), plainPage());
	leftButtonPreserves = new QCheckBox(i18n("Left mouse button preserves selection"), plainPage());
	shiftCtrlLeftSelects = new QCheckBox(i18n("Shift/Ctrl-Left mouse button selects"), plainPage());
	rightButtonSelects = new QCheckBox(i18n("Right mouse button selects"), plainPage());
	rightButtonPreserves = new QCheckBox(i18n("Right mouse button preserves selection"), plainPage());
   shiftCtrlRightSelects = new QCheckBox(i18n("Shift/Ctrl-Right mouse button selects"), plainPage());	
	spaceMovesDown = new QCheckBox(i18n("Spacebar moves down"), plainPage());
	spaceCalcSpace = new QCheckBox(i18n("Spacebar calculates disk space"), plainPage());
	insertMovesDown = new QCheckBox(i18n("Insert moves down"), plainPage());
	contextMenuImmediate = new QCheckBox(i18n("Right clicking pops context menu immediately"), plainPage());

	layout->addMultiCellWidget(qtSelection, 0,0,0,0);
	layout->addMultiCellWidget(leftButtonSelects, 1,1,0,0);
	layout->addMultiCellWidget(leftButtonPreserves, 2,2,0,0);
	layout->addMultiCellWidget(shiftCtrlLeftSelects, 3,3,0,0);
	layout->addMultiCellWidget(rightButtonSelects, 4,4,0,0);
	layout->addMultiCellWidget(rightButtonPreserves, 5,5,0,0);
	layout->addMultiCellWidget(shiftCtrlRightSelects, 6,6,0,0);
	layout->addMultiCellWidget(spaceMovesDown, 7,7,0,0);
	layout->addMultiCellWidget(spaceCalcSpace, 8,8,0,0);
	layout->addMultiCellWidget(insertMovesDown, 9,9,0,0);
	layout->addMultiCellWidget(contextMenuImmediate, 10, 10, 0, 0);
	
	{
		krConfig->setGroup("Custom Selection Mode");
		qtSelection->setChecked(krConfig->readBoolEntry("QT Selection", _QtSelection));
		leftButtonSelects->setChecked(krConfig->readBoolEntry("Left Selects", _LeftSelects));
		leftButtonPreserves->setChecked(krConfig->readBoolEntry("Left Preserves", _LeftPreserves));
		shiftCtrlLeftSelects->setChecked(krConfig->readBoolEntry("ShiftCtrl Left Selects", _ShiftCtrlLeft));
		rightButtonSelects->setChecked(krConfig->readBoolEntry("Right Selects", _RightSelects));
		rightButtonPreserves->setChecked(krConfig->readBoolEntry("Right Preserves", _RightPreserves));
		shiftCtrlRightSelects->setChecked(krConfig->readBoolEntry("ShiftCtrl Right Selects", _ShiftCtrlRight));
		spaceMovesDown->setChecked(krConfig->readBoolEntry("Space Moves Down", _SpaceMovesDown));
		spaceCalcSpace->setChecked(krConfig->readBoolEntry("Space Calc Space", _SpaceCalcSpace));
		insertMovesDown->setChecked(krConfig->readBoolEntry("Insert Moves Down", _InsertMovesDown));
		contextMenuImmediate->setChecked(krConfig->readBoolEntry("Immediate Context Menu", _ImmediateContextMenu));
	}
}

UserSelectionModeDlg::~UserSelectionModeDlg() {
}

#include "krdialogs.moc"
