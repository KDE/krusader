/*
 * The concept for this code was taken from the k3b project. k3b is an
 * excellent CD burning program for KDE, http://k3b.sourceforge.net
 * Anyway, original code is copyrighted by Sebastian Trueg. Thanks!
 */


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PANELTABBAR_H
#define PANELTABBAR_H

#include <qtabbar.h>
#include <qvaluelist.h>

class QMouseEvent;
class KAction;
class KActionMenu;
class ListPanel;
class KrMasterPanel;

class PanelTab: public QTab {
public:
  PanelTab(const QString & text);
  ListPanel *panel;
};

#define HIDE_ON_SINGLE_TAB true

class PanelTabBar : public QTabBar {
  Q_OBJECT
public:
  //PanelTabBar( KrMasterPanel *_master, const char* name = 0 );
  PanelTabBar( QWidget *parent );
  void insertAction( KAction* );

public slots:
// this class doesn't really use these functions
  int addPanel(ListPanel *panel);
  void updateTab(ListPanel *panel);

protected:
  void mousePressEvent( QMouseEvent* );

protected slots:
  void closeTab();
  void duplicateTab();

private:
  KrMasterPanel *_master;
  KActionMenu *_panelActionMenu;
  //QValueList<PanelTab> _panelTabs;
  bool _left;
  KAction *_closeAction, *_newTabSame, *_newTabRoot;
};

#endif
