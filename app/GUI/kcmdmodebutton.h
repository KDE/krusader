/*
    SPDX-FileCopyrightText: 2006 Václav Juza <vaclavjuza@gmail.com>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
