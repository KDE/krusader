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

#include <kurl.h>
//Added by qt3to4:
//#include <QResizeEvent>
//#include <QMouseEvent>
//#include <QDragMoveEvent>
//#include <QDragEnterEvent>
#include <QTabBar>
#include <QVariant>

class QMouseEvent;
class KAction;
class KActionMenu;
class ListPanel;


/**
 * This class extends QTabBar such that right-clicking on a tab pops-up a menu
 * containing relevant actions for the tab. It also emits signals (caught by PanelManager)
 * to create a new tab, close the current tab and change a panel when a tab was clicked
 */
class PanelTabBar : public QTabBar {
  Q_OBJECT
public:
  PanelTabBar( QWidget *parent );

public slots:
  /**
   * called by PanelManager with an already created panel, and creates the corrosponding tab
   */
  int addPanel(ListPanel *panel, bool setCurrent = true );

  ListPanel* getPanel(int tabIdx);
  void changePanel(int tabIdx, ListPanel *panel);

  /**
   * when the user changes the current path in a panel, this method updates the tab accordingly
   */
  void updateTab(ListPanel *panel);
  /**
   * actually removes the current tab WITHOUT actually deleting the panel.
   * returns a pointer to the panel which is going to be displayed next.
   * panelToDelete returns a reference to the pointer of the soon-to-die panel, to
   * be used by PanelManager.
   */
  ListPanel* removeCurrentPanel(ListPanel* &panelToDelete); // returns the panel focused after removing the current

signals:
  /**
   * This signal is emitted when the user clicked on a tab. PanelManager should change panels accordingly
   */
  void changePanel(ListPanel *p);
  /**
   * emitted when the user right-clicks and selected "close"
   */
  void closeCurrentTab();
  /**
   * emitted when the user right-clicks and selects an action that creates a new tab
   */
  void newTab(const KUrl& path);

protected:
  void mousePressEvent( QMouseEvent* );
  void insertAction( KAction* );
  QString squeeze(QString text, int index=-1);
#if 0 // TODO: fix this
  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dragMoveEvent(QDragMoveEvent *);
#endif
  virtual void resizeEvent ( QResizeEvent *e );

protected slots:
  void closeTab();
  void duplicateTab();

private:
  KActionMenu *_panelActionMenu;
  bool _left;
  int _maxTabLength;
};

#endif
