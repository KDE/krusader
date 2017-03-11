/***************************************************************************
                                konfigurator.h
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


#ifndef KONFIGURATOR_H
#define KONFIGURATOR_H

#include "konfiguratorpage.h"

// QtCore
#include <QTimer>

#include <KWidgetsAddons/KPageDialog>

class QString;
class QResizeEvent;
class QCloseEvent;

class Konfigurator : public KPageDialog
{
    Q_OBJECT

signals:
    void configChanged(bool isGUIRestartNeeded);

public:
    explicit Konfigurator(bool f = false, int startPage = 0); // true if Konfigurator is run for the first time

    void reject() Q_DECL_OVERRIDE;

protected:
    void newPage(KonfiguratorPage *, const QString &, const QString &, const QIcon &); // adds widget and connects to slot
    void createLayout(int startPage);
    void closeDialog();

    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void slotApplyEnable();
    bool slotPageSwitch(KPageWidgetItem *, KPageWidgetItem *);
    void slotRestorePage();
    void slotClose();
    void slotApply();
    void slotReset();
    void slotRestoreDefaults();
    void slotShowHelp();

private:
    QList<KPageWidgetItem*>     kgPages;
    bool                        firstTime;
    KPageWidgetItem            *lastPage;
    bool                        internalCall;
    QTimer                      restoreTimer;
    int                         sizeX;
    int                         sizeY;
};

#endif

