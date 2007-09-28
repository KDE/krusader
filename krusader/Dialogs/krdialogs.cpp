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
//Added by qt3to4:
#include <Q3Frame>
#include <QLabel>
#include <Q3VBoxLayout>
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
#include <q3hbox.h>
// Krusader includes
#include "../krusader.h"
#include "../resources.h"
#include "../VFS/vfs.h"
#include "../defaults.h"
#include <qdir.h>

KUrl KChooseDir::getDir(QString text,const KUrl& url, const KUrl& cwd) {
	KUrlRequesterDialog *dlg = new KUrlRequesterDialog( vfs::pathOrUrl( url, KUrl::AddTrailingSlash ),text,krApp);
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KUrl u;
	if (dlg->exec() == QDialog::Accepted) {
		u = KUrl(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeUrl(u.url())) {
			KUrl temp = u;
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

KUrl KChooseDir::getDir(QString text,const KUrl& url, const KUrl& cwd, bool &preserveAttrs ) {
	KUrlRequesterDlgForCopy *dlg = new KUrlRequesterDlgForCopy( vfs::pathOrUrl( url, KUrl::AddTrailingSlash ),text, preserveAttrs, krApp );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KUrl u;
	if (dlg->exec() == QDialog::Accepted) {
		u = KUrl(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeUrl(u.url())) {
			KUrl temp = u;
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

KUrl KChooseDir::getDir(QString text,const KUrl& url, const KUrl& cwd, bool &preserveAttrs, KUrl &baseURL ) {
	KUrlRequesterDlgForCopy *dlg = new KUrlRequesterDlgForCopy( vfs::pathOrUrl( url, KUrl::AddTrailingSlash ),text, preserveAttrs, krApp, true, baseURL );
	dlg->urlRequester()->completionObject()->setDir(cwd.url());
	KUrl u;
	if (dlg->exec() == QDialog::Accepted) {
		u = KUrl(dlg->urlRequester()->completionObject()->replacedPath(
			dlg->urlRequester()->lineEdit()->text()));
		if (u.isRelativeUrl(u.url())) {
			KUrl temp = u;
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
			baseURL = KUrl();
		}
	}
	preserveAttrs = dlg->preserveAttrs();
	delete dlg;
	return u;
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy( const QString& urlName, const QString& _text, bool presAttrs, QWidget *parent,
                                                  bool modal, KUrl baseURL )
		:   KDialog( parent ), baseUrlCombo( 0 ), copyDirStructureCB( 0 ) {
	
	setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	setButtonGuiItem( KDialog::User1, KStandardGuiItem::clear() );
	setWindowModality( modal ? Qt::WindowModal : Qt::NonModal );
	showButtonSeparator( true );

	Q3VBoxLayout * topLayout = new Q3VBoxLayout( this, 0, spacingHint() );

	QLabel * label = new QLabel( _text, this );
	topLayout->addWidget( label );

	urlRequester_ = new KUrlRequester( urlName, this );
	urlRequester_->setMinimumWidth( urlRequester_->sizeHint().width() * 3 );
	topLayout->addWidget( urlRequester_ );
	preserveAttrsCB = new QCheckBox(i18n("Preserve attributes (only for local targets)"), this);
	preserveAttrsCB->setChecked( presAttrs );
	topLayout->addWidget( preserveAttrsCB );
	if( !baseURL.isEmpty() ) {
		QFrame *line = new Q3Frame( this, "sepLine" );
		line->setFrameStyle( Q3Frame::HLine | Q3Frame::Sunken );
		topLayout->addWidget( line );
		copyDirStructureCB = new QCheckBox(i18n("Keep virtual directory structure"), this);
		connect( copyDirStructureCB, SIGNAL( toggled( bool ) ), this, SLOT( slotDirStructCBChanged() ) );
		copyDirStructureCB->setChecked( false );
		topLayout->addWidget( copyDirStructureCB );
		Q3HBox * hbox = new Q3HBox( this, "copyDirStructure" );
		new QLabel( i18n("Base URL:"),  hbox, "baseURLLabel" );
		
		baseUrlCombo = new QComboBox( hbox, "baseUrlRequester" );
		baseUrlCombo->setMinimumWidth( baseUrlCombo->sizeHint().width() * 3 );
		baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
		KUrl temp = baseURL, tempOld;
		do {
			QString baseURLText = temp.pathOrUrl();
			baseUrlCombo->insertItem( baseURLText );
			tempOld = temp;
			temp = temp.upUrl();
		}while( !tempOld.equals( temp, KUrl::CompareWithoutTrailingSlash ) );
		baseUrlCombo->setCurrentItem( 0 );
		
		topLayout->addWidget( hbox );
	}
	urlRequester_->setFocus();
	connect( urlRequester_->lineEdit(), SIGNAL(textChanged(const QString&)),
		SLOT(slotTextChanged(const QString&)) );
	bool state = !urlName.isEmpty();
	enableButtonOk( state );
	enableButton( KDialog::User1, state );
	connect( this, SIGNAL( user1Clicked() ), SLOT( slotClear() ) );
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy() {
}

bool KUrlRequesterDlgForCopy::preserveAttrs() {
	return preserveAttrsCB->isChecked();
}

bool KUrlRequesterDlgForCopy::copyDirStructure() {
	if( copyDirStructureCB == 0 )
		return false;
	return copyDirStructureCB->isChecked();
}

void KUrlRequesterDlgForCopy::slotTextChanged(const QString & text) {
	bool state = !text.trimmed().isEmpty();
	enableButtonOk( state );
	enableButton( KDialog::User1, state );
}

void KUrlRequesterDlgForCopy::slotClear() {
	urlRequester_->clear();
}

void KUrlRequesterDlgForCopy::slotDirStructCBChanged() {
	baseUrlCombo->setEnabled( copyDirStructureCB->isChecked() );
}

KUrl KUrlRequesterDlgForCopy::selectedURL() const {
	if ( result() == QDialog::Accepted ) {
		KUrl url = urlRequester_->url();
		if( url.isValid() )
			KRecentDocument::add(url);                                
		return url;
	}        
	else
		return KUrl();
}

KUrlRequester * KUrlRequesterDlgForCopy::urlRequester() {
	return urlRequester_;
}

KUrl KUrlRequesterDlgForCopy::baseURL() const {
	if( baseUrlCombo == 0 )
		return KUrl();
	return KUrl( baseUrlCombo->currentText() );
}

KRGetDate::KRGetDate(QDate date, QWidget *parent) : KDialog(parent,Qt::WStyle_DialogBorder) {
  setWindowModality( Qt::WindowModal );
  dateWidget = new KDatePicker(this);
  dateWidget->setDate( date );
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
