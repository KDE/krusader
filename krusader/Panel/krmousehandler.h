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

#ifndef KRMOUSEHANDLER_H
#define KRMOUSEHANDLER_H

#include <QPoint>
#include <QTimer>
#include <QTime>

class QMouseEvent;
class QWheelEvent;
class KrView;
class KrViewItem;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class KrMouseHandler : public QObject
{
    Q_OBJECT

public:
    KrMouseHandler(KrView * view, int contextMenuShift);

    bool mousePressEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool mouseDoubleClickEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *);
    bool dragEnterEvent(QDragEnterEvent *e);
    bool dragMoveEvent(QDragMoveEvent *e);
    bool dragLeaveEvent(QDragLeaveEvent *e);
    bool dropEvent(QDropEvent *e);
    void handleContextMenu(KrViewItem * it, const QPoint & pos);
    void otherEvent(QEvent * e);
    void cancelTwoClickRename();

public slots:
    void showContextMenu();

signals:
    void renameCurrentItem();

protected:
    KrView     * _view;
    KrViewItem * _rightClickedItem;
    KrViewItem * _clickedItem;
    bool         _rightClickSelects;
    bool         _singleClick;
    QPoint       _contextMenuPoint;
    QTimer       _contextMenuTimer;
    int          _contextMenuShift;
    bool         _singleClicked;
    KrViewItem * _singleClickedItem;
    QTime        _singleClickTime;
    QTimer       _renameTimer;
    QPoint       _dragStartPos;
    bool         _emptyContextMenu;
};

#endif
