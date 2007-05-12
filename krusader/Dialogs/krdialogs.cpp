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
#include <qhbox.h>
// Krusader includes
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfs.h"
#include "../defaults.h"
#include <qdir.h>

KURL KChooseDir::getDir(QString text,const KURL& url, const KURL& cwd) {
	KURLRequesterDlg *dlg = new KURLRequesterDlg( vfs::pathOrURL( url, 1 ),text,krApp,"");
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == QDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( QDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
	}
	delete dlg;
	return u;
}

KURL KChooseDir::getDir(QString text,const KURL& url, const KURL& cwd, bool &preserveAttrs ) {
	KURLRequesterDlgForCopy *dlg = new KURLRequesterDlgForCopy( vfs::pathOrURL( url, 1 ),text, preserveAttrs, krApp,"" );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == QDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( QDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KURL KChooseDir::getDir(QString text,const KURL& url, const KURL& cwd, bool &preserveAttrs, KURL &baseURL ) {
	KURLRequesterDlgForCopy *dlg = new KURLRequesterDlgForCopy( vfs::pathOrURL( url, 1 ),text, preserveAttrs, krApp,"", true, baseURL );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KURL u;
	if (dlg->exec() == QDialog::Accepted) {
		u = vfs::fromPathOrURL(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeURL(u.url())) {
			KURL temp = u;
			u = cwd;
			u.addPath(temp.url());
			u.cleanPath();
			if( u.protocol() == "zip" || u.protocol() == "krarc" || u.protocol() == "tar" || u.protocol() == "iso" ) {
				if( QDir( u.path() ).exists() )
					u.setProtocol( "file" );
			}
		}
		
		if( dlg->copyDirStructure() ) {
			baseURL = dlg->baseURL();
		} else {
			baseURL = KURL();
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KURLRequesterDlgForCopy::KURLRequesterDlgForCopy( const QString& urlName, const QString& _text, bool presAttrs, QWidget *parent,
                                                  const char *name, bool modal, KURL baseURL )
			:   KDialogBase( Plain, QString::null, Ok|Cancel|User1, Ok, parent, name, modal, true, KStdGuiItem::clear() ),
			baseUrlCombo( 0 ), copyDirStructureCB( 0 ) {
	
	QVBoxLayout * topLayout = new QVBoxLayout( plainPage(), 0, spacingHint() );

	QLabel * label = new QLabel( _text, plainPage() );
	topLayout->addWidget( label );

	urlRequester_ = new KURLRequester( urlName, plainPage(), "urlRequester" );
	urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
	topLayout->addWidget( urlRequester_ );
	preserveAttrsCB = new QCheckBox(i18n("Preserve attributes (only for local targets)"), plainPage());
	preserveAttrsCB->setChecked( presAttrs );
	topLayout->addWidget( preserveAttrsCB );
	if( !baseURL.isEmpty() ) {
		QFrame *line = new QFrame( plainPage(), "sepLine" );
		line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
		topLayout->addWidget( line );
		copyDirStructureCB = new QCheckBox(i18n("Keep virtual directory structure"), plainPage());
		connect( copyDirStructureCB, SIGNAL( toggled( bool ) ), this, SLOT( slotDirStructCBChanged() ) );
		copyDirStructureCB->setChecked( false );
		topLayout->addWidget( copyDirStructureCB );
		QHBox * hbox = new QHBox( plainPage(), "copyDirStructure" );
		new QLabel( i18n("Base URL:"),  hbox, "baseURLLabel" );
		
		baseUrlCombo = new QComboBox( hbox, "baseUrlRequester" );
		baseUrlCombo->setMinimumWidth( baseUrlCombo->sizeHint().width() * 3 );
		baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
		KURL temp = baseURL, tempOld;
		do {
			QString baseURLText = vfs::pathOrURL( temp );
			baseUrlCombo->insertItem( baseURLText );
			tempOld = temp;
			temp = temp.upURL();
		}while( !tempOld.equals( temp, true ) );
		baseUrlCombo->setCurrentItem( 0 );
		
		topLayout->addWidget( hbox );
	}
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

bool KURLRequesterDlgForCopy::copyDirStructure() {
	if( copyDirStructureCB == 0 )
		return false;
	return copyDirStructureCB->isChecked();
}

void KURLRequesterDlgForCopy::slotTextChanged(const QString & text) {
	bool state = !text.trimmed().isEmpty();
	enableButtonOK( state );
	enableButton( KDialogBase::User1, state );
}

void KURLRequesterDlgForCopy::slotClear() {
	urlRequester_->clear();
}

void KURLRequesterDlgForCopy::slotDirStructCBChanged() {
	baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
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

KURL KURLRequesterDlgForCopy::baseURL() const {
	if( baseUrlCombo == 0 )
		return KURL();
	return vfs::fromPathOrURL( baseUrlCombo->currentText() );
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

#include "krdialogs.moc"
