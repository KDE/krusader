/***************************************************************************
                          paneltabbar.cpp  -  description
                             -------------------
    begin                : Sun Jun 2 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "paneltabbar.h"
#include "Panel/listpanel.h"
#include "krusaderview.h"
#include "krslots.h"
#include <kaction.h>
#include <klocale.h>
#include <kshortcut.h>
#include <qevent.h>
#include <qwidgetstack.h>
#include <qfontmetrics.h>

PanelTabBar::PanelTabBar(QWidget *parent): QTabBar(parent) {
  _panelActionMenu = new KActionMenu( i18n("Panel"), this );

  _closeAction = new KAction(i18n("Close tab"), KShortcut::null(), this,
                      SLOT(closeTab()), krApp->actionCollection(), "close_tab");
  _newTabSame = new KAction(i18n("Duplicate tab"), KShortcut::null(), this,
                      SLOT(duplicateTab()), krApp->actionCollection(), "dup_tab");

  insertAction(_newTabSame);
  insertAction(_closeAction);
  _closeAction->setEnabled(false); //can't close a single tab

  setShape(QTabBar::RoundedBelow);
  //addTab(new QTab("")); // ugly hack - don't remove

  if (HIDE_ON_SINGLE_TAB) hide();
}

void PanelTabBar::mousePressEvent( QMouseEvent* e ) {
  QTab* clickedTab = selectTab( e->pos() );
  if( !clickedTab ) { // clicked on nothing ...
    QTabBar::mousePressEvent(e);
    return;
  }
  // else implied
  setCurrentTab( clickedTab );

  if ( e->button() == Qt::RightButton ) {
    // show the popup menu
    _panelActionMenu->popup( e->globalPos() );
  }

  if ( e->button() == Qt::LeftButton ) { // we need to change tabs
    // first, find the correct panel to load
    int id = currentTab();
    ListPanel *listpanel = 0L;
    //QValueList<PanelTab>::Iterator it;
    //for (it = _panelTabs.begin(); it != _panelTabs.end(); ++it) {
    //  if ((*it).id == id) {
    //    listpanel = (*it).panel;
    //    break;
    //  }
    //}
    // now, check if we're on the left or right
    /*bool needSetFocus = ((_master->_self) != (_master->_active));
    _master->_self = listpanel;
    _master->_stack->raiseWidget(_master->_self);
    _master->_self->setOther(_master->_other);
    _master->_other->setOther(_master->_self);
    _master->_active = _master->_self;

    kapp->processEvents();
    if (needSetFocus) _master->_active->slotFocusOnMe();*/
  }
//  QTabBar::mousePressEvent(e);
}

void PanelTabBar::insertAction( KAction* action ) {
  _panelActionMenu->insert( action );
}

int PanelTabBar::addPanel(ListPanel *panel) {
  int h = QFontMetrics(font()).height()+2;
  int newId = addTab(new PanelTab(panel->virtualPath));
  PanelTab *ptab= dynamic_cast<PanelTab*>(tab(newId));
  ptab->rect().setHeight(3);

  ptab->panel = panel;

  layoutTabs();
  setCurrentTab(newId);

  // enable close-tab action
  if (count()>1) {
    _closeAction->setEnabled(true);
    show();
  }

  connect(ptab->panel, SIGNAL(pathChanged(ListPanel*)), this, SLOT(updateTab(ListPanel*)));

  return newId;
}

void PanelTabBar::updateTab(ListPanel *panel) {
  // find which is the correct tab
  for (int i=0; i<count(); i++) {
    if (dynamic_cast<PanelTab*>(tab(i))->panel == panel) {
      tab(i)->setText(panel->virtualPath);
      break;
    }
  }

  //QValueList<PanelTab>::Iterator it;
  //for (it = _panelTabs.begin(); it!=_panelTabs.end(); ++it) {
  //  if ( (*it).panel == panel ) {
  //    tab((*it).id)->setText(panel->virtualPath);
  //    break;
  //  }
  //}

}

void PanelTabBar::duplicateTab() {
/*  int id = currentTab();
  ListPanel *listpanel;
  QValueList<PanelTab>::Iterator it;
  for (it = _panelTabs.begin(); it != _panelTabs.end(); ++it) {
    if ((*it).id == id) {
      listpanel = (*it).panel;
      break;
    }
  }*/

  //SLOTS->newTab(listpanel->virtualPath);
}

void PanelTabBar::closeTab() {
  // find the panel to kill
/*  int id = currentTab();
  ListPanel *listpanel = 0L;
  QValueList<PanelTab>::Iterator it;
  for (it = _panelTabs.begin(); it != _panelTabs.end(); ++it) {
    if ((*it).id == id) {
      listpanel = (*it).panel;
      _panelTabs.remove(it); // remove from the list of tabs
      break;
    }
  }
  // now, remove the panel and the tab
  /*bool needSetFocus = ((_master->_self) != (_master->_active));
  removeTab(tab(id));
  id = currentTab(); // get the NEW current tab
  _master->removePanel(listpanel, id);

  kapp->processEvents();
  if (needSetFocus) {
    _master->_active = _master->_self;
    _master->_active->slotFocusOnMe();
  }
  if (count()==1) {
    _closeAction->setEnabled(false);
    if (HIDE_ON_SINGLE_TAB) hide();
  }*/
}

// -----------------------------> PanelTab <----------------------------

PanelTab::PanelTab(const QString & text): QTab(text) {}

#include "paneltabbar.moc"
