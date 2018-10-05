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

#ifndef KRPLEASEWAIT_H
#define KRPLEASEWAIT_H

// QtCore
#include <QTimer>
#include <QPointer>
// QtGui
#include <QCloseEvent>
// QtWidgets
#include <QProgressDialog>

#include <KIO/Job>

class KRPleaseWait;

class KRPleaseWaitHandler : public QObject
{
    Q_OBJECT

public:
    explicit KRPleaseWaitHandler(QWidget *parentWindow);

public slots:

    void startWaiting(const QString& msg, int count = 0, bool cancel = false);
    void stopWait();
    void cycleProgress();
    void incProgress(int i);
    void killJob();
    void setJob(KIO::Job* j);
    bool wasCancelled() const {
        return _wasCancelled;
    }

private:
    QWidget *_parentWindow;
    QPointer<KIO::Job> job;
    KRPleaseWait * dlg;
    bool cycle, cycleMutex, incMutex, _wasCancelled;
};


class KRPleaseWait : public QProgressDialog
{
    Q_OBJECT
public:
    KRPleaseWait(const QString& msg, QWidget *parent, int count = 0 , bool cancel = false);

public slots:
    void incProgress(int howMuch);
    void cycleProgress();

protected:
    bool inc;
    QTimer* timer;
    void closeEvent(QCloseEvent * e) Q_DECL_OVERRIDE;
    bool canClose;
};

#endif
