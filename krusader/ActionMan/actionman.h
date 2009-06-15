/*****************************************************************************
 * Copyright (C) 2006 Jonas Bähr <jonas.baehr@web.de>                        *
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

#ifndef ACTIONMAN_H
#define ACTIONMAN_H

#include <kdialog.h>

class UserActionPage;

/**
 * This manages all useractions
 */
class ActionMan : public KDialog
{
    Q_OBJECT
public:
    ActionMan(QWidget* parent = 0);
    ~ActionMan();

protected slots:
    void slotClose();
    void slotApply();
    void slotEnableApplyButton();
    void slotDisableApplyButton();

private:
    UserActionPage* userActionPage;
};

#endif // ifndef ACTIONMAN_H
