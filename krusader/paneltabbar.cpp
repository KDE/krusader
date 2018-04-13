/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * Based on original code from Sebastian Trueg <trueg@kde.org>               *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "paneltabbar.h"

#include "defaults.h"
#include "tabactions.h"
#include "../krglobal.h"
#include "../icon.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"

// QtCore
#include <QEvent>
// QtGui
#include <QFontMetrics>
#include <QResizeEvent>
#include <QMouseEvent>
// QtWidgets
#include <QAction>
#include <QMenu>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KActionMenu>


static const int sDragEnterDelay = 500; // msec

PanelTabBar::PanelTabBar(QWidget *parent, TabActions *actions): QTabBar(parent),
    _maxTabLength(0), _tabClicked(false), _draggingTab(false), _dragTabIndex(-1)
{
    _panelActionMenu = new KActionMenu(i18n("Panel"), this);

    setAcceptDrops(true);

    insertAction(actions->actNewTab);
    insertAction(actions->actLockTab);
    insertAction(actions->actPinTab);
    insertAction(actions->actDupTab);
    insertAction(actions->actMoveTabToOtherSide);
    insertAction(actions->actCloseTab);
    insertAction(actions->actCloseInactiveTabs);
    insertAction(actions->actCloseDuplicatedTabs);

    setMovable(true); // enable drag'n'drop
    _dragTimer = new QTimer(this);
    _dragTimer->setSingleShot(true);
    _dragTimer->setInterval(sDragEnterDelay);
    connect(_dragTimer, &QTimer::timeout, this, [=]() {
        if (_dragTabIndex != -1 && _dragTabIndex != currentIndex()) {
            setCurrentIndex(_dragTabIndex);
        }
        _dragTabIndex = -1;
    });

    setShape(QTabBar::TriangularSouth);
}

void PanelTabBar::insertAction(QAction* action)
{
    _panelActionMenu->addAction(action);
}

int PanelTabBar::addPanel(ListPanel *panel, bool setCurrent, KrPanel *nextTo)
{
    int insertIndex = -1;

    if (nextTo) {
        for (int i = 0; i < count(); i++) {
            if (getPanel(i) == nextTo) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    QUrl virtualPath = panel->virtualPath();
    panel->setPinnedUrl(virtualPath);
    const QString text = squeeze(virtualPath);
    const int index = insertIndex != -1 ? insertTab(insertIndex, text) : addTab(text);

    setTabData(index, QVariant((long long) panel));

    setIcon(index, panel);

    // make sure all tabs lengths are correct
    layoutTabs();

    if (setCurrent)
        setCurrentIndex(index);

    return index;
}

ListPanel* PanelTabBar::getPanel(int tabIdx)
{
    QVariant v = tabData(tabIdx);
    if (v.isNull()) return 0;
    return (ListPanel*)v.toLongLong();
}

void PanelTabBar::changePanel(int tabIdx, ListPanel *panel)
{
    setTabData(tabIdx, QVariant((long long) panel));
}

ListPanel* PanelTabBar::removePanel(int index, ListPanel* &panelToDelete)
{
    panelToDelete = getPanel(index); // old panel to kill later
    disconnect(panelToDelete, 0, this, 0);

    removeTab(index);
    layoutTabs();

    return getPanel(currentIndex());
}

ListPanel* PanelTabBar::removeCurrentPanel(ListPanel* &panelToDelete)
{
    return removePanel(currentIndex(), panelToDelete);
}

void PanelTabBar::updateTab(ListPanel *panel)
{
    // find which is the correct tab
    for (int i = 0; i < count(); i++) {
        if ((ListPanel*)tabData(i).toLongLong() == panel) {
            setPanelTextToTab(i, panel);
            setIcon(i, panel);
            break;
        }
    }
}

void PanelTabBar::duplicateTab()
{
    int id = currentIndex();
    emit newTab(((ListPanel*)tabData(id).toLongLong())->virtualPath());
}

void PanelTabBar::setIcon(int index, ListPanel *panel)
{
    Icon tabIcon;
    if (panel->isLocked()) {
        tabIcon = Icon("lock");
    } else if (panel->isPinned()) {
        tabIcon = Icon("pin");
    }
    setTabIcon(index, tabIcon);
}

QString PanelTabBar::squeeze(const QUrl &url, int tabIndex)
{
    const QString longText = url.isEmpty() ? i18n("[invalid]") :
                                             url.isLocalFile() ? url.path() : url.toDisplayString();
    if (tabIndex >= 0)
        setTabToolTip(tabIndex, longText);

    const KConfigGroup group(krConfig, "Look&Feel");
    const bool showLongNames = group.readEntry("Fullpath Tab Names", _FullPathTabNames);

    QString text;
    if (!showLongNames) {
        const QString scheme = url.scheme().isEmpty() || url.isLocalFile() ? "" : (url.scheme() + ":");
        const QString host = url.host().isEmpty() ? "" : (url.host() + ":");
        const QString name = url.isLocalFile() && url.fileName().isEmpty() ? "/" : url.fileName();
        text = scheme + host + name;
    } else {
        text = longText;
    }

    if (text.isEmpty())
        text = i18nc("invalid URL path", "?");

    // set the real max length
    QFontMetrics fm(fontMetrics());
    _maxTabLength = (static_cast<QWidget*>(parent())->width() - (6 * fm.width("W"))) / fm.width("W");
    // each tab gets a fair share of the max tab length
    const int effectiveTabLength = _maxTabLength / (count() == 0 ? 1 : count());
    const int labelWidth = fm.width("W") * effectiveTabLength;
    const int textWidth = fm.width(text);
    if (textWidth <= labelWidth)
        return text;

    // squeeze text - start with the dots only
    QString squeezedText = "...";
    int squeezedWidth = fm.width(squeezedText);

    int letters = text.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    if (labelWidth < squeezedWidth)
        letters = 1;
    squeezedText = text.left(letters) + "..." + text.right(letters);
    squeezedWidth = fm.width(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
            letters++;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.width(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = text.left(letters) + "..." + text.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.width(squeezedText);
        } while (letters && squeezedWidth > labelWidth);
    }

    return squeezedText;
}

void PanelTabBar::resizeEvent(QResizeEvent *e)
{
    QTabBar::resizeEvent(e);

    layoutTabs();
}

void PanelTabBar::mouseMoveEvent(QMouseEvent* e)
{
    QTabBar::mouseMoveEvent(e);
    if(_tabClicked) {
        _draggingTab = true;
        emit draggingTab(e);
    }
}

void PanelTabBar::mousePressEvent(QMouseEvent* e)
{
    int clickedTab = tabAt(e->pos());

    if (-1 == clickedTab) { // clicked on nothing ...
        QTabBar::mousePressEvent(e);
        return;
    }

    _tabClicked = true;

    setCurrentIndex(clickedTab);

    ListPanel *p = getPanel(clickedTab);
    if (p)
        p->slotFocusOnMe();

    if (e->button() == Qt::RightButton) {
        // show the popup menu
        _panelActionMenu->menu()->popup(e->globalPos());
    } else {
        if (e->button() == Qt::MidButton)// close the current tab
            emit closeCurrentTab();
    }

    QTabBar::mousePressEvent(e);
}

void PanelTabBar::mouseReleaseEvent(QMouseEvent* e)
{
    QTabBar::mouseReleaseEvent(e);
    if(_draggingTab)
        emit draggingTabFinished(e);
    _draggingTab = false;
    _tabClicked = false;
}

void PanelTabBar::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
    handleDragEvent(tabAt(e->pos()));
    QTabBar::dragEnterEvent(e);
}

void PanelTabBar::dragLeaveEvent(QDragLeaveEvent *)
{
    handleDragEvent(-1);
}

void PanelTabBar::dragMoveEvent(QDragMoveEvent *e)
{
    e->ignore();
    handleDragEvent(tabAt(e->pos()));
    QTabBar::dragMoveEvent(e);
}

void PanelTabBar::handleDragEvent(int tabIndex)
{
    if (_dragTabIndex == tabIndex)
        return;

    _dragTabIndex = tabIndex;
    if (_dragTabIndex == -1) {
        _dragTimer->stop();
    } else {
        _dragTimer->start();
    }
}

void PanelTabBar::layoutTabs()
{
    for (int i = 0; i < count(); i++) {
        setPanelTextToTab(i, (ListPanel*) tabData(i).toLongLong());
    }
}

void PanelTabBar::setPanelTextToTab(int tabIndex, ListPanel *panel)
{
    // update tab text from pinnedUrl in case the tab is pinned
    if (panel->isPinned()) {
        setTabText(tabIndex, squeeze(panel->pinnedUrl(), tabIndex));
    } else {
        setTabText(tabIndex, squeeze(panel->virtualPath(), tabIndex));
    }
}
