/***************************************************************************
                                kfilelist.cpp
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

#include "kfilelist.h"
#include "../krusader.h"
#include "../GUI/kcmdline.h"
#include "krlistitem.h"
#include "../defaults.h"
#include <qdir.h>
#include <qdragobject.h>
#include <kglobalsettings.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qheader.h>
#include "../krusaderview.h"
#include "kvfspanel.h"
#include "../krslots.h"
#include "../resources.h"
#include <kdeversion.h>

KFileList::KFileList(QWidget *parent, const char *name ) :
  KListView(parent,name) {
  numOfSelected=0;
  dragSX=dragSY=0;
  dragState=nothing;
  pressX=pressY=0;
  stillPressed=false;
  stillScrolling=false;
  setMouseTracking(false);
  setDragAutoScroll(true);
  setSelectionModeExt(KListView::FileManager);
  toolTip=false;
  toolTipItem=0;
}

/////////////////////////////////// slots ////////////////////////////////////////
void KFileList::select(QString filter) {
  krConfig->setGroup("Look&Feel");
  bool markDirs = krConfig->readBoolEntry("Mark Dirs",_MarkDirs);
  // ugly hack !!! see info in TODO regarding getMask gui
  bool compareMode = filter.find(i18n("compare mode")) != -1;

  QListViewItem *temp=currentItem();
  QListViewItem *iterator=firstChild();
  while (iterator!=0) {
    if (iterator->text(1)==QString("<DIR>") && !markDirs) {
      iterator=iterator->itemBelow();
      continue;
    }
    // ugly hack - all of it!
    if (compareMode) {
      KRListItem::cmpColor color = KRListItem::none;
      if (filter.find(i18n("Newer files")) != -1) color = KRListItem::newer;
      if (filter.find(i18n("Older files")) != -1) color = KRListItem::older;
      if (filter.find(i18n("Identical files")) != -1) color = KRListItem::identical;
      if (filter.find(i18n("Exclusive files")) != -1) color = KRListItem::exclusive;
      if (dynamic_cast<KRListItem*>(iterator)->getCompareModeID() == color) {
        if (!iterator->isSelected()) ++numOfSelected;
        iterator->setSelected(true);
        iterator=iterator->itemBelow();
      } else iterator=iterator->itemBelow();
    } else
    if (QDir::match(filter,iterator->text(0))) {
      // we're increasing the number of selected files
      if (!iterator->isSelected()) ++numOfSelected;
      iterator->setSelected(true);
      iterator=iterator->itemBelow();
    } else iterator=iterator->itemBelow();
  }
  triggerUpdate();
  ensureItemVisible(temp);
  emit selectionChanged();
}

void KFileList::unselect(QString filter) {
  QListViewItem *temp=currentItem();
  QListViewItem *iterator=firstChild();
  while (iterator!=0) {
    if (iterator->isSelected() && QDir::match(filter,iterator->text(0)))
      // we're decreasing the number of selected files
      if (iterator->isSelected()) --numOfSelected;
      iterator->setSelected(false);
    iterator=iterator->itemBelow();
  }
  triggerUpdate();
  ensureItemVisible(temp);
  emit selectionChanged();
}

void KFileList::invertSelection() {
  QListViewItem *temp=currentItem();
  QListViewItem *iterator=firstChild();
  while (iterator!=0) {
    if (iterator->isSelected()) {
      --numOfSelected;
      iterator->setSelected(false);
      iterator=iterator->itemBelow();
      continue;
    }
    if (iterator->text(1)==QString("<DIR>") && !krConfig->readBoolEntry("Mark Dirs",_MarkDirs)); //NOP
      else {
        iterator->setSelected(true);
        ++numOfSelected;
      }
    iterator=iterator->itemBelow();
  }
  triggerUpdate();
  ensureItemVisible(temp);
  emit selectionChanged();
}

void KFileList::keyPressEvent(QKeyEvent *e) {
  QKeyEvent *ne;

  if ( !e || !firstChild() )	return; // subclass bug
  switch (e->key()) {
    case Key_Enter  :
    case Key_Return :
      QListView::keyPressEvent(e);
      return;
    case Key_Right :
      if (e->state()==ControlButton) { // user pressed CTRL+Right
       if (krApp->mainView->activePanel==krApp->mainView->left) {
        // refresh the other panel (if possible) with our path
        if (krApp->mainView->activePanel->otherPanel->type=="list") {
          krApp->mainView->activePanel->otherPanel->openUrl(
            krApp->mainView->activePanel->realPath);
          krApp->mainView->activePanel->otherPanel->slotFocusOnMe();
          return;
        }
       }
      } else
      if (e->state()==AltButton) { // user pressed Alt+Right
       if (krApp->mainView->right->type=="list") { // and the right panel is list
        KVFSPanel *p=krApp->mainView->activePanel;
        krApp->mainView->right->popBookmarks();
        p->slotFocusOnMe();
        return;
       }
      } else { // just a normal click - do a lynx-like moving thing
        QListViewItem *iterator=currentItem();
        if (iterator->text(1)=="<DIR>")  // we create a return-pressed event,
          emit returnPressed(iterator);  // thereby emulating a chdir
        return; // safety
      }
    case Key_Left  :
      if (e->state()==ControlButton) { // user pressed CTRL+Left
       if (krApp->mainView->activePanel==krApp->mainView->right) {
        // refresh the other panel (if possible) with our path
        if (krApp->mainView->activePanel->otherPanel->type=="list") {
          krApp->mainView->activePanel->otherPanel->openUrl(
            krApp->mainView->activePanel->realPath);
          krApp->mainView->activePanel->otherPanel->slotFocusOnMe();
          return;
        }
       }
      } else
      if (e->state()==AltButton) { // user pressed Alt+Right
       if (krApp->mainView->left->type=="list") { // and the right panel is list
        KVFSPanel *p=krApp->mainView->activePanel;
        krApp->mainView->left->popBookmarks();
        p->slotFocusOnMe();
        return;
       }
      } else {  // a normal click - do a lynx-like moving thing
        SLOTS->dirUp(); // ask krusader to move up a directory
        return; // safety
      }
    case Key_Down :
      if (e->state()==ControlButton) { // user pressed CTRL+Down
        // give the keyboard focus to the command line
        if (krApp->mainView->cmdLine->isVisible())
          krApp->mainView->cmdLineFocus();
        else if (krApp->mainView->terminal_dock->isVisible())
          krApp->mainView->terminal_dock->setFocus();
        return;
      } else QListView::keyPressEvent(e);
      break;
    case Key_Backspace :      // dir up
      if (e->state()==0) SLOTS->dirUp();            // dirUp
      return;
    case Key_QuoteLeft :     // home
      SLOTS->home();
      return;
    case Key_Delete :         // kill file
      SLOTS->deleteFiles();
      return;
    default:
      QListView::keyPressEvent(e);
      return;
  }
}


void KFileList::markCurrent(bool movedown) {
	QListViewItem *iterator=currentItem();
	if (iterator->isSelected()) --numOfSelected;
	else ++numOfSelected;
	iterator->setSelected(!iterator->isSelected());
	if (movedown) setCurrentItem(iterator->itemBelow());
	ensureItemVisible(currentItem());
	emit(selectionChanged());
	triggerUpdate();
}

void KFileList::contentsMouseMoveEvent(QMouseEvent *e) {
  if (e->state() & LeftButton) {
    if (dragState==dragging) return;
    if (dragState==pending) // let's check how far the mouse moved
    if ( ( QPoint(dragSX,dragSY) - QPoint(e->x(),e->y()) ).manhattanLength() >
      QApplication::startDragDistance() ) { // <patch> thanks to Cristi Dumitrescu
         dragState=dragging;
         dragSX=0;dragSY=0;
         if (e->state() & ShiftButton) emit letsDrag(1); // move drag
          else emit letsDrag(0);                         // copy/link drag
         return;
      } else return;
    // if we got here, dragState==nothing, so let's start a pseudo-drag
    dragSX=e->x(); dragSY=e->y();
    dragState=pending;
  } else
  if (e->state() & RightButton) QListView::contentsMouseMoveEvent(e);
    else
  if (e->state() & MidButton) {}  // future use ???
}

/*void KFileList::viewportMouseMoveEvent(QMouseEvent *e) {
  if (e->state()!=0) QListView::viewportMouseMoveEvent(e);  // a button is pressed
  // a simple move with no button pressed
  QListViewItem *i=itemAt(e->pos());  // are we pointing on something ?
  if (!i) return;                     // if not, exit.
  emit onItem(i);                     // otherwise, let the world know!
 	if (toolTipItem==i) return;         // don't delete tooltip until we leave the item
 	if (toolTip) {                      // if there's a pending tooltip, remove it
 	  QToolTip::remove(this);           // first and then create a new one
 	  toolTip=false;
 	  toolTipItem=0;
 	}
 	if (!toolTip) {                     // ... here !
// 	  if (i->
// 	  toolTipItem=i;
// 	  QToolTip::add(this,i->text(0));
// 	  toolTip=true;
 	}
} */

/*void KFileList::contentsMousePressEvent(QMouseEvent *e) {
  krConfig->setGroup("Look&Feel");
  mouseSelType=krConfig->readNumEntry("Mouse Selection",_MouseSelection);
  if (e->button()==RightButton) { // for right-click only
    // record mouse location for future reference
    pressX=e->pos().x(); pressY=e->pos().y();
    QListViewItem* i=itemAt(e->pos());
    yDelta = pressY - QCursor::pos().y();
    if (yDelta<0) yDelta = -yDelta;
    stillPressed=true;
    // after some time, we will check if we need to pop a rightclick menu
    switch (mouseSelType) {
      case 0 :
      case 1 : QTimer::singleShot( 60, this, SLOT(checkForRightClickMenu()));
               break;
      // right-click selection
      case 2 : QTimer::singleShot( 200, this, SLOT(checkForRightClickMenu()));
               break;
    }
    // if we're doing the classic-krusader selection method, call the listview
    // function, to allow the swush thing
    switch (mouseSelType) {
      // classic-mode, let the list handle it
      case 0 : KListView::contentsMousePressEvent(e);
               break;
      // left-click selection, just make it current
      case 1 : if (i) setCurrentItem(i);
               emit mouseButtonPressed(e->button(),i, e->pos(), 1);
               break;
      // right-click selection, we'd have to make this current anyway,
      case 2 : if (i) setCurrentItem(i);
               emit mouseButtonPressed(e->button(),i, e->pos(), 1);
               break;
    }
  }
  if (e->button()==LeftButton) {
    // record mouse location for future reference
    pressX=e->pos().x(); pressY=e->pos().y();
    QListViewItem *i = itemAt(e->pos());
    switch (mouseSelType) {
      // classic-mode, let the list handle it
      case 0 : KListView::contentsMousePressEvent(e);
               break;
      // left-key selection, so don't do nothing here
      case 1 : emit mouseButtonPressed(e->button(),i, e->pos(), 1);
               break;
      // right-key selection, so left makes current
      case 2 : setCurrentItem(i);
               emit mouseButtonPressed(e->button(),i, e->pos(), 1);
               break;
    }
  }
}

void KFileList::contentsMouseReleaseEvent(QMouseEvent *e) {
  krConfig->setGroup("Look&Feel");
  mouseSelType=krConfig->readNumEntry("Mouse Selection",_MouseSelection);
  if (e->button()==RightButton) { // for right-click only
    QPoint p=QPoint(pressX,pressY);
    QListViewItem* i=itemAt(p);
    stillPressed = false; // released button
    if (!i) return; // did the user click on a item in the list?
    switch (mouseSelType) {
      // classic-mode, the listview handles it
      case 0 : QListView::contentsMouseReleaseEvent(e);
               break;
      // left-click selection, do nothing ... it's already current
      case 1 :  break;
      // right-click selection mode
      case 2 : bool tmp=i->isSelected();
               if (tmp) --numOfSelected; else ++numOfSelected;
               i->setSelected(!tmp);
               emit(selectionChanged());
               triggerUpdate(); // refresh panel
               break;
    }
  }
  if (e->button()==LeftButton) { // handle left-click
    QPoint p=QPoint(pressX,pressY);
    QListViewItem* i=itemAt(p);
    stillPressed = false; // released button
    if (!i) return; // did the user click on a item in the list?
    switch (mouseSelType) {
      // classic-mode, the listview handles it
      case 0 : QListView::contentsMouseReleaseEvent(e);
               break;
      // right-click selection mode
      case 2 : break;
      // left-click selection
      case 1 : bool tmp=i->isSelected();
               if (tmp) --numOfSelected; else ++numOfSelected;
               i->setSelected(!tmp);
               setCurrentItem(i);
               emit(selectionChanged());
               triggerUpdate(); // refresh panel
               break;
    }
  }
} */

/*void KFileList::checkForRightClickMenu() {
  if (stillPressed) { // if the right-button is still pressed
    // check if we moved enough - assume "swush"
    int delta = QCursor::pos().y() - (pressY + yDelta);
    if (delta<0) delta=-delta;
    if (delta > KGlobalSettings::dndEventDelay()) return; // swush !
    // no swush ... pop a menu
    QPoint p=QPoint(pressX,pressY);
    QListViewItem* i=itemAt(p);
    stillPressed=false;
    if (!i) return; // if we didn't click on an item ...
    // or if we clicked on the "root decoration"
    if ( !( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
         treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) +
         itemMargin() || p.x() < header()->cellPos( header()->mapToActual( 0 ))) )
       return;
    // if we got here, no root decoration, and the item we clicked on is actual
    // but first, if we're using non-classic selection, and some other files BUT NOT US are
    // selected, and the user clicked this file, then he obviously want to include it in the
    // right-click menu
    if (mouseSelType > 0 && !i->isSelected())
      if (numOfSelected>0) i->setSelected(true);
    QPoint temp=mapToGlobal(p);
    emit rightClickMenu(i,temp);
  }
}*/

QListViewItem* KFileList::firstUnmarkedAboveCurrent(){
  QListViewItem *iterator=currentItem()->itemAbove();
	while( iterator && iterator->isSelected() )
	  iterator=iterator->itemAbove();
	return iterator;
}

void KFileList::getSelectedNames(QStringList* fileNames) {
  QListViewItem *iterator=firstChild();
	while (iterator != 0) {
		if ( iterator->isSelected() && iterator->text(0)!=QString("..") ) {
		  fileNames->append(iterator->text(0) );
		}
		iterator = iterator->itemBelow();
	}
	
	// if the list is empty - add current file
	if( fileNames->isEmpty() ){
	  iterator = currentItem();
	  if (iterator == 0) return; // safety - if the program just started: no current
	  if(iterator->text(0) == "..") return;
	  else {
	    fileNames->append( iterator->text(0) );
	  }
	}
}

void KFileList::scrollItem( int y ){
  if ( y==0 || !stillScrolling) return;
  QListViewItem *i = currentItem();
  if (!i) i=firstChild();
  if( y>0 ) i = i->itemBelow();
  else i = i->itemAbove();
  if (i){
    setCurrentItem(i);
    ensureItemVisible(i);
  }
  if (stillScrolling)
    QTimer::singleShot(200, this, SLOT(keepScrolling()));
}

void KFileList::keepScrolling() {
  scrollItem(scrollDir);
}

void KFileList::startScrolling( int y ) {
  if (y == 0) {
    stillScrolling = false;
    return;
  }
  scrollDir = y; stillScrolling = true;
  keepScrolling();
}

#include "kfilelist.moc"
