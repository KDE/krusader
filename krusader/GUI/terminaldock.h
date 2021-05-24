/*
    SPDX-FileCopyrightText: 2008 Václav Juza <vaclavjuza@gmail.com>
    SPDX-FileCopyrightText: 2008-2020 Krusader Krew [https://krusader.org]

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TERMINALDOCK_H
#define TERMINALDOCK_H

// QtCore
#include <QEvent>
#include <QString>
#include <QObject>
// QtWidgets
#include <QHBoxLayout>
#include <QWidget>

#include <kde_terminal_interface.h>
#include <KParts/ReadOnlyPart>

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
    TerminalDock(QWidget* parent, KrMainWindow *mainWindow);
    ~TerminalDock() override;
    void sendInput(const QString& input, bool clearCommand=true);
    void sendCd(const QString& path);
    bool eventFilter(QObject * watched, QEvent * e) override;
    bool isTerminalVisible() const;
    bool isInitialised() const;
    bool initialise();
    void hideEvent(QHideEvent * e) override;
    void showEvent(QShowEvent * e) override;
    inline KParts::Part* part() {
        return konsole_part;
    }
private slots:
    void killTerminalEmulator();
private:
    KrMainWindow *_mainWindow;
    QString lastPath;                       // path of the last sendCd
    QHBoxLayout *terminal_hbox;             // hbox for terminal_dock
    KParts::ReadOnlyPart* konsole_part;     // the actual part pointer
    TerminalInterface* t;                   // TerminalInterface of the konsole part
    bool initialised;
    bool firstInput;
    bool applyShortcuts(QKeyEvent * ke);
};

#endif /* TERMINALDOCK_H */
