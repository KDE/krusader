/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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
#ifndef KRMASKCHOICE_H
#define KRMASKCHOICE_H

// QtWidgets
#include <QDialog>

class QLabel;
class QListWidgetItem;
class QPushButton;
class KComboBox;
class KrListWidget;

class KRMaskChoice : public QDialog
{
    Q_OBJECT

public:
    explicit KRMaskChoice(QWidget* parent = nullptr);
    ~KRMaskChoice() override;

    KComboBox* selection;
    QLabel* PixmapLabel1;
    QLabel* label;
    KrListWidget* preSelections;
    QPushButton* PushButton7;
    QPushButton* PushButton7_2;
    QPushButton* PushButton7_3;

public slots:
    virtual void addSelection();
    virtual void clearSelections();
    virtual void deleteSelection();
    virtual void acceptFromList(QListWidgetItem *);
    virtual void currentItemChanged(QListWidgetItem *);
};

#endif // KRMASKCHOICE_H
