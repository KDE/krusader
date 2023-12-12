/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYNCHRONIZEDIALOG_H
#define SYNCHRONIZEDIALOG_H

// QtWidgets
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

#include "synchronizer.h"

class SynchronizeDialog : QDialog
{
    Q_OBJECT

public:
    SynchronizeDialog(QWidget *, Synchronizer *sync, int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t, int);
    ~SynchronizeDialog() override;

    inline bool wasSyncronizationStarted()
    {
        return syncStarted;
    }

public slots:
    void startSynchronization();
    void synchronizationFinished();
    void processedSizes(int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t);
    void pauseOrResume();
    void pauseAccepted();

private:
    QProgressBar *progress;

    QCheckBox *cbRightToLeft;
    QCheckBox *cbLeftToRight;
    QCheckBox *cbDeletable;

    QLabel *lbRightToLeft;
    QLabel *lbLeftToRight;
    QLabel *lbDeletable;

    QCheckBox *cbOverwrite;

    QPushButton *btnStart;
    QPushButton *btnPause;

    Synchronizer *synchronizer;

    int leftCopyNr;
    KIO::filesize_t leftCopySize;
    int rightCopyNr;
    KIO::filesize_t rightCopySize;
    int deleteNr;
    KIO::filesize_t deleteSize;

    int parallelThreads;

    bool isPause;
    bool syncStarted;
};

#endif /* __SYNCHRONIZE_DIALOG__ */
