/*****************************************************************************
 * Copyright (C) 2008 Václav Juza <vaclavjuza@gmail.com>                     *
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

#include "terminaldock.h"

// QtCore
#include <QEvent>
#include <QDir>
#include <QString>
#include <QObject>
#include <QUrl>
// QtGui
#include <QKeyEvent>
#include <QClipboard>
// QtWidgets
#include <QHBoxLayout>
#include <QApplication>
#include <QWidget>

#include <kde_terminal_interface.h>
#include <KCoreAddons/KPluginLoader>
#include <KCoreAddons/KPluginFactory>
#include <KI18n/KLocalizedString>
#include <KService/KService>
#include <KWidgetsAddons/KToggleAction>
#include <KWidgetsAddons/KMessageBox>

#include "kcmdline.h"
#include "../kractions.h"
#include "../krmainwindow.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../FileSystem/filesystem.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/listpanel.h"
#include "../Panel/listpanelactions.h"
#include "../Panel/panelfunc.h"

/**
 * A widget containing the konsolepart for the Embedded terminal emulator
 */
TerminalDock::TerminalDock(QWidget* parent, KrMainWindow *mainWindow) : QWidget(parent),
    _mainWindow(mainWindow), konsole_part(0), t(0), initialised(false)
{
    terminal_hbox = new QHBoxLayout(this);
}

TerminalDock::~TerminalDock()
{
}

bool TerminalDock::initialise()
{
    if (! initialised) { // konsole part is not yet loaded or it has already failed
        KService::Ptr service = KService::serviceByDesktopName("konsolepart");

        if (service) {
            QWidget *focusW = qApp->focusWidget();
            // Create the part
            QString error;
            konsole_part = service->createInstance<KParts::ReadOnlyPart>(this, this, QVariantList(), &error);

            if (konsole_part) { //loaded successfully
                terminal_hbox->addWidget(konsole_part->widget());
                setFocusProxy(konsole_part->widget());
                connect(konsole_part, SIGNAL(destroyed()),
                        this, SLOT(killTerminalEmulator()));
                // must filter app events, because some of them are processed
                // by child widgets of konsole_part->widget()
                // and would not be received on konsole_part->widget()
                qApp->installEventFilter(this);
                t = qobject_cast<TerminalInterface*>(konsole_part);
                if (t) {
                    lastPath = QDir::currentPath();
                    t->showShellInDir(lastPath);
                }
                initialised = true;
            } else
                KMessageBox::error(0, i18n("<b>Cannot create embedded terminal.</b><br/>"
                                           "The reported error was: %1", error));
            // the Terminal Emulator may be hidden (if we are creating it only
            // to send command there and see the results later)
            if (focusW) {
                focusW->setFocus();
            } else {
                ACTIVE_PANEL->gui->slotFocusOnMe();
            }
        } else
            KMessageBox::sorry(0, i18nc("missing program - arg1 is a URL",
                                        "<b>Cannot create embedded terminal.</b><br>"
                                        "You can fix this by installing Konsole:<br/>%1",
                                        QString("<a href='%1'>%1</a>").arg(
                                            "http://www.kde.org/applications/system/konsole")),
                               0, KMessageBox::AllowLink);
    }
    return isInitialised();
}

void TerminalDock::killTerminalEmulator()
{
    initialised = false;
    konsole_part = NULL;
    t = NULL;
    qApp->removeEventFilter(this);
    MAIN_VIEW->setTerminalEmulator(false);
}

void TerminalDock::sendInput(const QString& input, bool clearCommand)
{
    if (!t)
        return;

    if (clearCommand) {
        // send SIGINT before input command to avoid unwanted behaviour when current line is not empty
        // and command is appended to current input (e.g. "rm -rf x " concatenated with 'cd /usr');
        // code "borrowed" from Dolphin, Copyright (C) 2007-2010 by Peter Penz <peter.penz19@gmail.com>
        const int processId = t->terminalProcessId();
        if (processId > 0) {
            kill(processId, SIGINT);
        }
    }

    t->sendInput(input);
}

/*! Sends a `cd` command to the embedded terminal emulator so as to synchronize the directory of the actual panel and
    the directory of the embedded terminal emulator.

    To avoid that Krusader's embedded terminal adds a lot of `cd` messages to the shell history: the user has to use
    bash and have set `HISTCONTROL=ignorespace` or `HISTCONTROL=ignoreboth` (which is the default in a lot of Linux
    distributions so in that case the user hasn't got to do anything), or the user has to use an equivalent method.
*/
void TerminalDock::sendCd(const QString& path)
{
    if (path.compare(lastPath) != 0) {
        // A space exists in front of the `cd` so as to avoid that Krusader's embedded terminal adds a lot of `cd`
        // messages to the shell history, in Dolphin it's done the same way: https://bugs.kde.org/show_bug.cgi?id=204039
        sendInput(QString(" cd ") + KrServices::quote(path) + QString("\n"));
        lastPath = path;
    }
}

bool TerminalDock::applyShortcuts(QKeyEvent * ke)
{
    int pressedKey = (ke->key() | ke->modifiers());

    // TODO KF5 removed
    if (KrActions::actToggleTerminal->shortcut().matches(pressedKey)) {
        KrActions::actToggleTerminal->activate(QAction::Trigger);
        return true;
    }

    if (!krSwitchFullScreenTE->shortcut().isEmpty() && krSwitchFullScreenTE->shortcut().matches(pressedKey)) {
        krSwitchFullScreenTE->activate(QAction::Trigger);
        return true;
    }

    if (_mainWindow->listPanelActions()->actPaste->shortcut().matches(pressedKey)) {
        QString text = QApplication::clipboard()->text();
        if (! text.isEmpty()) {
            text.replace('\n', '\r');
            sendInput(text, false);
        }
        return true;
    }

    //insert current to the terminal
    if ((ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) &&
        (ke->modifiers() & Qt::ControlModifier)) {
        SLOTS->insertFileName((ke->modifiers() & Qt::ShiftModifier) != 0);
        return true;
    }

    //navigation
    if ((ke->key() ==  Qt::Key_Down) && (ke->modifiers() == Qt::ControlModifier)) {
        if (MAIN_VIEW->cmdLine()->isVisible()) {
            MAIN_VIEW->cmdLine()->setFocus();
        }
        return true;
    } else if ((ke->key() ==  Qt::Key_Up)
               && ((ke->modifiers()  == Qt::ControlModifier)
                   || (ke->modifiers()  == (Qt::ControlModifier | Qt::ShiftModifier)))) {
        ACTIVE_PANEL->gui->slotFocusOnMe();
        return true;
    }

    return false;
}

bool TerminalDock::eventFilter(QObject * watched, QEvent * e)
{
    if (konsole_part == NULL || konsole_part->widget() == NULL)
        return false;

    // we must watch for child widgets as well,
    // otherwise some shortcuts are "eaten" by them before
    // being procesed in konsole_part->widget() context
    // examples are Ctrl+F, Ctrl+Enter
    QObject *w;
    for (w = watched; w != NULL; w = w->parent())
        if (w == konsole_part->widget())
            break;
    if (w == NULL)    // is not a child of konsole_part
        return false;

    switch (e->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *ke = (QKeyEvent *)e;
        // If not present, some keys would be considered a shortcut, for example "a"
        if ((ke->key() ==  Qt::Key_Insert) && (ke->modifiers()  == Qt::ShiftModifier)) {
            ke->accept();
            return true;
        }
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier) &&
                (ke->key() >= 32) && (ke->key() <= 127)) {
            ke->accept();
            return true;
        }
        break;
    }
    case QEvent::KeyPress: {
        QKeyEvent *ke = (QKeyEvent *)e;
        if (applyShortcuts(ke)) {
            ke->accept();
            return true;
        }
        break;
    }
    default:
        return false;
    }
    return false;
}

bool TerminalDock::isTerminalVisible() const
{
    return isVisible() && konsole_part != NULL && konsole_part->widget() != NULL
           && konsole_part->widget()->isVisible();
}

bool TerminalDock::isInitialised() const
{
    return konsole_part != NULL && konsole_part->widget() != NULL;
}

void TerminalDock::hideEvent(QHideEvent * /*e*/)
{
    // BUGFIX: when the terminal emulator is toggled on, first it is shown in minimum size
    //         then QSplitter resizes it to the desired size.
    //         this minimum resize scrolls up the content of the konsole widget
    // SOLUTION:
    //         we hide the console widget while the resize ceremony happens, then reenable it
    if (konsole_part && konsole_part->widget())
        konsole_part->widget()->hide(); // hide the widget to prevent from resize
}

void TerminalDock::showEvent(QShowEvent * /*e*/)
{
    if (konsole_part && konsole_part->widget()) {
        // BUGFIX: TE scrolling bug (see upper)
        //         show the Konsole part delayed
        QTimer::singleShot(0, konsole_part->widget(), SLOT(show()));
    }
}

