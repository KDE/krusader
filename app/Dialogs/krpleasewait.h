/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRPLEASEWAIT_H
#define KRPLEASEWAIT_H

// QtCore
#include <QPointer>
#include <QTimer>
// QtGui
#include <QCloseEvent>
// QtWidgets
#include <QProgressDialog>

#include <KIO/Job>

class KrPleaseWait;

class KrPleaseWaitHandler : public QObject
{
    Q_OBJECT

public:
    explicit KrPleaseWaitHandler(QWidget *parentWindow);

public slots:

    void startWaiting(const QString &msg, int count = 0, bool cancel = false);
    void stopWait();
    void cycleProgress();
    void incProgress(int i);
    void killJob();
    void setJob(KIO::Job *j);
    bool wasCancelled() const
    {
        return _wasCancelled;
    }

private:
    QWidget *_parentWindow;
    QPointer<KIO::Job> job;
    KrPleaseWait *dlg;
    bool cycle, cycleMutex, incMutex, _wasCancelled;
};

class KrPleaseWait : public QProgressDialog
{
    Q_OBJECT
public:
    KrPleaseWait(const QString &msg, QWidget *parent, int count = 0, bool cancel = false);

public slots:
    void incProgress(int howMuch);
    void cycleProgress();

protected:
    bool inc;
    QTimer *timer;
    void closeEvent(QCloseEvent *e) override;
    bool canClose;
};

#endif
