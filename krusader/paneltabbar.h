/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * based on original code from Sebastian Trueg <trueg@kde.org>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef PANELTABBAR_H
#define PANELTABBAR_H

// QtCore
#include <QTimer>
#include <QUrl>
// QtGui
#include <QDragMoveEvent>
#include <QDragEnterEvent>
// QtWidgets
#include <QTabBar>

class QMouseEvent;
class QAction;
class KActionMenu;
class KrPanel;
class ListPanel;
class TabActions;

/**
 * This class extends QTabBar such that right-clicking on a tab pops-up a menu
 * containing relevant actions for the tab. It also emits signals (caught by PanelManager)
 * to create a new tab, close the current tab and change a panel when a tab was clicked
 */
class PanelTabBar : public QTabBar
{
    Q_OBJECT
public:
    PanelTabBar(QWidget *parent, TabActions *actions);

public slots:
    /**
     * called by PanelManager with an already created panel, and creates the corresponding tab
     */
    int addPanel(ListPanel *panel, bool setCurrent = true, KrPanel *nextTo = 0);

    ListPanel* getPanel(int tabIdx);
    void changePanel(int tabIdx, ListPanel *panel);
    void layoutTabs();

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
    ListPanel* removePanel(int index, ListPanel* &panelToDelete);

signals:
    /**
     * emitted when the user right-clicks and selected "close"
     */
    void closeCurrentTab();
    /**
     * emitted when the user right-clicks and selects an action that creates a new tab
     */
    void newTab(const QUrl &path);

    void draggingTab(QMouseEvent*);
    void draggingTabFinished(QMouseEvent*);

protected:
    virtual void mouseMoveEvent(QMouseEvent*e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent*) Q_DECL_OVERRIDE;
    void insertAction(QAction*);
    QString squeeze(const QUrl &url, int tabIndex = -1);
    virtual void dragEnterEvent(QDragEnterEvent *) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QDragLeaveEvent *) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

protected slots:
    void duplicateTab();

private:
    void setIcon(int index, ListPanel *panel);
    void handleDragEvent(int tabIndex);
    KActionMenu *_panelActionMenu;
    bool _left;
    int _maxTabLength;
    bool _tabClicked, _draggingTab;

    QTimer *_dragTimer;
    int _dragTabIndex;
};

#endif
