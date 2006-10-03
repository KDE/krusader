/***************************************************************************
                                krspwidgets.cpp
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

#include "krspwidgets.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../kicons.h"
#include "../Filter/filtertabs.h"
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <qbitmap.h>
#include "../resources.h"

///////////////////// initiation of the static members ////////////////////////
QStrList KRSpWidgets::maskList;

///////////////////////////////////////////////////////////////////////////////

KRSpWidgets::KRSpWidgets(){
}

KRQuery KRSpWidgets::getMask(QString caption, bool nameOnly ) {
  if( !nameOnly ) {
    return FilterTabs::getQuery();
  }
  else {
    KRMaskChoiceSub *p=new KRMaskChoiceSub();
    p->setCaption(caption);
    p->exec();
    if (p->selection->currentText()=="") return KRQuery();
    else return KRQuery( p->selection->currentText() );
  }
}

/////////////////////////// newFTP ////////////////////////////////////////
KURL KRSpWidgets::newFTP() {
	newFTPSub *p=new newFTPSub();
	p->exec();
	if (p->url->currentText()=="") return KURL(); // empty url
	KURL url;
	
	QString protocol = p->prefix->currentText();
	protocol.truncate(protocol.length() - 3); // remove the trailing ://
	QString username = p->username->text().simplifyWhiteSpace();
	QString password = p->password->text().simplifyWhiteSpace();
	QString uri = p->url->currentText();

	int uriStart = uri.findRev( '@' ); /* lets the user enter user and password in the URI field */
	if( uriStart != -1 ) {
		QString uriUser = uri.left( uriStart );
		QString uriPsw = QString::null;
		uri = uri.mid( uriStart + 1 );

		int pswStart = uriUser.find( ':' ); /* getting the password name from the URL */
		if( pswStart != -1 ) {
			uriPsw = uriUser.mid( pswStart + 1 );
			uriUser = uriUser.left( pswStart );
		}

		if( !uriUser.isEmpty() )           /* handling the ftp proxy username and password also */
			username = username.isEmpty() ? uriUser : username + "@" + uriUser;

		if( !uriPsw.isEmpty() )           /* handling the ftp proxy username and password also */
			password = password.isEmpty() ? uriPsw : password + "@" + uriPsw;
	}

	QString host = uri;               /* separating the hostname and path from the uri */
	QString path = QString::null;
	int pathStart = uri.find( "/" );
	if( pathStart != -1 ) {
		path = host.mid( pathStart );
		host = host.left( pathStart );
	}

	/* setting the parameters of the URL */
	url.setProtocol(protocol);
	url.setHost( host );
	url.setPath( path );
	if ( protocol == "ftp" || protocol == "fish" || protocol == "sftp" )
		url.setPort(p->port->cleanText().toInt());
	if (!username.isEmpty())
		url.setUser( username );
	if (!password.isEmpty())
		url.setPass( password );
  
  return url;
}

newFTPSub::newFTPSub() : newFTPGUI(0,0,true) {
  url->setFocus();
  setGeometry(krApp->x()+krApp->width()/2-width()/2,krApp->y()+krApp->height()/2-height()/2,width(),height());
}

void newFTPSub::accept() {
  url->addToHistory( url->currentText() );
  // save the history and completion list when the history combo is
  // destroyed
  krConfig->setGroup("Private");
  QStringList list = url->completionObject()->items();
  krConfig->writeEntry( "newFTP Completion list", list );
  list = url->historyItems();
  krConfig->writeEntry( "newFTP History list", list );

  newFTPGUI::accept();
}

void newFTPSub::reject() {
  url->setCurrentText("");
  newFTPGUI::reject();
}

/////////////////////////// KRMaskChoiceSub ///////////////////////////////
KRMaskChoiceSub::KRMaskChoiceSub() : KRMaskChoice(0,0,true) {
  PixmapLabel1->setPixmap(krLoader->loadIcon("kr_select", KIcon::Desktop, 32));
  label->setText(i18n("Enter a selection:"));
  // the predefined selections list
  krConfig->setGroup("Private");
  QStrList lst;
  int i=krConfig->readListEntry("Predefined Selections",lst);
  if (i>0) preSelections->insertStrList(lst);
  // the combo-box tweaks
  selection->setDuplicatesEnabled(false);
  selection->insertStrList(KRSpWidgets::maskList);
  selection->lineEdit()->setText("*");
  selection->lineEdit()->selectAll();
  selection->setFocus();
}

void KRMaskChoiceSub::reject() {
  selection->clear();
  KRMaskChoice::reject();
}

void KRMaskChoiceSub::accept() {
  bool add = true;
  char *tmp;
  // make sure we don't have that already
  for ( tmp = KRSpWidgets::maskList.first(); tmp ; tmp = KRSpWidgets::maskList.next() )
    if (QString(tmp).simplifyWhiteSpace() == selection->currentText().simplifyWhiteSpace()) {
      // break if we found one such as this
      add = false;
      break;
    }

  if (add)
    KRSpWidgets::maskList.insert(0,selection->currentText().local8Bit());
  // write down the predefined selections list
  QStrList list;
  QListBoxItem *i=preSelections->firstItem();
  while (i!=0) {
    if (i->text().find(i18n("compare mode"))==-1)
      list.append(i->text().local8Bit());
    i=i->next();
  }
  krConfig->setGroup("Private");
  krConfig->writeEntry("Predefined Selections",list);
  KRMaskChoice::accept();
}

void KRMaskChoiceSub::addSelection() {
  QString temp=selection->currentText();
  bool itemExists=false;
  QListBoxItem *i=preSelections->firstItem();
  // check if the selection already exists
  while (i!=0)
    if (i->text()==temp) {
      itemExists=true;
      break;
    } else i=i->next();
  if (temp!="" && !itemExists) {
    preSelections->insertItem(selection->currentText());
    preSelections->update();
  }
}

void KRMaskChoiceSub::deleteSelection() {
  if (preSelections->currentItem()!=-1 &&
      preSelections->currentText().find(i18n("compare mode"))==-1) {
    preSelections->removeItem(preSelections->currentItem());
    preSelections->update();
  }
}

void KRMaskChoiceSub::clearSelections() {
  preSelections->clear();
  preSelections->update();
}

void KRMaskChoiceSub::acceptFromList(QListBoxItem *i) {
  selection->insertItem(i->text(),0);
  accept();
}

////////////////////////// QuickNavLineEdit ////////////////////

QuickNavLineEdit::QuickNavLineEdit(const QString &string, QWidget *parent, const char *name):
	KLineEdit(string, parent, name) { init(); }
	
QuickNavLineEdit::QuickNavLineEdit(QWidget *parent, const char *name): 
	KLineEdit(parent, name) { init(); }

int QuickNavLineEdit::findCharFromPos(const QString & str, const QFontMetrics & metrics, int pos)
{
	if (pos < 0)
		return -1;
	for (int i = 1; i <= str.length(); ++i)
		if (metrics.width(str, i) > pos)
			return i;
	return str.length();
}

void QuickNavLineEdit::init() {
	_numOfSelectedChars=0;
	_dummyDisplayed=false;
	_pop=0;
	//setCompletionMode( KGlobalSettings::CompletionPopupAuto );  ==> removed by public demand
}

void QuickNavLineEdit::leaveEvent(QEvent *) {
	clearAll();
}

void QuickNavLineEdit::mousePressEvent( QMouseEvent *m ) {
	if (m->state()!=ControlButton) clearAll();
	else
	{
		if (!_numOfSelectedChars)
		{
			_numOfSelectedChars = charCount(m);
			if (_numOfSelectedChars < 0)
				_numOfSelectedChars = 0;
		}
		if (_numOfSelectedChars)
			emit returnPressed(text().left(_numOfSelectedChars));
	}
	KLineEdit::mousePressEvent(m);
}

int QuickNavLineEdit::charCount(const QMouseEvent * const m,QString * const str) {
	// find how much of the string we've selected (approx) 
	// and select from from the start to the closet slash (on the right)
	const QString tx = text().simplifyWhiteSpace();
	if (tx.isEmpty()) {
		clearAll();
    return -1;
	}
	
	int numOfChars = findCharFromPos(tx, fontMetrics(), m->x() - 5);
	if(str) *str=tx;
	return tx.find('/', numOfChars);
}

void QuickNavLineEdit::mouseMoveEvent( QMouseEvent *m) {
	if (m->state()!=ControlButton) { // works only with ctrl pressed
		clearAll();
		KLineEdit::mouseMoveEvent(m);
		return;
	}
  QString tx;
  int idx=charCount(m,&tx);
		if (idx == -1 && !_dummyDisplayed) { // pointing on or after the current directory
			if (_pop) delete _pop;
			_pop = KPassivePopup::message( i18n("Quick Navigation"),
				"<qt>" + i18n("Already at <i>%1</i>").arg(tx.left(idx)) + "</qt>",
				*(KCursor::handCursor().bitmap()), this);

			_dummyDisplayed=true;
			_numOfSelectedChars=0;
		} else if (idx>0 && idx!=_numOfSelectedChars) {
			_numOfSelectedChars=idx;
			if (_pop) delete _pop;
			_dummyDisplayed=false;

			_pop = KPassivePopup::message( i18n("Quick Navigation"),
				"<qt>" + i18n("Click to go to <i>%1</i>").arg(tx.left(idx)) + "</qt>",
				*(KCursor::handCursor().bitmap()), this );
		}
	KLineEdit::mouseMoveEvent(m);
}

