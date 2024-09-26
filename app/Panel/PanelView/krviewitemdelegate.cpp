/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewitemdelegate.h"

#include "../../compat.h"
#include "../krcolorcache.h"
#include "../krglobal.h"
#include "../listpanel.h"
#include "krviewproperties.h"

// QtCore
#include <QDebug>
// QtGui
#include <QKeyEvent>
#include <QPainter>
// QtWidgets
#include <QApplication>
#include <QDialog>
#include <QLineEdit>

#include <KSharedConfig>

KrViewItemDelegate::KrViewItemDelegate(QObject *parent)
    : QItemDelegate(parent)
    , _currentlyEdited(-1)
    , _dontDraw(false)
    , _editor(nullptr)
{
}

void KrViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;
    _dontDraw = (_currentlyEdited == index.row()) && (index.column() == KrViewProperties::Ext);
    QItemDelegate::paint(painter, opt, index);
}

void KrViewItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    if (!_dontDraw)
        QItemDelegate::drawDisplay(painter, option, rect, text);
}

QWidget *KrViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const
{
    _currentlyEdited = index.row();
    _editor = QItemDelegate::createEditor(parent, sovi, index);
    return _editor;
}

void KrViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData(editor, index);
    auto *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        KConfigGroup gl(krConfig, "Look&Feel");
        QFont font = index.data(Qt::FontRole).value<QFont>();
        lineEdit->setFont(font);
        if (gl.readEntry("Rename Selects Extension", true))
            lineEdit->selectAll();
        else {
            QString nameWithoutExt = index.data(Qt::UserRole).toString();
            lineEdit->deselect();
            lineEdit->setSelection(0, static_cast<int>(nameWithoutExt.length()));
        }

        KrColorSettings colorSettings;

        if (!colorSettings.getBoolValue("KDE Default")) {
            QPalette renamePalette = lineEdit->palette();

            if (!colorSettings.getColorTextValue("Rename Foreground").isEmpty())
                renamePalette.setColor(QPalette::Text, colorSettings.getColorValue("Rename Foreground"));

            if (!colorSettings.getColorTextValue("Rename Background").isEmpty())
                renamePalette.setColor(QPalette::Base, colorSettings.getColorValue("Rename Background"));

            lineEdit->setPalette(renamePalette);
        }
    }
}

QSize KrViewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    QWidget *editor = qobject_cast<QWidget *>(object);
    if (!editor)
        return false;
    if (event->type() == QEvent::KeyPress) {
        switch (dynamic_cast<QKeyEvent *>(event)->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            onEditorClose();
            emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (auto *e = qobject_cast<QLineEdit *>(editor)) {
                if (!e->hasAcceptableInput())
                    return true;
                event->accept();
                emit commitData(editor);
                emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
                onEditorClose();
                return true;
            }
            return false;
        case Qt::Key_Escape:
            event->accept();
            // don't commit data
            onEditorClose();
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
            if (QApplication::activeModalWidget() && !QApplication::activeModalWidget()->isAncestorOf(editor)
                && qobject_cast<QDialog *>(QApplication::activeModalWidget()))
                return false;
            onEditorClose();
            // manually set focus back to panel after rename canceled by focusing another window
            ACTIVE_PANEL->gui->slotFocusOnMe();
            emit closeEditor(editor, RevertModelCache);
        }
    } else if (event->type() == QEvent::ShortcutOverride) {
        const QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape || (ke->key() == Qt::Key_Backspace && ke->modifiers() == Qt::ControlModifier)) {
            event->accept();
            return true;
        }
    }
    return false;
}

//! Helper class to represent an editor selection
class EditorSelection : public QPair<int, int>
{
public:
    EditorSelection(int start, int length)
        : QPair<int, int>(start, length)
    {
    }

    int start() const
    {
        return first;
    }
    int length() const
    {
        return second;
    }
};

//! Generate helpful file name selections: full name (always present), name candidates, extension candidates
static QList<EditorSelection> generateFileNameSelections(const QString &text)
{
    auto selections = QList<EditorSelection>();
    auto length = static_cast<int>(text.length());
    auto parts = text.split('.');

    // append full selection
    selections.append(EditorSelection(0, length));

    // append forward selections
    int selectionLength = 0;
    bool isFirstPart = true;
    for (auto part : parts) {
        // if the part is not the first one, we need to add one character to account for the dot
        selectionLength += static_cast<int>(part.length()) + !isFirstPart;
        isFirstPart = false;
        // if we reached the full length, don't add the selection, since it's a full selection
        if (selectionLength == length)
            break;

        // don't add empty selections (could happen if the full name starts with a dot)
        if (selectionLength > 0)
            selections.append(EditorSelection(0, selectionLength));
    }

    // append backward selections
    std::reverse(parts.begin(), parts.end());
    selectionLength = 0;
    isFirstPart = true;
    for (auto part : parts) {
        // if the part is not the first one, we need to add one character to account for the dot
        selectionLength += static_cast<int>(part.length()) + !isFirstPart;
        isFirstPart = false;
        // if we reached the full length, don't add the selection, since it's a full selection
        if (selectionLength == length)
            break;

        // don't add empty selections (could happen if the full name ends with a dot)
        if (selectionLength > 0)
            selections.append(EditorSelection(length - selectionLength, selectionLength));
    }

    return selections;
}

void KrViewItemDelegate::cycleEditorSelection()
{
    auto editor = qobject_cast<QLineEdit *>(_editor);
    if (!editor) {
        qWarning() << "Unable to cycle through editor selections due to a missing or unsupported type of item editor" << _editor;
        return;
    }

    EditorSelection currentSelection(editor->selectionStart(), editor->selectionLength());
    auto text = editor->text();
    const auto selections = generateFileNameSelections(text);

    // try to find current selection in the list
    qsizetype currentIndex = 0;
    for (auto selection : selections) {
        if (selection == currentSelection)
            break;
        currentIndex++;
    }

    // if we found current selection, pick the next in the cycle
    auto selectionCount = selections.length();
    if (currentIndex < selections.length())
        currentIndex = (currentIndex + 1) % selectionCount;
    // otherwise pick the first one - the full selection
    else
        currentIndex = 0;

    // set the selection
    auto selection = selections[currentIndex];
    qDebug() << "setting selection" << selection << "index" << currentIndex;
    editor->setSelection(selection.start(), selection.length());
}
