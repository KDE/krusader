/***************************************************************************
                                krusaderview.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRUSADERVIEW_H
#define KRUSADERVIEW_H

// KDE includes
#include <klocale.h>
#include <kapplication.h>
#include <kparts/part.h>

// QT includes
#include <QtGui/QLayout>
#include <QtGui/QSplitter>
#include <qwidget.h>
#include <QtGui/QPushButton>
#include <QGridLayout>
#include <QEvent>

#include "krglobal.h"

//HACK
#include "Panel/krpanel.h"

class PanelManager;
class ListPanel;

// forward declaration
class KFnKeys;
class KCMDLine;
class TerminalDock;

class KrusaderView : public QWidget
{
    Q_OBJECT

public:
    KrusaderView(QWidget *parent = 0);
    virtual ~KrusaderView() {}
    void start(KConfigGroup &cfg, bool restoreSettings, QStringList leftTabs, QStringList rightTabs);
    void saveSettings(KConfigGroup &cfg);
    void cmdLineFocus();  // command line receive's keyboard focus
    void cmdLineUnFocus();// return focus from command line to active panel
    inline bool isLeftActive() const {
        return (ACTIVE_PANEL->gui == left);
    }
    inline PanelManager *activeManager() const {
        return activeMng;
    }
    inline PanelManager *inactiveManager() const {
        return activeMng == leftMng ? rightMng : leftMng;
    }
    QList<int> getTerminalEmulatorSplitterSizes();
    inline bool          isVertical() const {
        return horiz_splitter != 0 ? horiz_splitter->orientation() == Qt::Vertical : false;
    }

public slots:
    void slotSetActivePanel(ListPanel *p);
    void slotPathChanged(ListPanel *p);
    void slotTerminalEmulator(bool);
    // manage the function keys to the CURRENT vfs
    //////////////////////////////////////////////
    // Tab - switch focus
    void panelSwitch();
    void toggleVerticalMode();

    void focusTerminalEmulator();
    void switchFullScreenTE();

    void focusUp();
    void focusDown();

    void profiles(QString profileName = QString());
    void loadPanelProfiles(QString group);
    void savePanelProfiles(QString group);

    void killTerminalEmulator();

public:
    ListPanel  *left, *right;       // the actual panels
    PanelManager *activeMng, *leftMng, *rightMng;       // saving them for panel swaps
    KFnKeys   *fnKeys;          // function keys
    KCMDLine    *cmdLine;                   // command line widget
    TerminalDock  *terminal_dock;             // docking widget for terminal emulator
    QSplitter  *horiz_splitter, *vert_splitter;
    QList<int>   verticalSplitterSizes;

private:
    int getFocusCandidates(QVector<QWidget*> &widgets);

    QGridLayout *mainLayout, *terminal_layout;
};

#endif
