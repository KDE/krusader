/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew <https://krusader.org>

    Based on original code from Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "paneltabbar.h"

#include "defaults.h"
#include "tabactions.h"
#include "../krglobal.h"
#include "../icon.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "compat.h"

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
    const KConfigGroup cfg(krConfig, "Look&Feel");
    const bool expandingTabs = cfg.readEntry("Expanding Tabs", true);
    const bool showCloseButtons = cfg.readEntry("Show Close Tab Buttons", true);
    _doubleClickClose  = cfg.readEntry("Close Tab By Double Click", false);

    _panelActionMenu = new KActionMenu(i18n("Panel"), this);

    setAcceptDrops(true);
    setExpanding(expandingTabs);
    setTabsClosable(showCloseButtons);

    insertAction(actions->actNewTabFromUI);
    insertAction(actions->actLockTab);
    insertAction(actions->actPinTab);
    insertAction(actions->actDupTab);
    insertAction(actions->actMoveTabToOtherSide);
    insertAction(actions->actMoveTabToLeft);
    insertAction(actions->actMoveTabToRight);
    insertAction(actions->actCloseTab);
    insertAction(actions->actUndoCloseTab);
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

int PanelTabBar::addPanel(ListPanel *panel, bool setCurrent, int insertIndex)
{
    // If the position where to place the new tab is not specified,
    // take the settings into account
    if (insertIndex == -1) {
        insertIndex = KConfigGroup(krConfig, "Look&Feel").readEntry("Insert Tabs After Current", false) ?
                    currentIndex() + 1 : count();
    }

    QUrl virtualPath = panel->virtualPath();
    panel->setPinnedUrl(virtualPath);
    const QString text = squeeze(virtualPath);
    // In the help about `insertTab()` it's written that it inserts a new tab at
    // position `index`. If `index` is out of range, the new tab is appened. Returns
    // the new tab's index
    insertIndex = insertTab(insertIndex, text);

    setTabData(insertIndex, QVariant(reinterpret_cast<long long>(panel)));

    setIcon(insertIndex, panel);

    // make sure all tabs lengths are correct
    layoutTabs();

    if (setCurrent)
        setCurrentIndex(insertIndex);

    return insertIndex;
}

ListPanel* PanelTabBar::getPanel(int tabIdx)
{
    QVariant v = tabData(tabIdx);
    if (v.isNull()) return nullptr;
    return (ListPanel*)v.toLongLong();
}

void PanelTabBar::changePanel(int tabIdx, ListPanel *panel)
{
    setTabData(tabIdx, QVariant((long long) panel));
}

ListPanel* PanelTabBar::removePanel(int index, ListPanel* &panelToDelete)
{
    panelToDelete = getPanel(index); // old panel to kill later
    disconnect(panelToDelete, nullptr, this, nullptr);

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
        const QString scheme = url.scheme().isEmpty() || url.isLocalFile() ? "" : (url.scheme() + ':');
        const QString host = url.host().isEmpty() ? "" : (url.host() + ':');
        const QString name = url.isLocalFile() && url.fileName().isEmpty() ? "/" : url.fileName();
        text = scheme + host + name;
    } else {
        text = longText;
    }

    if (text.isEmpty())
        text = i18nc("invalid URL path", "?");

    // set the real max length
    QFontMetrics fm(fontMetrics());
    _maxTabLength = (dynamic_cast<QWidget*>(parent())->width() - (6 * fm.QFONTMETRICS_WIDTH("W"))) / fm.QFONTMETRICS_WIDTH("W");
    // each tab gets a fair share of the max tab length
    const int effectiveTabLength = _maxTabLength / (count() == 0 ? 1 : count());
    const int labelWidth = fm.QFONTMETRICS_WIDTH("W") * effectiveTabLength;
    const int textWidth = fm.QFONTMETRICS_WIDTH(text);
    if (textWidth <= labelWidth)
        return text;

    // squeeze text - start with the dots only
    QString squeezedText = "...";
    int squeezedWidth = fm.QFONTMETRICS_WIDTH(squeezedText);

    int letters = text.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    if (labelWidth < squeezedWidth)
        letters = 1;
    squeezedText = text.left(letters) + "..." + text.right(letters);
    squeezedWidth = fm.QFONTMETRICS_WIDTH(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
            letters++;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.QFONTMETRICS_WIDTH(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = text.left(letters) + "..." + text.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.QFONTMETRICS_WIDTH(squeezedText);
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

void PanelTabBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!_doubleClickClose)
    {
        QTabBar::mouseDoubleClickEvent(e);
        return;
    }

    int clickedTab = tabAt(e->pos());

    if (-1 == clickedTab) { // clicked on nothing ...
        QTabBar::mouseDoubleClickEvent(e);
        return;
    }

    _tabClicked = true;

    setCurrentIndex(clickedTab);

    if (e->button() == Qt::LeftButton) // close the current tab
        emit closeCurrentTab();

    QTabBar::mouseDoubleClickEvent(e);
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
