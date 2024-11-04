/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krmousehandler.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "krselectionmode.h"
#include "krview.h"
#include "krviewitem.h"

// QtCore
#include <QStandardPaths>
// QtWidgets
#include <QApplication>
#include <QStyle>

#include <KSharedConfig>
#include <KUrlMimeData>

#define CANCEL_TWO_CLICK_RENAME                                                                                                                                \
    {                                                                                                                                                          \
        _singleClicked = false;                                                                                                                                \
        _renameTimer.stop();                                                                                                                                   \
    }

KrMouseHandler::KrMouseHandler(KrView *view, int contextMenuShift)
    : _view(view)
    , _rightClickedItem(nullptr)
    , _contextMenuShift(contextMenuShift)
    , _singleClicked(false)
    , _dragStartPos(-1, -1)
    , _emptyContextMenu(false)
{
    KConfigGroup grpSvr(krConfig, "Look&Feel");
    // decide on single click/double click selection
    bool singleClickTmp = QApplication::style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    _singleClick = grpSvr.readEntry("Single Click Selects", _SingleClickSelects) && singleClickTmp;
    connect(&_contextMenuTimer, &QTimer::timeout, this, &KrMouseHandler::showContextMenu);
    connect(&_renameTimer, &QTimer::timeout, this, &KrMouseHandler::renameCurrentItem);
}

bool KrMouseHandler::mousePressEvent(QMouseEvent *e)
{
    _rightClickedItem = _clickedItem = nullptr;
    KrViewItem *item = _view->getKrViewItemAt(e->pos());
    KrSelectionMode *mode = KrSelectionMode::getSelectionHandler();

    if (!_view->isFocused())
        _view->op()->emitNeedFocus();
    if (e->button() == Qt::LeftButton) {
        _dragStartPos = e->pos();
        if (e->modifiers() == Qt::NoModifier) {
            if (item) {
                if (mode->leftButtonSelects()) {
                    if (mode->leftButtonPreservesSelection())
                        item->setSelected(!item->isSelected());
                    else {
                        if (item->isSelected())
                            _clickedItem = item;
                        else {
                            // clear the current selection
                            _view->unselectAll();
                            item->setSelected(true);
                        }
                    }
                }
                _view->setCurrentKrViewItem(item);
            } else {
                // empty space under items clicked
                if (mode->leftButtonSelects() && !mode->leftButtonPreservesSelection()) {
                    // clear the current selection
                    _view->unselectAll();
                }
            }
            e->accept();
            return true;
        } else if (e->modifiers() == Qt::ControlModifier) {
            if (item && (mode->shiftCtrlLeftButtonSelects() || mode->leftButtonSelects())) {
                // get current selected item names
                _selectedItemNames.clear();
                _view->getSelectedItems(&_selectedItemNames, false);

                item->setSelected(!item->isSelected());

                // select also the focused item if there are no other selected items
                KrViewItem *previousItem = _view->getCurrentKrViewItem();
                if (previousItem->name() != ".." && _selectedItemNames.empty()) {
                    previousItem->setSelected(true);
                }
            }
            if (item) {
                _view->setCurrentKrViewItem(item);
            }
            e->accept();
            return true;
        } else if (e->modifiers() == Qt::ShiftModifier) {
            if (item && (mode->shiftCtrlLeftButtonSelects() || mode->leftButtonSelects())) {
                KrViewItem *current = _view->getCurrentKrViewItem();
                if (current != nullptr)
                    _view->selectRegion(item, current, true);
            }
            if (item)
                _view->setCurrentKrViewItem(item);
            e->accept();
            return true;
        }
    }
    if (e->button() == Qt::RightButton) {
        // dragStartPos = e->pos();
        if (e->modifiers() == Qt::NoModifier) {
            if (item) {
                if (mode->rightButtonSelects()) {
                    if (mode->rightButtonPreservesSelection()) {
                        if (mode->showContextMenu() >= 0) {
                            _rightClickSelects = !item->isSelected();
                            _rightClickedItem = item;
                        }
                        item->setSelected(!item->isSelected());
                    } else {
                        if (item->isSelected()) {
                            _clickedItem = item;
                        } else {
                            // clear the current selection
                            _view->unselectAll();
                            item->setSelected(true);
                        }
                    }
                }
                _view->setCurrentKrViewItem(item);
            }
            handleContextMenu(item, e->globalPosition().toPoint());
            e->accept();
            return true;
        } else if (e->modifiers() == Qt::ControlModifier) {
            if (item && (mode->shiftCtrlRightButtonSelects() || mode->rightButtonSelects())) {
                item->setSelected(!item->isSelected());
            }
            if (item)
                _view->setCurrentKrViewItem(item);
            e->accept();
            return true;
        } else if (e->modifiers() == Qt::ShiftModifier) {
            if (item && (mode->shiftCtrlRightButtonSelects() || mode->rightButtonSelects())) {
                KrViewItem *current = _view->getCurrentKrViewItem();
                if (current != nullptr)
                    _view->selectRegion(item, current, true);
            }
            if (item)
                _view->setCurrentKrViewItem(item);
            e->accept();
            return true;
        }
    }
    if (e->button() == Qt::ForwardButton) {
        _view->op()->emitGoForward();
        return true;
    }
    if (e->button() == Qt::BackButton) {
        _view->op()->emitGoBack();
        return true;
    }
    return false;
}

bool KrMouseHandler::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        _dragStartPos = QPoint(-1, -1);

    KrViewItem *item = _view->getKrViewItemAt(e->pos());
    KrSelectionMode *mode = KrSelectionMode::getSelectionHandler();

    if (item && item == _clickedItem) {
        if (((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::NoModifier) && (mode->leftButtonSelects()) && !(mode->leftButtonPreservesSelection()))
            || ((e->button() == Qt::RightButton) && (e->modifiers() == Qt::NoModifier) && (mode->rightButtonSelects())
                && !(mode->rightButtonPreservesSelection()))) {
            // clear the current selection
            _view->unselectAll();
            item->setSelected(true);
        }
    }

    if (mode->resetSelectionItems()) {
        bool leftButtonClicked = (e->button() == Qt::LeftButton) && (e->modifiers() == Qt::NoModifier);
        bool rightButtonClicked = (e->button() == Qt::RightButton) && (e->modifiers() == Qt::NoModifier);

        bool leftTriggerCondition = !mode->leftButtonSelects() && mode->shiftCtrlLeftButtonSelects();
        bool rightTriggerCondition = !mode->rightButtonSelects() && mode->shiftCtrlRightButtonSelects();

        if ((leftButtonClicked && leftTriggerCondition) || (rightButtonClicked && rightTriggerCondition)) {
            // clear the current selection
            _view->unselectAll();
        }
    }

    if (e->button() == Qt::RightButton) {
        _rightClickedItem = nullptr;
        _contextMenuTimer.stop();
    }
    if (_singleClick && e->button() == Qt::LeftButton && e->modifiers() == Qt::NoModifier) {
        CANCEL_TWO_CLICK_RENAME;
        e->accept();
        if (item == nullptr)
            return true;
        QString tmp = item->name();
        _view->op()->emitExecuted(tmp);
        return true;
    } else if (!_singleClick && e->button() == Qt::LeftButton) {
        if (item && e->modifiers() == Qt::NoModifier) {
            if (_singleClicked && !_renameTimer.isActive() && _singleClickedItem == item) {
                KSharedConfigPtr config = KSharedConfig::openConfig();
                KConfigGroup group(krConfig, "KDE");
                int doubleClickInterval = group.readEntry("DoubleClickInterval", 400);

                int msecsFromLastClick = _singleClickTime.msecsTo(QTime::currentTime());

                if (msecsFromLastClick > doubleClickInterval && msecsFromLastClick < 5 * doubleClickInterval) {
                    _singleClicked = false;
                    _renameTimer.setSingleShot(true);
                    _renameTimer.start(doubleClickInterval);
                    return true;
                }
            }

            CANCEL_TWO_CLICK_RENAME;
            _singleClicked = true;
            _singleClickedItem = item;
            _singleClickTime = QTime::currentTime();

            return true;
        }
    }

    CANCEL_TWO_CLICK_RENAME;

    if (e->button() == Qt::MiddleButton && item != nullptr) {
        e->accept();
        if (item == nullptr)
            return true;
        _view->op()->emitMiddleButtonClicked(item);
        return true;
    }
    return false;
}

bool KrMouseHandler::mouseDoubleClickEvent(QMouseEvent *e)
{
    CANCEL_TWO_CLICK_RENAME;

    KrViewItem *item = _view->getKrViewItemAt(e->pos());
    if (_singleClick)
        return false;
    if (e->button() == Qt::LeftButton && item != nullptr) {
        e->accept();
        const QString &tmp = item->name();
        _view->op()->emitExecuted(tmp);
        return true;
    }
    return false;
}

bool KrMouseHandler::mouseMoveEvent(QMouseEvent *e)
{
    KrViewItem *item = _view->getKrViewItemAt(e->pos());
    KrSelectionMode *mode = KrSelectionMode::getSelectionHandler();

    if ((_singleClicked || _renameTimer.isActive()) && item != _singleClickedItem)
        CANCEL_TWO_CLICK_RENAME;

    if (!item)
        return false;

    const QString desc = item->description();
    _view->op()->emitItemDescription(desc);

    if (_dragStartPos != QPoint(-1, -1) && (e->buttons() & Qt::LeftButton)
        && (_dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        _view->op()->startDrag();
        return true;
    }

    if (mode->rightButtonPreservesSelection() && mode->rightButtonSelects() && mode->showContextMenu() >= 0 && e->buttons() == Qt::RightButton) {
        e->accept();
        if (item != _rightClickedItem && item && _rightClickedItem) {
            _view->selectRegion(item, _rightClickedItem, _rightClickSelects);
            _rightClickedItem = item;
            _view->setCurrentKrViewItem(item);
            _contextMenuTimer.stop();
        }
        return true;
    }
    return false;
}

bool KrMouseHandler::wheelEvent(QWheelEvent *e)
{
    if (!_view->isFocused())
        _view->op()->emitNeedFocus();

    if (e->modifiers() == Qt::ControlModifier) {
        if (e->angleDelta().y() > 0) {
            _view->zoomIn();
        } else {
            _view->zoomOut();
        }
        e->accept();
        return true;
    }

    return false;
}

void KrMouseHandler::showContextMenu()
{
    if (_rightClickedItem)
        _rightClickedItem->setSelected(true);
    if (_emptyContextMenu)
        _view->op()->emitEmptyContextMenu(_contextMenuPoint);
    else
        _view->op()->emitContextMenu(_contextMenuPoint);
}

void KrMouseHandler::handleContextMenu(KrViewItem *it, const QPoint &pos)
{
    if (!_view->isFocused())
        _view->op()->emitNeedFocus();
    int i = KrSelectionMode::getSelectionHandler()->showContextMenu();
    _contextMenuPoint = QPoint(pos.x(), pos.y() - _contextMenuShift);
    if (i < 0) {
        if (!it || it->isDummy())
            _view->op()->emitEmptyContextMenu(_contextMenuPoint);
        else {
            _view->setCurrentKrViewItem(it);
            _view->op()->emitContextMenu(_contextMenuPoint);
        }
    } else if (i > 0) {
        _emptyContextMenu = !it || it->isDummy();
        _contextMenuTimer.setSingleShot(true);
        _contextMenuTimer.start(i);
    }
}

void KrMouseHandler::otherEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Timer:
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        break;
    default:
        CANCEL_TWO_CLICK_RENAME;
    }
}

void KrMouseHandler::cancelTwoClickRename()
{
    CANCEL_TWO_CLICK_RENAME;
}

bool KrMouseHandler::dragEnterEvent(QDragEnterEvent *e)
{
    QList<QUrl> URLs = KUrlMimeData::urlsFromMimeData(e->mimeData());
    e->setAccepted(!URLs.isEmpty());
    return true;
}

bool KrMouseHandler::dragMoveEvent(QDragMoveEvent *e)
{
    QList<QUrl> URLs = KUrlMimeData::urlsFromMimeData(e->mimeData());
    e->setAccepted(!URLs.isEmpty());
    return true;
}

bool KrMouseHandler::dragLeaveEvent(QDragLeaveEvent * /*e*/)
{
    return false;
}

bool KrMouseHandler::dropEvent(QDropEvent *e)
{
    _view->op()->emitGotDrop(e);
    return true;
}
