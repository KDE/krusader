/*****************************************************************************
 * Copyright (C) 2008 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2008-2018 Krusader Krew [https://krusader.org]              *
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

#include "krtreewidget.h"
#include "krstyleproxy.h"

// QtGui
#include <QContextMenuEvent>
// QtWidgets
#include <QStyleOptionViewItem>
#include <QToolTip>
#include <QHeaderView>
#include <QApplication>

KrTreeWidget::KrTreeWidget(QWidget * parent) : QTreeWidget(parent), _inResize(false)
{
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);

    _stretchingColumn = -1;

    KrStyleProxy *krstyle = new KrStyleProxy();
    krstyle->setParent(this);
    setStyle(krstyle);
}

bool KrTreeWidget::event(QEvent * event)
{
    switch (event->type()) {
        // HACK: QT 4 Context menu key isn't handled properly
    case QEvent::ContextMenu: {
        QContextMenuEvent* ce = (QContextMenuEvent*) event;

        if (ce->reason() == QContextMenuEvent::Mouse) {
            QPoint pos = viewport()->mapFromGlobal(ce->globalPos());

            QTreeWidgetItem * item = itemAt(pos);
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
    }
    break;
    case QEvent::KeyPress: {
        // HACK: QT 4 Ctrl+A bug fix: Ctrl+A doesn't work if QTreeWidget contains parent / child items
        //       Insert doesn't change the selections for multi selection modes
        QKeyEvent* ke = (QKeyEvent*) event;
        switch (ke->key()) {
        case Qt::Key_Insert: {
            if (ke->modifiers() != 0)
                break;

            QAbstractItemView::SelectionMode mode = selectionMode();

            if (mode != QAbstractItemView::ContiguousSelection && mode != QAbstractItemView::ExtendedSelection &&
                    mode != QAbstractItemView::MultiSelection)
                break;

            ke->accept();

            if (currentItem() == 0)
                return true;

            currentItem()->setSelected(!currentItem()->isSelected());
            return true;
        }
        case Qt::Key_A:
            if (ke->modifiers() == Qt::ControlModifier) {
                QAbstractItemView::SelectionMode mode = selectionMode();

                if (mode == QAbstractItemView::ContiguousSelection || mode == QAbstractItemView::ExtendedSelection ||
                        mode == QAbstractItemView::MultiSelection) {
                    selectAll();
                    ke->accept();
                    return true;
                }
            }
            break;
        default:
            break;
        }
    }
    break;
    case QEvent::Resize: {
        QResizeEvent * re = (QResizeEvent *)event;
        if (!_inResize && re->oldSize() != re->size()) {
            if (_stretchingColumn != -1 && columnCount()) {
                QList< int > columnsSizes;
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
                            int newNs = columnsSizes[ i ] + delta;
                            if (newNs < 8)
                                newNs = 8;
                            header()->resizeSection(i, newNs);
                        } else if (header()->sectionSize(i) != columnsSizes[ i ]) {
                            header()->resizeSection(i, columnsSizes[ i ]);
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
        QHelpEvent *he = static_cast<QHelpEvent*>(event);

        if (viewport()) {
            QPoint pos = viewport()->mapFromGlobal(he->globalPos());

            QTreeWidgetItem * item = itemAt(pos);

            int column = columnAt(pos.x());

            if (item) {
                if (!item->toolTip(column).isEmpty())
                    break;

                QString tip = item->text(column);

                int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                int requiredWidth = QFontMetrics(font()).width(tip) + 2 * textMargin;

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
                    QStyleOptionViewItem opts = viewOptions();
                    QSize iconSize = icon.actualSize(opts.decorationSize);
                    requiredWidth += iconSize.width();

                    int pixmapMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, this) + 1;
                    requiredWidth += 2 * pixmapMargin;
                }

                if (!tip.isEmpty() && (columnWidth(column) < requiredWidth))
                    QToolTip::showText(he->globalPos(), tip, this);
                return true;
            }
        }
    }
    break;
    default:
        break;
    }
    return QTreeWidget::event(event);
}
