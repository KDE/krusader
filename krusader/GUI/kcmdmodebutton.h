/*****************************************************************************
 * Copyright (C) 2006 Václav Juza <vaclavjuza@gmail.com>                     *
 * Copyright (C) 2006-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef KCMDMODEBUTTON_H
#define KCMDMODEBUTTON_H

// QtWidgets
#include <QToolButton>

class KActionMenu;

/**
 * represents a button for switching the command-line execution mode
 * It extends QToolButton, set the icon etc., and creates a popup menu
 * containing the actions to actually switch the mode.
 */
class KCMDModeButton : public QToolButton
{
    Q_OBJECT
public:
    /** Constructor. Sets up the menu, and the icon */
    explicit KCMDModeButton(QWidget *parent = nullptr);
    ~KCMDModeButton() override;

    /** Shows the popup menu. Called when clicked to the button */
    void showMenu();

private:
    /** The menu containing the actions for switching the mode */
    KActionMenu *action;
};

#endif
