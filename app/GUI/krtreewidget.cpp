/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krtreewidget.h"
#include "../compat.h"
#include "krstyleproxy.h"

// QtGui
#include <QContextMenuEvent>
// QtWidgets
#include <QApplication>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#include <QToolTip>

KrTreeWidget::KrTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , _inResize(false)
{
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);

    _stretchingColumn = -1;

    auto *krstyle = new KrStyleProxy();
    krstyle->setParent(this);
    setStyle(krstyle);
}

bool KrTreeWidget::event(QEvent *event)
{
    switch (event->type()) {
        // HACK: QT 4 Context menu key isn't handled properly
    case QEvent::ContextMenu: {
        auto *ce = dynamic_cast<QContextMenuEvent *>(event);

        if (ce->reason() == QContextMenuEvent::Mouse) {
            QPoint pos = viewport()->mapFromGlobal(ce->globalPos());

            QTreeWidgetItem *item = itemAt(pos);
            int column = columnAt(pos.x());

            emit itemRightClicked(item, ce->globalPos(), column);
            return true;
        } else {
            if (currentItem()) {
                QRect r = visualItemRect(currentItem());
                QPoint p = viewport()->mapToGlobal(QPoint(r.x() + 5, r.y() + 5));

                emit itemRightClicked(currentItem(), p, currentColumn());
                return true;
            }
        }
    } break;
    case QEvent::KeyPress: {
        // HACK: QT 4 Ctrl+A bug fix: Ctrl+A doesn't work if QTreeWidget contains parent / child items
        //       Insert doesn't change the selections for multi selection modes
        auto *ke = dynamic_cast<QKeyEvent *>(event);
        switch (ke->key()) {
        case Qt::Key_Insert: {
            if (ke->modifiers() != 0)
                break;

            QAbstractItemView::SelectionMode mode = selectionMode();

            if (mode != QAbstractItemView::ContiguousSelection && mode != QAbstractItemView::ExtendedSelection && mode != QAbstractItemView::MultiSelection)
                break;

            ke->accept();

            if (currentItem() == nullptr)
                return true;

            currentItem()->setSelected(!currentItem()->isSelected());
            return true;
        }
        case Qt::Key_A:
            if (ke->modifiers() == Qt::ControlModifier) {
                QAbstractItemView::SelectionMode mode = selectionMode();

                if (mode == QAbstractItemView::ContiguousSelection || mode == QAbstractItemView::ExtendedSelection
                    || mode == QAbstractItemView::MultiSelection) {
                    selectAll();
                    ke->accept();
                    return true;
                }
            }
            break;
        default:
            break;
        }
    } break;
    case QEvent::Resize: {
        auto *re = dynamic_cast<QResizeEvent *>(event);
        if (!_inResize && re->oldSize() != re->size()) {
            if (_stretchingColumn != -1 && columnCount()) {
                QList<int> columnsSizes;
                int oldSize = 0;

                for (int i = 0; i != header()->count(); i++) {
                    columnsSizes.append(header()->sectionSize(i));
                    oldSize += header()->sectionSize(i);
                }

                bool res = QTreeWidget::event(event);

                int newSize = viewport()->width();
                int delta = newSize - oldSize;

                if (delta) {
                    _inResize = true;

                    for (int i = 0; i != header()->count(); i++) {
                        if (i == _stretchingColumn) {
                            int newNs = columnsSizes[i] + delta;
                            if (newNs < 8)
                                newNs = 8;
                            header()->resizeSection(i, newNs);
                        } else if (header()->sectionSize(i) != columnsSizes[i]) {
                            header()->resizeSection(i, columnsSizes[i]);
                        }
                    }
                    _inResize = false;
                }
                return res;
            }
        }
        break;
    }
    case QEvent::ToolTip: {
        auto *he = dynamic_cast<QHelpEvent *>(event);

        if (viewport()) {
            QPoint pos = viewport()->mapFromGlobal(he->globalPos());

            QTreeWidgetItem *item = itemAt(pos);

            int column = columnAt(pos.x());

            if (item) {
                if (!item->toolTip(column).isEmpty())
                    break;

                QString tip = item->text(column);

                int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                int requiredWidth = QFontMetrics(font()).horizontalAdvance(tip) + 2 * textMargin;

                if (column == 0 && indentation()) {
                    int level = 0;

                    QTreeWidgetItem *parent = item;

                    while ((parent = parent->parent()))
                        level++;

                    if (rootIsDecorated())
                        level++;

                    requiredWidth += level * indentation();
                }

                QIcon icon = item->icon(column);
                if (!icon.isNull()) {
                    QStyleOptionViewItem opts;
                    initViewItemOption(&opts);
                    QSize iconSize = icon.actualSize(opts.decorationSize);
                    requiredWidth += iconSize.width();

                    int pixmapMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, this) + 1;
                    requiredWidth += 2 * pixmapMargin;
                }

                if (!tip.isEmpty() && (columnWidth(column) < requiredWidth))
                    QToolTip::showText(he->globalPos(), tip, this);
                return true;
            }
        }
    } break;
    default:
        break;
    }
    return QTreeWidget::event(event);
}
