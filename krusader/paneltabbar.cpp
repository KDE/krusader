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
#include <kdebug.h>

PanelTabBar::PanelTabBar(QWidget *parent): QTabBar(parent) {
  _panelActionMenu = new KActionMenu( i18n("Panel"), this );

  _closeAction = new KAction(i18n("Close tab"), KShortcut::null(), this,
                      SLOT(closeTab()), krApp->actionCollection(), "close_tab");
  _newTabSame = new KAction(i18n("Duplicate tab"), KShortcut::null(), this,
                      SLOT(duplicateTab()), krApp->actionCollection(), "dup_tab");

  insertAction(_newTabSame);
  insertAction(_closeAction);
  _closeAction->setEnabled(false); //can't close a single tab

  setShape(QTabBar::TriangularBelow);

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
  emit changePanel(dynamic_cast<PanelTab*>(clickedTab)->panel);

  if ( e->button() == Qt::RightButton ) {
    // show the popup menu
    _panelActionMenu->popup( e->globalPos() );
  }

  if ( e->button() == Qt::LeftButton ) { // we need to change tabs
    // first, find the correct panel to load
    int id = currentTab();
    ListPanel *listpanel = dynamic_cast<PanelTab*>(tab(id))->panel;
    emit changePanel(listpanel);
  }
  QTabBar::mousePressEvent(e);
}

void PanelTabBar::insertAction( KAction* action ) {
  _panelActionMenu->insert( action );
}

int PanelTabBar::addPanel(ListPanel *panel) {
  int newId = addTab(new PanelTab(panel->virtualPath, panel));

  layoutTabs();
  setCurrentTab(newId);

  // enable close-tab action
  if (count()>1) {
    _closeAction->setEnabled(true);
    show(); // todo-do we need this?
  }

  connect(dynamic_cast<PanelTab*>(tab(newId))->panel, SIGNAL(pathChanged(ListPanel*)),
          this, SLOT(updateTab(ListPanel*)));

  return newId;
}

ListPanel* PanelTabBar::removeCurrentPanel(ListPanel* &panelToDelete) {
  int id = currentTab();
  ListPanel *oldp = dynamic_cast<PanelTab*>(tab(id))->panel; // old panel to kill later
  disconnect(dynamic_cast<PanelTab*>(tab(id))->panel);
  removeTab(tab(id));
  layoutTabs();

  // setup current one
  id = currentTab();
  ListPanel *p = dynamic_cast<PanelTab*>(tab(id))->panel;
  // disable close action?
  if (count()==1) {
    _closeAction->setEnabled(false);
  }

  panelToDelete = oldp;
  return p;
}

void PanelTabBar::updateTab(ListPanel *panel) {
  // find which is the correct tab
  for (int i=0; i<count(); i++) {
    if (dynamic_cast<PanelTab*>(tabAt(i))->panel == panel) {
      tabAt(i)->setText(panel->virtualPath);
      break;
    }
  }
}

void PanelTabBar::duplicateTab() {
  int id = currentTab();
  emit newTab(dynamic_cast<PanelTab*>(tab(id))->panel->virtualPath);
}

void PanelTabBar::closeTab() {
  emit closeCurrentTab();
}

// -----------------------------> PanelTab <----------------------------

PanelTab::PanelTab(const QString & text): QTab(text) {}
PanelTab::PanelTab(const QString & text, ListPanel *p): QTab(text), panel(p) {}

#include "paneltabbar.moc"
