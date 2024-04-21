/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONFIGURATOR_H
#define KONFIGURATOR_H

#include "konfiguratorpage.h"

// QtCore
#include <QTimer>

#include <KPageDialog>

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

    void reject() override;

protected:
    void newPage(KonfiguratorPage *, const QString &, const QString &, const QIcon &); // adds widget and connects to slot
    void createLayout(int startPage);
    void closeDialog();

    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *event) override;

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
    QList<KPageWidgetItem *> kgPages;
    bool firstTime;
    KPageWidgetItem *lastPage;
    bool internalCall;
    QTimer restoreTimer;
    int sizeX;
    int sizeY;
};

#endif
