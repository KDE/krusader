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

#include "krpleasewait.h"

// QtCore
#include <QTimer>
#include <QDateTime>
// QtGui
#include <QCloseEvent>
// QtWidgets
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KCursor>

#include "../krglobal.h"

KRPleaseWait::KRPleaseWait(QString msg, QWidget *parent, int count, bool cancel):
        QProgressDialog(cancel ? nullptr : parent) , inc(true)
{
    setModal(!cancel);

    timer = new QTimer(this);
    setWindowTitle(i18n("Krusader::Wait"));

    setMinimumDuration(500);
    setAutoClose(false);
    setAutoReset(false);

    connect(timer, &QTimer::timeout, this, &KRPleaseWait::cycleProgress);

    QProgressBar* progress = new QProgressBar(this);
    progress->setMaximum(count);
    progress->setMinimum(0);
    setBar(progress);

    QLabel* label = new QLabel(this);
    setLabel(label);

    QPushButton* btn = new QPushButton(i18n("&Cancel"), this);
    setCancelButton(btn);

    btn->setEnabled(canClose = cancel);
    setLabelText(msg);

    show();
}

void KRPleaseWait::closeEvent(QCloseEvent * e)
{
    if (canClose) {
        emit canceled();
        e->accept();
    } else              /* if cancel is not allowed, we disable */
        e->ignore();         /* the window closing [x] also */
}

void KRPleaseWait::incProgress(int howMuch)
{
    setValue(value() + howMuch);
}

void KRPleaseWait::cycleProgress()
{
    if (inc) setValue(value() + 1);
    else     setValue(value() - 1);
    if (value() >= 9) inc = false;
    if (value() <= 0) inc = true;
}

KRPleaseWaitHandler::KRPleaseWaitHandler(QWidget *parentWindow)
    : QObject(parentWindow), _parentWindow(parentWindow), job(), dlg(nullptr)
{
}

void KRPleaseWaitHandler::stopWait()
{
    if (dlg != nullptr) delete dlg;
    dlg = nullptr;
    cycleMutex = incMutex = false;
    // return cursor to normal arrow
    _parentWindow->setCursor(Qt::ArrowCursor);
}


void KRPleaseWaitHandler::startWaiting(QString msg, int count , bool cancel)
{
    if (dlg == nullptr) {
        dlg = new KRPleaseWait(msg , _parentWindow, count, cancel);
        connect(dlg, &KRPleaseWait::canceled, this, &KRPleaseWaitHandler::killJob);
    }
    incMutex = cycleMutex = _wasCancelled = false;
    dlg->setValue(0);

    dlg->setLabelText(msg);
    if (count == 0) {
        dlg->setMaximum(10);
        cycle = true;
        cycleProgress();
    } else {
        dlg->setMaximum(count);
        cycle = false;
    }
}

void KRPleaseWaitHandler::cycleProgress()
{
    if (cycleMutex) return;
    cycleMutex = true;
    if (dlg) dlg->cycleProgress();
    if (cycle) QTimer::singleShot(2000, this, &KRPleaseWaitHandler::cycleProgress);
    cycleMutex = false;
}

void KRPleaseWaitHandler::killJob()
{
    if (!job.isNull()) job->kill(KJob::EmitResult);
    stopWait();
    _wasCancelled = true;
}

void KRPleaseWaitHandler::setJob(KIO::Job* j)
{
    job = j;
}

void KRPleaseWaitHandler::incProgress(int i)
{
    if (incMutex) return;
    incMutex = true;
    if (dlg) dlg->incProgress(i);
    incMutex = false;
}

