/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    Based on original code from Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANELTABBAR_H
#define PANELTABBAR_H

// QtCore
#include <QTimer>
#include <QUrl>
// QtGui
#include <QDragEnterEvent>
#include <QDragMoveEvent>
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
    int addPanel(ListPanel *panel, bool setCurrent = true, int insertIndex = -1);

    ListPanel *getPanel(int tabIdx);
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
    ListPanel *removeCurrentPanel(ListPanel *&panelToDelete); // returns the panel focused after removing the current
    ListPanel *removePanel(int index, ListPanel *&panelToDelete);

signals:
    /**
     * emitted when the user right-clicks and selected "close"
     */
    void closeCurrentTab();
    /**
     * emitted when the user press Ctrl+LMB
     */
    void duplicateCurrentTab();
    /**
     * emitted when the user right-clicks and selects an action that creates a new tab
     */
    void newTab(const QUrl &path);

    void draggingTab(QMouseEvent *);
    void draggingTabFinished(QMouseEvent *);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void insertAction(QAction *);
    QString squeeze(const QUrl &url, int tabIndex = -1);
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void resizeEvent(QResizeEvent *e) override;

protected slots:
    void duplicateTab();

private:
    void setIcon(int index, ListPanel *panel);
    void handleDragEvent(int tabIndex);
    void setPanelTextToTab(int tabIndex, ListPanel *panel);

    KActionMenu *_panelActionMenu;
    bool _left;
    int _maxTabLength;
    bool _tabClicked, _tabDoubleClicked, _draggingTab, _doubleClickClose;

    QTimer *_dragTimer;
    int _dragTabIndex;
};

#endif
