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

#include "krviewitemdelegate.h"

#include "krviewproperties.h"
#include "../krglobal.h"
#include "../listpanel.h"

// QtGui
#include <QKeyEvent>
#include <QPainter>
// QtWidgets
#include <QDialog>
#include <QApplication>
#include <QLineEdit>

#include <KConfigCore/KSharedConfig>

KrViewItemDelegate::KrViewItemDelegate(QObject *parent) :
        QItemDelegate(parent), _currentlyEdited(-1), _dontDraw(false) {}

void KrViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;
    _dontDraw = (_currentlyEdited == index.row()) && (index.column() == KrViewProperties::Ext);
    QItemDelegate::paint(painter, opt, index);
}

void KrViewItemDelegate::drawDisplay(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text) const
{
    if (!_dontDraw)
        QItemDelegate::drawDisplay(painter, option, rect, text);
}

QWidget * KrViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const
{
    _currentlyEdited = index.row();
    return QItemDelegate::createEditor(parent, sovi, index);
}

void KrViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData(editor, index);
    QLineEdit *lineEdit = qobject_cast<QLineEdit *> (editor);
    if (lineEdit) {
        KConfigGroup gl(krConfig, "Look&Feel");
        QFont font = index.data(Qt::FontRole).value<QFont>();
        lineEdit->setFont(font);
        if (gl.readEntry("Rename Selects Extension", true))
            lineEdit->selectAll();
        else {
            QString nameWithoutExt = index.data(Qt::UserRole).toString();
            lineEdit->deselect();
            lineEdit->setSelection(0, nameWithoutExt.length());
        }
    }
}

QSize KrViewItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    if (size.isEmpty()) {
        // prevent items without text from bloating the view vertically
        return QSize(0, 0);
    }
    return size;
}

bool KrViewItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget *editor = qobject_cast<QWidget*>(object);
    if (!editor)
        return false;
    if (event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            _currentlyEdited = -1;
            emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (QLineEdit *e = qobject_cast<QLineEdit*>(editor)) {
                if (!e->hasAcceptableInput())
                    return true;
                event->accept();
                emit commitData(editor);
                emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
                _currentlyEdited = -1;
                return true;
            }
            return false;
        case Qt::Key_Escape:
            event->accept();
            // don't commit data
            _currentlyEdited = -1;
            emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
            break;
        default:
            return false;
        }

        if (editor->parentWidget())
            editor->parentWidget()->setFocus();
        return true;
    } else if (event->type() == QEvent::FocusOut) {
        if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
            QWidget *w = QApplication::focusWidget();
            while (w) { // don't worry about focus changes internally in the editor
                if (w == editor)
                    return false;
                w = w->parentWidget();
            }
            // Opening a modal dialog will start a new eventloop
            // that will process the deleteLater event.
            if (QApplication::activeModalWidget()
                    && !QApplication::activeModalWidget()->isAncestorOf(editor)
                    && qobject_cast<QDialog*>(QApplication::activeModalWidget()))
                return false;
            _currentlyEdited = -1;
            // manually set focus back to panel after rename canceled by focusing another window
            ACTIVE_PANEL->gui->slotFocusOnMe();
            emit closeEditor(editor, RevertModelCache);
        }
    } else if (event->type() == QEvent::ShortcutOverride) {
        const QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape ||
            (ke->key() == Qt::Key_Backspace && ke->modifiers() == Qt::ControlModifier)) {
            event->accept();
            return true;
        }
    }
    return false;
}
