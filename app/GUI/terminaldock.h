/*
    SPDX-FileCopyrightText: 2008 Václav Juza <vaclavjuza@gmail.com>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TERMINALDOCK_H
#define TERMINALDOCK_H

// QtCore
#include <QEvent>
#include <QObject>
#include <QString>
// QtWidgets
#include <QHBoxLayout>
#include <QWidget>

#include <KParts/ReadOnlyPart>
#include <kde_terminal_interface.h>

class KrMainWindow;

/**
 * This is a class to make the work with the embedded terminal emulator easier.
 * It takes care of loading the library and initialising when necessary.
 * A big advantage is that the code which uses this class does not need to
 * check if the part was loaded successfully.
 *
 */
class TerminalDock : public QWidget
{
    Q_OBJECT
public:
    TerminalDock(QWidget *parent, KrMainWindow *mainWindow);
    ~TerminalDock() override;
    void sendInput(const QString &input, bool clearCommand = true);
    void sendCd(const QString &path);
    bool eventFilter(QObject *watched, QEvent *e) override;
    bool isTerminalVisible() const;
    bool isInitialised() const;
    bool initialise();
    void hideEvent(QHideEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void onTerminalFocusChanged(bool focused);

private slots:
    void killTerminalEmulator();
    void currentDirChanged(const QString &terminalPath);

private:
    KrMainWindow *_mainWindow;
    QString lastPath; // path of the last sendCd
    QHBoxLayout *terminal_hbox; // hbox for terminal_dock
    KParts::ReadOnlyPart *konsole_part; // the actual part pointer
    TerminalInterface *terminal; // TerminalInterface of the konsole part (same object)
    bool initialised;
    bool firstInput;
    bool applyShortcuts(QKeyEvent *ke);
};

#endif /* TERMINALDOCK_H */
