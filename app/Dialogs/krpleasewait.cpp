/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krpleasewait.h"

// QtCore
#include <QDateTime>
#include <QTimer>
// QtGui
#include <QCloseEvent>
// QtWidgets
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

#include <KCursor>
#include <KLocalizedString>

#include "../krglobal.h"

KrPleaseWait::KrPleaseWait(const QString &msg, QWidget *parent, int count, bool cancel)
    : QProgressDialog(cancel ? nullptr : parent)
    , inc(true)
{
    setModal(!cancel);

    timer = new QTimer(this);
    setWindowTitle(i18n("Krusader::Wait"));

    setMinimumDuration(500);
    setAutoClose(false);
    setAutoReset(false);

    connect(timer, &QTimer::timeout, this, &KrPleaseWait::cycleProgress);

    auto *progress = new QProgressBar(this);
    progress->setMaximum(count);
    progress->setMinimum(0);
    setBar(progress);

    QLabel *label = new QLabel(this);
    setLabel(label);

    QPushButton *btn = new QPushButton(i18n("&Cancel"), this);
    setCancelButton(btn);

    btn->setEnabled(canClose = cancel);
    setLabelText(msg);

    show();
}

void KrPleaseWait::closeEvent(QCloseEvent *e)
{
    if (canClose) {
        emit canceled();
        e->accept();
    } else /* if cancel is not allowed, we disable */
        e->ignore(); /* the window closing [x] also */
}

void KrPleaseWait::incProgress(int howMuch)
{
    setValue(value() + howMuch);
}

void KrPleaseWait::cycleProgress()
{
    if (inc)
        setValue(value() + 1);
    else
        setValue(value() - 1);
    if (value() >= 9)
        inc = false;
    if (value() <= 0)
        inc = true;
}

KrPleaseWaitHandler::KrPleaseWaitHandler(QWidget *parentWindow)
    : QObject(parentWindow)
    , _parentWindow(parentWindow)
    , dlg(nullptr)
{
}

void KrPleaseWaitHandler::stopWait()
{
    if (dlg != nullptr)
        delete dlg;
    dlg = nullptr;
    cycleMutex = incMutex = false;
    // return cursor to normal arrow
    _parentWindow->setCursor(Qt::ArrowCursor);
}

void KrPleaseWaitHandler::startWaiting(const QString &msg, int count, bool cancel)
{
    if (dlg == nullptr) {
        dlg = new KrPleaseWait(msg, _parentWindow, count, cancel);
        connect(dlg, &KrPleaseWait::canceled, this, &KrPleaseWaitHandler::killJob);
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

void KrPleaseWaitHandler::cycleProgress()
{
    if (cycleMutex)
        return;
    cycleMutex = true;
    if (dlg)
        dlg->cycleProgress();
    if (cycle)
        QTimer::singleShot(2000, this, &KrPleaseWaitHandler::cycleProgress);
    cycleMutex = false;
}

void KrPleaseWaitHandler::killJob()
{
    if (!job.isNull())
        job->kill(KJob::EmitResult);
    stopWait();
    _wasCancelled = true;
}

void KrPleaseWaitHandler::setJob(KIO::Job *j)
{
    job = j;
}

void KrPleaseWaitHandler::incProgress(int i)
{
    if (incMutex)
        return;
    incMutex = true;
    if (dlg)
        dlg->incProgress(i);
    incMutex = false;
}
