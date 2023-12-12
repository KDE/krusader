/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRUSADERVIEW_H
#define KRUSADERVIEW_H

// QtCore
#include <QEvent>
// QtWidgets
#include <QGridLayout>
#include <QLayout>
#include <QPushButton>
#include <QSplitter>
#include <QWidget>

#include "krglobal.h"

class PanelManager;
class ListPanel;
class KFnKeys;
class KCMDLine;
class TerminalDock;

class KrusaderView : public QWidget
{
    Q_OBJECT

public:
    explicit KrusaderView(QWidget *parent = nullptr);
    ~KrusaderView() override = default;
    void start(const KConfigGroup &cfg, bool restoreSettings, const QList<QUrl> &leftTabs, const QList<QUrl> &rightTabs);
    void updateGUI(const KConfigGroup &cfg);
    void saveSettings(KConfigGroup &cfg);
    void cmdLineFocus(); // command line receives keyboard focus
    void cmdLineUnFocus(); // return focus from command line to active panel

    bool isLeftActive() const
    {
        return leftMng == activeMng;
    }

    // used by krGlobal macros
    PanelManager *activeManager() const
    {
        return activeMng;
    }
    PanelManager *inactiveManager() const
    {
        return activeMng == leftMng ? rightMng : leftMng;
    }
    PanelManager *leftManager() const
    {
        return leftMng;
    }
    PanelManager *rightManager() const
    {
        return rightMng;
    }
    KrPanel *activePanel() const;
    ListPanel *leftPanel() const;
    ListPanel *rightPanel() const;

    KFnKeys *fnKeys() const
    {
        return _fnKeys;
    }
    KCMDLine *cmdLine() const
    {
        return _cmdLine;
    }
    TerminalDock *terminalDock() const
    {
        return _terminalDock;
    }
    bool isVertical() const
    {
        return horiz_splitter != nullptr ? horiz_splitter->orientation() == Qt::Vertical : false;
    }
    void swapSides();
    void setPanelSize(bool leftPanel, int percent);
    bool isTerminalEmulatorFullscreen();

public slots:
    void slotSetActiveManager(PanelManager *manager);
    void slotPathChanged(ListPanel *listPanel);
    // Tab - switch focus
    void panelSwitch();
    void toggleVerticalMode();

    void setTerminalEmulator(bool show, bool fullscreen = false);
    void focusTerminalEmulator();
    void toggleFullScreenTerminalEmulator();

    void focusUp();
    void focusDown();

    void profiles(const QString &profileName = QString());
    void loadPanelProfiles(const QString &group);
    void savePanelProfiles(const QString &group);

    void draggingTab(PanelManager *from, QMouseEvent *e);
    void draggingTabFinished(PanelManager *from, QMouseEvent *e);

private:
    int getFocusCandidates(QVector<QWidget *> &widgets);
    bool cursorIsOnOtherSide(PanelManager *of, const QPoint &globalPos);
    PanelManager *createManager(bool left);
    void updateCurrentActivePath();

    KFnKeys *_fnKeys; // function keys
    KCMDLine *_cmdLine; // command line widget
    TerminalDock *_terminalDock; // docking widget for terminal emulator
    QSplitter *horiz_splitter, *vert_splitter;
    QList<int> verticalSplitterSizes;
    PanelManager *activeMng, *leftMng, *rightMng; // saving them for panel swaps
    QGridLayout *mainLayout, *terminal_layout;
};

#endif
