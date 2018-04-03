/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
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

#ifndef SYNCHRONIZEDIALOG_H
#define SYNCHRONIZEDIALOG_H

// QtWidgets
#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

#include "synchronizer.h"

class SynchronizeDialog : QDialog
{
    Q_OBJECT

public:
    SynchronizeDialog(QWidget*, Synchronizer *sync,
                      int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t, int);
    ~SynchronizeDialog();

    inline bool wasSyncronizationStarted()    {
        return syncStarted;
    }

public slots:
    void startSynchronization();
    void synchronizationFinished();
    void processedSizes(int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t);
    void pauseOrResume();
    void pauseAccepted();

private:
    QProgressBar  *progress;

    QCheckBox     *cbRightToLeft;
    QCheckBox     *cbLeftToRight;
    QCheckBox     *cbDeletable;

    QLabel        *lbRightToLeft;
    QLabel        *lbLeftToRight;
    QLabel        *lbDeletable;

    QCheckBox     *cbOverwrite;

    QPushButton   *btnStart;
    QPushButton   *btnPause;

    Synchronizer  *synchronizer;

    int               leftCopyNr;
    KIO::filesize_t   leftCopySize;
    int               rightCopyNr;
    KIO::filesize_t   rightCopySize;
    int               deleteNr;
    KIO::filesize_t   deleteSize;

    int               parallelThreads;

    bool           isPause;
    bool           syncStarted;
};

#endif /* __SYNCHRONIZE_DIALOG__ */
