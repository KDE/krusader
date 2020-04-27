/*****************************************************************************
 * Copyright (C) 2018-2019 Shie Erlich <krusader@users.sourceforge.net>      *
 * Copyright (C) 2018-2019 Rafi Yanai <krusader@users.sourceforge.net>       *
 * Copyright (C) 2018-2019 Krusader Krew [https://krusader.org]              *
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

#include "krhistorycombobox.h"

// QtCore
#include <QEvent>
// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QAbstractItemView>

/**
 *  A KrHistoryComboBox event filter that e.g. deletes the current item when Shift+Del is pressed
 *  There was more information in https://doc.qt.io/qt-5/qobject.html#installEventFilter,
 *  https://forum.qt.io/post/160618 and
 *  https://stackoverflow.com/questions/17820947/remove-items-from-qcombobox-from-ui/52459337#52459337
 */
class KHBoxEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KHBoxEventFilter(QObject *parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

bool KHBoxEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key::Key_Delete) {
            auto comboBox = qobject_cast<KHistoryComboBox *>(obj);
            if (comboBox != nullptr) {
                QString entryToDelete = comboBox->currentText();
                // Delete the current item
                comboBox->removeItem(comboBox->currentIndex());
                // The item has to be deleted also from the completion list
                comboBox->completionObject()->removeItem(entryToDelete);
                return true;
            }
        }
    }
    // Perform the usual event processing
    return QObject::eventFilter(obj, event);
}

/**
 *  An event filter for the popup list of a KrHistoryComboBox, e.g. it deletes the current
 *  item when the user presses Shift+Del
 */
class KHBoxListEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KHBoxListEventFilter(QObject *parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

bool KHBoxListEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ShiftModifier && keyEvent->key() == Qt::Key::Key_Delete) {
            auto itemView = qobject_cast<QAbstractItemView *>(obj);
            if (itemView->model() != nullptr) {
                QString entryToDelete = itemView->currentIndex().data().toString();
                // Delete the current item from the popup list
                itemView->model()->removeRow(itemView->currentIndex().row());
                // The item has to be deleted also from the completion list of the KHistoryComboBox
                if (itemView->parent() != nullptr) {
                    auto comboBox = qobject_cast<KHistoryComboBox *>(itemView->parent()->parent());
                    if (comboBox != nullptr) {
                        comboBox->completionObject()->removeItem(entryToDelete);
                        return true;
                    }
                }
            }
        }
    }
    // Perform the usual event processing
    return QObject::eventFilter(obj, event);
}

#include "krhistorycombobox.moc" // required for class definitions with Q_OBJECT macro in implementation files

KrHistoryComboBox::KrHistoryComboBox(bool useCompletion, QWidget *parent)
    : KHistoryComboBox(useCompletion, parent)
{
    installEventFilter(new KHBoxEventFilter(this));

    QAbstractItemView *itemView = view();
    if (itemView != nullptr)
        itemView->installEventFilter(new KHBoxListEventFilter(this));
}
