/***************************************************************************
                                bookmangui.cpp
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

#include <qtabwidget.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qfontmetrics.h>
#include <kfiledialog.h>
#include "bookmangui.h"
#include "bookman.h"
#include "bookmanedit.h"
#include "../krusader.h"
#include "../resources.h"
#include "../defaults.h"
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistview.h>

BookManGUI::BookManGUI(QString path) : BookManGUIBase(0,0,true), changed(false),
  notInAccept(true) {
  // fix, should be moved to the xml file of bookman
  disconnect(nameData, 0, 0, 0);
  connect( nameData, SIGNAL( returnPressed() ), this, SLOT( accept() ) );

  // if path=="", than we opened bookMan from the menu, so don't close it
  // after adding a single bookmark - rafi's thing
  closeGUI=!(path=="" || path==QString::null);
  krConfig->setGroup("Bookmarks");
  bool sorted=(krConfig->readBoolEntry("Sorted",_Sorted));
  sortedCheckbox->setChecked(sorted);
  // if a URL was passed, open the Add Bookmark page and update data
  if (path!="") {
    urlData->setText(path); 
    nameData->setFocus();
  } else tabWidget->setCurrentPage(tabWidget->currentPageIndex()+1);
  // create the list in the 2nd tab - Edit
  krConfig->setGroup("Look&Feel");
  bookmarks->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
 	int i=QFontMetrics(bookmarks->font()).width("W");
  bookmarks->setColumnWidth(0,i*8);
  bookmarks->setColumnWidth(1,i*16);
  bookmarks->setSorting(-1);  // unsorted list
  KRBookmark *it;
  QListViewItem *item;
  for ( it=krBookMan->bookmarks.last() ; it != 0;
        it=krBookMan->bookmarks.prev() ) {
    item=new QListViewItem(bookmarks,(*it).getName(), (*it).getUrl());
  }
  // ... and ... RUN!!!
  exec();
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////// Implementation of slots /////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void BookManGUI::slotPreAddBookmark() {
  // if we got here, than the user pressed the ADD button and not
  // enter, so we conclude he don't want to close the GUI (by pressing enter)
  // again, during this session
  closeGUI=false;
  slotAddBookmark();
}

void BookManGUI::slotAddBookmark() {
  // if the URL field is empty, don't do anything
  if (urlData->text().stripWhiteSpace()=="") return;
  // if the user didn't give a name, set the name as the url
  if (nameData->text().isEmpty()) nameData->setText(urlData->text());
  // let's see if a bookmark with the same NAME exists...
  if (krBookMan->find(nameData->text()) != 0)
    KMessageBox::sorry(krApp,i18n("A bookmark with the same name already exist. If you create another one with the same name, how will you know which is which ? :-)"));
      else { // Bookmark is Kosher
        krBookMan->newBookmark(nameData->text(),urlData->text());
        // rafi's bug - close gui only if we came from the toolbar
        if (closeGUI && notInAccept) accept();
        urlData->clear();
        nameData->clear();
      }

}

void BookManGUI::accept() {  // write down the additional info
  notInAccept = false;
  slotApply();
  if (closeGUI) slotAddBookmark();
  krBookMan->loadBookmarks();
  krConfig->setGroup("Bookmarks");
  krConfig->writeEntry("Sorted",sortedCheckbox->isChecked());
  BookManGUIBase::accept();
}

void BookManGUI::reject() {
  if (changed) {
    int result=KMessageBox::warningContinueCancel(krApp,
               i18n("You have made changes to the bookmarks. All changes will be lost!"),QString::null,i18n("Ok"));
    if (result==KMessageBox::Cancel) return;
  }
  BookManGUIBase::reject();
}

void BookManGUI::slotBrowse() {
  QString temp=KFileDialog::getExistingDirectory(urlData->text(),0,i18n("Please select a directory"));
  if (temp != QString::null)
			urlData->setText(temp);
}

void BookManGUI::slotEditBookmark() {
  QListViewItem *i=bookmarks->currentItem();
  if (i==0) return;
  KRBookmark *bm=krBookMan->find(i->text(0));
  QString _name=bm->getName();
  QString _url=bm->getUrl();  // keep values for later comparision
  new BookManEdit(bm);
  if (bm->getName()!=_name || bm->getUrl()!=_url) {
    changed=true;
    i->setText(0,bm->getName());
    i->setText(1,bm->getUrl());
  }
}

void BookManGUI::slotApply() {
  krBookMan->saveBookmarks();
  changed=false;
}

void BookManGUI::slotRemoveBookmark() {
  QListViewItem *i=bookmarks->currentItem();
  if (i==0) return;
  KRBookmark *bm=krBookMan->find(i->text(0));
  krBookMan->bookmarks.remove(bm);
  delete i;
  changed=true;
  bookmarks->triggerUpdate();
}

void BookManGUI::slotUp() {
  QListViewItem *i=bookmarks->currentItem();
  if (i==0) return;
  changed=true;
  KRBookmark *bm=krBookMan->find(i->text(0));
  QString _name=bm->getName();
  unsigned int loc=krBookMan->bookmarks.find(bm);
  if (loc==0) return;  // we reached the end of the list
  // insert the item up the list and remove it from it's location, thereby
  // moving it 1 place up
  krBookMan->bookmarks.insert(loc-1,bm);
  krBookMan->bookmarks.setAutoDelete(false);
  krBookMan->bookmarks.remove(loc+1);
  krBookMan->bookmarks.setAutoDelete(true);
  // now, clear the listview and re-read it
  bookmarks->clear();
  for ( bm=krBookMan->bookmarks.last() ; bm != 0;
        bm=krBookMan->bookmarks.prev() )
    i=new QListViewItem(bookmarks,(*bm).getName(), (*bm).getUrl());
  // make our item active again
  i=bookmarks->firstChild();
  while (i && i->text(0)!=_name) i=i->itemBelow();
  bookmarks->setCurrentItem(i);
  i->setSelected(true);
  bookmarks->ensureItemVisible(i);
}

void BookManGUI::slotDown() {
  QListViewItem *i=bookmarks->currentItem();
  if (i==0) return;
  changed=true;
  KRBookmark *bm=krBookMan->find(i->text(0));
  QString _name=bm->getName();
  unsigned int loc=krBookMan->bookmarks.find(bm);
  if (loc==(krBookMan->bookmarks.count()-1)) return;  // we reached the end of the list
  // insert the item down the list and remove it from it's location, thereby
  // moving it 1 place down
  krBookMan->bookmarks.insert(loc+2,bm);
  krBookMan->bookmarks.setAutoDelete(false);
  krBookMan->bookmarks.remove(loc);
  krBookMan->bookmarks.setAutoDelete(true);
  // now, clear the listview and re-read it
  bookmarks->clear();
  for ( bm=krBookMan->bookmarks.last() ; bm != 0;
        bm=krBookMan->bookmarks.prev() )
    i=new QListViewItem(bookmarks,(*bm).getName(), (*bm).getUrl());
  // make our item active again
  i=bookmarks->firstChild();
  while (i && i->text(0)!=_name) i=i->itemBelow();
  bookmarks->setCurrentItem(i);
  i->setSelected(true);
  bookmarks->ensureItemVisible(i);
}

#include "bookmangui.moc"

