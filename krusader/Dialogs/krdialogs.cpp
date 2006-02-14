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
#include <qtooltip.h>
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
#include <krecentdocument.h>
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
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
		}
	}
	delete dlg;
	return u;
}

KURL KChooseDir::getDir(QString text,const KURL& url, const KURL& cwd, bool &preserveAttrs ) {
	KURLRequesterDlgForCopy *dlg = new KURLRequesterDlgForCopy(url.prettyURL(1),text, preserveAttrs, krApp,"" );        
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == QDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KURLRequesterDlgForCopy::KURLRequesterDlgForCopy( const QString& urlName, const QString& _text, bool presAttrs, QWidget *parent,
                                                  const char *name, bool modal )
			:   KDialogBase( Plain, QString::null, Ok|Cancel|User1, Ok, parent, name, modal, true, KStdGuiItem::clear() ) {
	QVBoxLayout * topLayout = new QVBoxLayout( plainPage(), 0, spacingHint() );

	QLabel * label = new QLabel( _text, plainPage() );
	topLayout->addWidget( label );

	urlRequester_ = new KURLRequester( urlName, plainPage(), "urlRequester" );
	urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
	topLayout->addWidget( urlRequester_ );
	preserveAttrsCB = new QCheckBox(i18n("Preserve attributes (only for local targets)"), plainPage());        
	preserveAttrsCB->setChecked( presAttrs );
	topLayout->addWidget( preserveAttrsCB );
	urlRequester_->setFocus();
	connect( urlRequester_->lineEdit(), SIGNAL(textChanged(const QString&)),
		SLOT(slotTextChanged(const QString&)) );
	bool state = !urlName.isEmpty();
	enableButtonOK( state );
	enableButton( KDialogBase::User1, state );
	connect( this, SIGNAL( user1Clicked() ), SLOT( slotClear() ) );
}

KURLRequesterDlgForCopy::KURLRequesterDlgForCopy() {
}

bool KURLRequesterDlgForCopy::preserveAttrs() {
	return preserveAttrsCB->isChecked();
}

void KURLRequesterDlgForCopy::slotTextChanged(const QString & text) {
	bool state = !text.stripWhiteSpace().isEmpty();
	enableButtonOK( state );
	enableButton( KDialogBase::User1, state );
}

void KURLRequesterDlgForCopy::slotClear() {
	urlRequester_->clear();
}

KURL KURLRequesterDlgForCopy::selectedURL() const {
	if ( result() == QDialog::Accepted ) {
		KURL url = KURL::fromPathOrURL( urlRequester_->url() );
		if( url.isValid() )
			KRecentDocument::add(url);                                
		return url;
	}        
	else
		return KURL();
}

KURLRequester * KURLRequesterDlgForCopy::urlRequester() {
	return urlRequester_;
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
	
	qtSelection = new QCheckBox(i18n("Based on KDE's selection mode"), plainPage());
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
	
	QToolTip::add(qtSelection, i18n("If checked, use a mode based on KDE's style."));
	QToolTip::add(leftButtonSelects, i18n("If checked, left clicking an item will select it."));
	QToolTip::add(leftButtonPreserves, i18n("If checked, left clicking an item will select it, but will not"
		" unselect other, already selected items."));
	QToolTip::add(shiftCtrlLeftSelects, i18n("If checked, shift/ctrl left clicking will select items. \nNote:"
		" This is meaningless if 'Left Button Selects' is checked."));
	QToolTip::add(rightButtonSelects, i18n("If checked, right clicking an item will select it."));
	QToolTip::add(rightButtonPreserves, i18n("If checked, right clicking an item will select it, but will not"
		" unselect other, already selected items."));
	QToolTip::add(shiftCtrlRightSelects, i18n("If checked, shift/ctrl right clicking will select items. \nNote:"
		" This is meaningless if 'Right Button Selects' is checked."));
	QToolTip::add(spaceMovesDown, i18n("If checked, pressing the spacebar will select the current item and move"
		" down. \nOtherwise, current item is selected, but remains the current item."));
	QToolTip::add(spaceCalcSpace, i18n("If checked, pressing the spacebar while the current item is a folder,"
		" will (except from selecting the folder) \ncalculate space occupied by the folder (recursively)."));
	QToolTip::add(insertMovesDown, i18n("If checked, pressing INSERT will select the current item, and move down"
		" to the next item. \nOtherwise, current item is not changed."));
	QToolTip::add(contextMenuImmediate, i18n("If checked, right clicking will result in an immediate showing of"
		" the context menu. \nOtherwise, user needs to click and hold the right mouse button for 500ms."));

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
