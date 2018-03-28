/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef FEEDTOLISTBOXDIALOG_H
#define FEEDTOLISTBOXDIALOG_H

// QtWidgets
#include <QDialog>

class Synchronizer;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QTreeWidget;

class FeedToListBoxDialog : public QDialog
{
    Q_OBJECT

public:
    FeedToListBoxDialog(QWidget*, Synchronizer *, QTreeWidget *, bool);
    virtual ~FeedToListBoxDialog() {}

    bool isAccepted() {
        return accepted;
    }

protected slots:
    void slotOk();

private:
    Synchronizer * synchronizer;
    QTreeWidget  * syncList;
    QCheckBox    * cbSelected;
    QLineEdit    * lineEdit;
    QComboBox    * sideCombo;
    bool           equalAllowed;
    bool           accepted;
};

#endif /* __FEED_TO_LISTBOX_DIALOG__ */
