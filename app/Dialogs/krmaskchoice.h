/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <krusader@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRMASKCHOICE_H
#define KRMASKCHOICE_H

// QtWidgets
#include <QDialog>

class QLabel;
class QListWidgetItem;
class QPushButton;
class KComboBox;
class KrListWidget;

class KrMaskChoice : public QDialog
{
    Q_OBJECT

public:
    explicit KrMaskChoice(QWidget *parent = nullptr);
    ~KrMaskChoice() override;

    KComboBox *selection;
    QLabel *PixmapLabel1;
    QLabel *label;
    KrListWidget *preSelections;
    QPushButton *PushButton7;
    QPushButton *PushButton7_2;
    QPushButton *PushButton7_3;

public slots:
    virtual void addSelection();
    virtual void clearSelections();
    virtual void deleteSelection();
    virtual void acceptFromList(QListWidgetItem *);
    virtual void currentItemChanged(QListWidgetItem *);
};

#endif // KRMASKCHOICE_H
