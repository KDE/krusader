/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    FeedToListBoxDialog(QWidget *, Synchronizer *, QTreeWidget *, bool);
    ~FeedToListBoxDialog() override = default;

    bool isAccepted()
    {
        return accepted;
    }

protected slots:
    void slotOk();

private:
    Synchronizer *synchronizer;
    QTreeWidget *syncList;
    QCheckBox *cbSelected;
    QLineEdit *lineEdit;
    QComboBox *sideCombo;
    bool equalAllowed;
    bool accepted;
};

#endif /* __FEED_TO_LISTBOX_DIALOG__ */
