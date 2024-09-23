/*
    SPDX-FileCopyrightText: 2008 Václav Juza <vaclavjuza@gmail.com>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: 2007-2010 Peter Penz <peter.penz19@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "terminaldock.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QObject>
#include <QString>
#include <QUrl>
// QtGui
#include <QClipboard>
#include <QKeyEvent>
// QtWidgets
#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KService>
#include <KToggleAction>
#include <kde_terminal_interface.h>

#include "../FileSystem/filesystem.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/listpanel.h"
#include "../Panel/listpanelactions.h"
#include "../Panel/panelfunc.h"
#include "../defaults.h"
#include "../kractions.h"
#include "../krmainwindow.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "kcmdline.h"

/**
 * A widget containing the konsolepart for the Embedded terminal emulator
 */
TerminalDock::TerminalDock(QWidget *parent, KrMainWindow *mainWindow)
    : QWidget(parent)
    , _mainWindow(mainWindow)
    , konsole_part(nullptr)
    , terminal(nullptr)
    , initialised(false)
    , firstInput(true)
{
    terminal_hbox = new QHBoxLayout(this);
}

TerminalDock::~TerminalDock()
{
    if (konsole_part) {
        disconnect(konsole_part, &KParts::ReadOnlyPart::destroyed, this, &TerminalDock::killTerminalEmulator);
    }
};

bool TerminalDock::initialise()
{
    if (!initialised) { // konsole part is not yet loaded or it has already failed

        const KPluginMetaData pluginMetaData(QStringLiteral("kf6/parts/konsolepart"));
        KPluginFactory *pluginFactory = KPluginFactory::loadFactory(pluginMetaData).plugin;

        if (pluginFactory) {
            QWidget *focusW = qApp->focusWidget();
            // Create the part
            konsole_part = pluginFactory->create<KParts::ReadOnlyPart>(this);
            if (konsole_part) { // loaded successfully
                terminal_hbox->addWidget(konsole_part->widget());
                setFocusProxy(konsole_part->widget());
                connect(konsole_part, &KParts::ReadOnlyPart::destroyed, this, &TerminalDock::killTerminalEmulator);
                // must filter app events, because some of them are processed
                // by child widgets of konsole_part->widget()
                // and would not be received on konsole_part->widget()
                qApp->installEventFilter(this);
                terminal = qobject_cast<TerminalInterface *>(konsole_part);
                if (terminal) {
                    lastPath = QDir::currentPath();
                    terminal->showShellInDir(lastPath);
                }
                initialised = true;
                firstInput = true;
            } else
                KMessageBox::error(nullptr,
                                   i18n("<b>Cannot create embedded terminal.</b><br/>"));
            // the Terminal Emulator may be hidden (if we are creating it only
            // to send command there and see the results later)
            if (focusW) {
                focusW->setFocus();
            } else {
                ACTIVE_PANEL->gui->slotFocusOnMe();
            }
        } else
            KMessageBox::error(nullptr,
                               i18nc("missing program - arg1 is a URL",
                                     "<b>Cannot create embedded terminal.</b><br>"
                                     "You can fix this by installing Konsole:<br/>%1",
                                     QString("<a href='%1'>%1</a>").arg("https://www.kde.org/applications/system/konsole")),
                               nullptr,
                               KMessageBox::AllowLink);
    }
    return isInitialised();
}

void TerminalDock::killTerminalEmulator()
{
    qDebug() << "killed";

    initialised = false;
    konsole_part = nullptr;
    terminal = nullptr;
    qApp->removeEventFilter(this);
    MAIN_VIEW->setTerminalEmulator(false);
}

void TerminalDock::sendInput(const QString &input, bool clearCommand)
{
    if (!terminal)
        return;

    if (clearCommand) {
        // send SIGINT before input command to avoid unwanted behaviour when current line is not empty
        // and command is appended to current input (e.g. "rm -rf x " concatenated with 'cd /usr');
        // code "borrowed" from Dolphin
        const int processId = terminal->terminalProcessId();
        // workaround (firstInput): kill is sent to terminal if shell is not initialized yet
        if (processId > 0 && !firstInput) {
            kill(processId, SIGINT);
        }
    }
    firstInput = false;

    terminal->sendInput(input);
}

/*! Sends a `cd` command to the embedded terminal emulator so as to synchronize the directory of the actual panel and
    the directory of the embedded terminal emulator.

    To avoid that Krusader's embedded terminal adds a lot of `cd` messages to the shell history: the user has to use
    bash and have set `HISTCONTROL=ignorespace` or `HISTCONTROL=ignoreboth` (which is the default in a lot of Linux
    distributions so in that case the user hasn't got to do anything), or the user has to use an equivalent method.
*/
void TerminalDock::sendCd(const QString &path)
{
    if (path.compare(lastPath) != 0) {
        // A space exists in front of the `cd` so as to avoid that Krusader's embedded terminal adds a lot of `cd`
        // messages to the shell history, in Dolphin it's done the same way: https://bugs.kde.org/show_bug.cgi?id=204039
        sendInput(QString(" cd ") + KrServices::quote(path) + QString("\n"));
        lastPath = path;
    }
}

bool TerminalDock::applyShortcuts(QKeyEvent *ke)
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

    if (_mainWindow->listPanelActions()->actPaste->shortcuts().contains(pressedKey)) {
        QString text = QApplication::clipboard()->text();
        if (!text.isEmpty()) {
            text.replace('\n', '\r');
            sendInput(text, false);
        }
        return true;
    }

    // insert current to the terminal
    if ((ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) && (ke->modifiers() & Qt::ControlModifier)) {
        SLOTS->insertFileName((ke->modifiers() & Qt::ShiftModifier) != 0);
        return true;
    }

    // navigation
    if ((ke->key() == Qt::Key_Down) && (ke->modifiers() == Qt::ControlModifier)) {
        if (MAIN_VIEW->cmdLine()->isVisible()) {
            MAIN_VIEW->cmdLine()->setFocus();
        }
        return true;
    } else if ((ke->key() == Qt::Key_Up) && ((ke->modifiers() == Qt::ControlModifier) || (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)))) {
        ACTIVE_PANEL->gui->slotFocusOnMe();
        return true;
    }

    return false;
}

bool TerminalDock::eventFilter(QObject *watched, QEvent *e)
{
    if (konsole_part == nullptr || konsole_part->widget() == nullptr)
        return false;

    // we must watch for child widgets as well,
    // otherwise some shortcuts are "eaten" by them before
    // being processed in konsole_part->widget() context
    // examples are Ctrl+F, Ctrl+Enter
    QObject *w;
    for (w = watched; w != nullptr; w = w->parent())
        if (w == konsole_part->widget())
            break;
    if (w == nullptr) // is not a child of konsole_part
        return false;

    switch (e->type()) {
    case QEvent::ShortcutOverride: {
        auto *ke = dynamic_cast<QKeyEvent *>(e);
        // If not present, some keys would be considered a shortcut, for example "a"
        if ((ke->key() == Qt::Key_Insert) && (ke->modifiers() == Qt::ShiftModifier)) {
            ke->accept();
            return true;
        }
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier) && (ke->key() >= 32) && (ke->key() <= 127)) {
            ke->accept();
            return true;
        }
        break;
    }
    case QEvent::KeyPress: {
        auto *ke = dynamic_cast<QKeyEvent *>(e);
        if (applyShortcuts(ke)) {
            ke->accept();
            return true;
        }
        break;
    }
    case QEvent::FocusIn:
    case QEvent::FocusOut: {
        onTerminalFocusChanged(e->type() == QEvent::FocusIn);
        break;
    }
    default:
        return false;
    }
    return false;
}

bool TerminalDock::isTerminalVisible() const
{
    return isVisible() && konsole_part != nullptr && konsole_part->widget() != nullptr && konsole_part->widget()->isVisible();
}

bool TerminalDock::isInitialised() const
{
    return konsole_part != nullptr && konsole_part->widget() != nullptr;
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
        QTimer::singleShot(0, konsole_part->widget(), &QWidget::show);
    }
}

void TerminalDock::onTerminalFocusChanged(bool focused)
{
    if (konsole_part == nullptr) {
        return;
    }

    if (!focused) {
        disconnect(konsole_part, SIGNAL(currentDirectoryChanged(QString)), nullptr, nullptr);
        return;
    }

    const KConfigGroup cfg = krConfig->group("General");
    if (!cfg.readEntry("Follow Terminal CD", _FollowTerminalCD)) {
        return;
    }

    connect(konsole_part, SIGNAL(currentDirectoryChanged(QString)), this, SLOT(currentDirChanged(QString)));
}

void TerminalDock::currentDirChanged(const QString &terminalPath)
{
    const QString panelPath = ACTIVE_PANEL->virtualPath().toLocalFile();
    if (panelPath == terminalPath) {
        return;
    }

    qDebug() << "terminal cwd changed=" << terminalPath << "panelPath=" << panelPath;
    lastPath = terminalPath;
    _mainWindow->activePanel()->func->openUrl(QUrl::fromLocalFile(terminalPath));
}
