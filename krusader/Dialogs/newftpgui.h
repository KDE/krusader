/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2009 Fathi Boudra <fboudra@gmail.com>                       *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef NEWFTPGUI_H
#define NEWFTPGUI_H

// QtWidgets
#include <QDialog>
#include <QLabel>
#include <QSpinBox>

#include <KCompletion/KComboBox>
#include <KCompletion/KHistoryComboBox>
#include <KCompletion/KLineEdit>

/**
 * The "New Network Connection" dialog
 */
class newFTPGUI : public QDialog
{
    Q_OBJECT

public:
    explicit newFTPGUI(QWidget *parent = nullptr);
    ~newFTPGUI() override;

    KComboBox* prefix;
    KHistoryComboBox* url;
    QSpinBox* port;
    KLineEdit* username;
    KLineEdit* password;

protected:
    bool event(QEvent *) Q_DECL_OVERRIDE;

private slots:
    void slotTextChanged(const QString &);

private:
    QLabel* iconLabel;
    QLabel* aboutLabel;
    QLabel* protocolLabel;
    QLabel* passwordLabel;
    QLabel* hostLabel;
    QLabel* usernameLabel;
    QLabel* portLabel;
};

#endif
