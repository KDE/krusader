/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/


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

