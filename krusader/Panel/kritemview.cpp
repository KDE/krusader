/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <cskarai@freemail.hu>                      *
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

#include "kritemview.h"
#include "krmousehandler.h"
#include "krvfsmodel.h"
#include "krinterviewitemdelegate.h"
#include "krinterviewitem.h"
#include "../GUI/krstyleproxy.h"
#include "../defaults.h"

#include <QWidget>
#include <QAbstractItemView>
#include <kconfig.h>


KrItemView::KrItemView(QWidget *parent, KrViewInstance &instance, KConfig *cfg) :
    QAbstractItemView(parent),
    KrInterView(instance, cfg, this)
{
    setWidget(this);
    setModel(_model);

    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionModel(new DummySelectionModel(_model, this));

    KConfigGroup grpSvr(_config, "Look&Feel");
    _viewFont = grpSvr.readEntry("Filelist Font", _FilelistFont);

    setStyle(new KrStyleProxy());
    setItemDelegate(new KrInterViewItemDelegate());
    setMouseTracking(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(_mouseHandler, SIGNAL(renameCurrentItem()), SLOT(renameCurrentItem()));
}

KrItemView::~KrItemView()
{
    setModel(0);
    delete _operator;
    _operator = 0;
}

void KrItemView::setFileIconSize(int size)
{
    KrView::setFileIconSize(size);
    setIconSize(QSize(fileIconSize(), fileIconSize()));
    updateGeometries();
}

void KrItemView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (_model->ready()) {
        KrViewItem * item = getKrInterViewItem(currentIndex());
        op()->emitCurrentChanged(item);
    }
    QAbstractItemView::currentChanged(current, previous);
}

void KrItemView::renameCurrentItem()
{
    QModelIndex cIndex = currentIndex();
    QModelIndex nameIndex = _model->index(cIndex.row(), KrViewProperties::Name);
    edit(nameIndex);
    updateEditorData();
    update(nameIndex);
}

bool KrItemView::event(QEvent * e)
{
    _mouseHandler->otherEvent(e);
    return QAbstractItemView::event(e);
}

void KrItemView::mousePressEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mousePressEvent(ev))
        QAbstractItemView::mousePressEvent(ev);
}

void KrItemView::mouseReleaseEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseReleaseEvent(ev))
        QAbstractItemView::mouseReleaseEvent(ev);
}

void KrItemView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseDoubleClickEvent(ev))
        QAbstractItemView::mouseDoubleClickEvent(ev);
}

void KrItemView::mouseMoveEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseMoveEvent(ev))
        QAbstractItemView::mouseMoveEvent(ev);
}

void KrItemView::wheelEvent(QWheelEvent *ev)
{
    if (!_mouseHandler->wheelEvent(ev))
        QAbstractItemView::wheelEvent(ev);
}

void KrItemView::dragEnterEvent(QDragEnterEvent *ev)
{
    if (!_mouseHandler->dragEnterEvent(ev))
        QAbstractItemView::dragEnterEvent(ev);
}

void KrItemView::dragMoveEvent(QDragMoveEvent *ev)
{
    QAbstractItemView::dragMoveEvent(ev);
    _mouseHandler->dragMoveEvent(ev);
}

void KrItemView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    if (!_mouseHandler->dragLeaveEvent(ev))
        QAbstractItemView::dragLeaveEvent(ev);
}

void KrItemView::dropEvent(QDropEvent *ev)
{
    if (!_mouseHandler->dropEvent(ev))
        QAbstractItemView::dropEvent(ev);
}

bool KrItemView::viewportEvent(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(he->pos());

        if (index.isValid()) {
            int width = visualRect(index).width();
            int textWidth = elementWidth(index);

            if (textWidth <= width) {
                event->accept();
                return true;
            }
        }
    }
    return QAbstractItemView::viewportEvent(event);
}

QRect KrItemView::mapToViewport(const QRect &rect) const
{
    if (!rect.isValid())
        return rect;

    QRect result = rect;

    int dx = -horizontalOffset();
    int dy = -verticalOffset();
    result.adjust(dx, dy, dx, dy);
    return result;
}
