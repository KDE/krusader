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

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QWidget>
#include <QDir>
#include <QString>
#include <QObject>

#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <ktoggleaction.h>
#include <kurl.h>

#include "../kractions.h"
#include "../krusaderapp.h"
#include "../krusaderview.h"
#include "kcmdline.h"
#include "../VFS/vfs.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../krservices.h"

/**
 * A widget containing the konsolepart for the Embedded terminal emulator
 */
TerminalDock::TerminalDock(QWidget* parent) : QWidget(parent)
{
    initialised = false;
    t = NULL;
    konsole_part = NULL;
    terminal_hbox = new QHBoxLayout(this);
}

TerminalDock::~TerminalDock()
{
}

bool TerminalDock::initialise()
{
    if (! initialised) { // konsole part is not yet loaded or it has already failed
        KPluginFactory * factory = KPluginLoader("libkonsolepart").factory();
        if (factory) {
            QWidget *focusW = qApp->focusWidget();
            // Create the part
            konsole_part = factory->create<KParts::ReadOnlyPart>((QObject *)this);

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
            }
            // the Terminal Emulator may be hidden (if we are creating it only
            // to send command there and see the results later)
            if (focusW) {
                focusW->setFocus();
            } else {
                ACTIVE_PANEL->slotFocusOnMe();
            }
        }
        initialised = true;
    }
    return isInitialised();
}

void TerminalDock::killTerminalEmulator()
{
    initialised = false;
    konsole_part = NULL;
    t = NULL;
    qApp->removeEventFilter(this);
    MAIN_VIEW->killTerminalEmulator();
}

void TerminalDock::sendInput(const QString& input)
{
    if (t) {
        t->sendInput(input);
    }
}

void TerminalDock::sendCd(const QString& path)
{
    if (path.compare(lastPath) != 0) {
        sendInput(QString("cd ") + KrServices::quote(path) + QString("\n"));
        lastPath = path;
    }
}

bool TerminalDock::applyShortcuts(QKeyEvent * ke)
{
    int pressedKey = (ke->key() | ke->modifiers());

    if (krToggleTerminal->shortcut().contains(pressedKey)) {
        krToggleTerminal->activate(QAction::Trigger);
        return true;
    }

    if (krSwitchFullScreenTE->shortcut().contains(pressedKey)) {
        krSwitchFullScreenTE->activate(QAction::Trigger);
        return true;
    }

    if (KrActions::actPaste->shortcut().contains(pressedKey)) {
        QString text = QApplication::clipboard()->text();
        if (! text.isEmpty()) {
            text.replace('\n', '\r');
            sendInput(text);
        }
        return true;
    }

    //insert current to the terminal
    if ((ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
            && ((ke->modifiers() & ~Qt::ShiftModifier) == Qt::ControlModifier)) {

        QString filename = ACTIVE_PANEL->view->getCurrentItem();
        if (filename.isEmpty()) {
            return true;
        }
        if (ke->modifiers() & Qt::ShiftModifier) {
            QString path = vfs::pathOrUrl(ACTIVE_FUNC->files()->vfs_getOrigin(), KUrl::AddTrailingSlash);
            filename = path + filename;
        }

        filename = KrServices::quote(filename);

        sendInput(QString(" ") + filename + QString(" "));
        return true;
    }

    //navigation
    if ((ke->key() ==  Qt::Key_Down) && (ke->modifiers() == Qt::ControlModifier)) {
        if (MAIN_VIEW->cmdLine->isVisible()) {
            MAIN_VIEW->cmdLine->setFocus();
        }
        return true;
    } else if ((ke->key() ==  Qt::Key_Up)
               && ((ke->modifiers()  == Qt::ControlModifier)
                   || (ke->modifiers()  == (Qt::ControlModifier | Qt::ShiftModifier)))) {
        ACTIVE_PANEL->slotFocusOnMe();
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

#include "terminaldock.moc"
