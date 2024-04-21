/*
    SPDX-FileCopyrightText: 2006 Václav Juza <vaclavjuza@gmail.com>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmdmodebutton.h"

#include "../icon.h"
#include "../kractions.h"

// QtWidgets
#include <QMenu>

#include <KActionMenu>
#include <KLocalizedString>

/**
 * KCMDModeButton class, which represents a button with popup menu to switch
 * the mode of the krusader built-in command-line
 */
KCMDModeButton::KCMDModeButton(QWidget *parent)
    : QToolButton(parent)
{
    /* // from the old terminal-button:
      setText( i18n( "If pressed, Krusader executes command line in a terminal." ) );
      setToolTip( i18n( "If pressed, Krusader executes command line in a terminal." ) );
      QWhatsThis::add( terminal, i18n(
                            "The 'run in terminal' button allows Krusader "
                            "to run console (or otherwise non-graphical) "
                            "programs in a terminal of your choice. If it's "
                            "pressed, terminal mode is active." ) );
    */
    setIcon(Icon("utilities-terminal"));
    adjustSize();
    action = new KActionMenu(i18n("Execution mode"), this);
    Q_CHECK_PTR(action);
    for (int i = 0; KrActions::execTypeArray[i] != nullptr; i++) {
        action->addAction(*KrActions::execTypeArray[i]);
    }
    QMenu *pP = action->menu();
    Q_CHECK_PTR(pP);
    setMenu(pP);
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);
}

KCMDModeButton::~KCMDModeButton()
{
    delete action;
}

/** called when clicked to the button */
void KCMDModeButton::showMenu()
{
    QMenu *pP = menu();
    if (pP) {
        menu()->exec(mapToGlobal(QPoint(0, 0)));
    }
}
