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
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <klocale.h>
#include <kcombobox.h>

#include "../resources.h"

///////////////////// initiation of the static members ////////////////////////
QStrList KRSpWidgets::maskList;

///////////////////////////////////////////////////////////////////////////////

KRSpWidgets::KRSpWidgets(){
}

QString KRSpWidgets::getMask(QString caption) {
  KRMaskChoiceSub *p=new KRMaskChoiceSub();
  p->setCaption(caption);
  p->exec();
  if (p->selection->currentText()=="") return QString::null; 
    else return p->selection->currentText();
}

void KRSpWidgets::selectCompareColorMask(int *left, int *right) {
  int l = krApp->mainView->left->colorMask;
  int r = krApp->mainView->right->colorMask;

  // create the dialog, and fill in the data according to current color mask
  colorMaskDialog *dlg = new colorMaskDialog();
  dlg->newerLeft->setChecked(l & KRListItem::newer);
  dlg->olderLeft->setChecked(l & KRListItem::older);
  dlg->identicalLeft->setChecked(l & KRListItem::identical);
  dlg->exclusiveLeft->setChecked(l & KRListItem::exclusive);
  dlg->newerRight->setChecked(r & KRListItem::newer);
  dlg->olderRight->setChecked(r & KRListItem::older);
  dlg->identicalRight->setChecked(r & KRListItem::identical);
  dlg->exclusiveRight->setChecked(r & KRListItem::exclusive);

  if (!dlg->exec()) {
    *left = *right = -1;
    return;
  }

  // create masks
  (*left) = 0; (*right) = 0;
  (*left) |= (dlg->newerLeft->isChecked() ? KRListItem::newer : 0);
  (*left) |= (dlg->olderLeft->isChecked() ? KRListItem::older : 0);
  (*left) |= (dlg->identicalLeft->isChecked() ? KRListItem::identical : 0);
  (*left) |= (dlg->exclusiveLeft->isChecked() ? KRListItem::exclusive : 0);

  (*right) |= (dlg->newerRight->isChecked() ? KRListItem::newer : 0);
  (*right) |= (dlg->olderRight->isChecked() ? KRListItem::older : 0);
  (*right) |= (dlg->identicalRight->isChecked() ? KRListItem::identical : 0);
  (*right) |= (dlg->exclusiveRight->isChecked() ? KRListItem::exclusive : 0);
}

/////////////////////////// newFTP ////////////////////////////////////////
QString KRSpWidgets::newFTP() {
  newFTPSub *p=new newFTPSub();
  p->exec();
  if (p->url->currentText()=="") return QString::null;
  QString URL=p->prefix->currentText();
  if( !p->username->text().isEmpty() ){
    URL = URL + p->username->text();
    if ( !p->password->text().isEmpty() )
         URL = URL + ":" + p->password->text();
    URL = URL + "@";
  }
  URL = URL + p->url->currentText();
  if( p->prefix->currentText().startsWith("ftp") ){
    int i = URL.find("/",p->prefix->currentText().length());
		if ( i==-1 ) URL = URL + ":" + p->port->cleanText();
		else URL=URL.left(i)+":" + p->port->cleanText()+URL.mid(i);
	}

  return URL;
}

newFTPSub::newFTPSub() : newFTPGUI(0,0,true) {
  url->setFocus();
  setGeometry(krApp->x()+krApp->width()/2-width()/2,krApp->y()+krApp->height()/2-height()/2,width(),height());
}

void newFTPSub::accept() {
  url->addToHistory( url->currentText() );
  // save the history and completion list when the history combo is
  // destroyed
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
  PixmapLabel1->setPixmap(LOADICON("kr_select"));
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
  // use compare-mode if available
  if (krApp->krCompareDirs->isChecked()) {
    // an ugly hack - use the (compare mode) string as a way to distinguish
    // normal entries --> this should be fixed so that the dialog will know
    preSelections->insertItem(i18n("Newer files (compare mode)"));
    preSelections->insertItem(i18n("Older files (compare mode)"));
    preSelections->insertItem(i18n("Identical files (compare mode)"));
    preSelections->insertItem(i18n("Exclusive files (compare mode)"));
    preSelections->update();
  }
}

void KRMaskChoiceSub::reject() {
  selection->clear();
  KRMaskChoice::reject();
}

void KRMaskChoiceSub::accept() {
  QString topItem=KRSpWidgets::maskList.getFirst();
  if (topItem!=selection->currentText())
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
