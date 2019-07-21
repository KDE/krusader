/*****************************************************************************
 * Copyright (C) 2006 Jonas Bähr <jonas.baehr@web.de>                        *
 * Copyright (C) 2006-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef ACTIONMAN_H
#define ACTIONMAN_H

// QtWidgets
#include <QDialog>

class UserActionPage;

/**
 * This manages all useractions
 */
class ActionMan : public QDialog
{
    Q_OBJECT
public:
    explicit ActionMan(QWidget* parent = 0);
    ~ActionMan();

protected slots:
    void slotClose();
    void slotApply();
    void slotEnableApplyButton();
    void slotDisableApplyButton();

private:
    UserActionPage* userActionPage;
    QPushButton *applyButton;
};

#endif // ifndef ACTIONMAN_H
