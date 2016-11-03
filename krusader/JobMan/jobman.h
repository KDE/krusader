/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef JOBMAN_H
#define JOBMAN_H

// QtCore
#include <QAction>
// QtWidgets
#include <QProgressBar>

#include <KCoreAddons/KJob>
#include <KWidgetsAddons/KToolBarPopupAction>

/**
 * @brief The job manager provides a progress dialog and control over (KIO) file operation jobs.
 *
 * Job manager does not have a window (or dialog). All funtions are provided via toolbar actions.
 *
 * TODO
 * Jobs can be queued, i.e. they are given to the manager in suspend state and can be started
 * later by the user.
 *
 * Note that the desktop system (KDE Plasma shell) may also has control over the jobs.
 *
 * Reference: plasma-workspace/kuiserver/progresslistdelegate.h
 *
 * TODO: if a job still exists Krusader does not exit on quit() until the job is finished. If it is
 * suspended this takes forever.
 */
class JobMan : public QObject
{
    Q_OBJECT

public:
    explicit JobMan(QObject *parent = 0);
    QAction *menuAction() { return _menuAction; }
    QAction *progressAction() { return _progressAction; }
    QAction *pauseResumeAction() { return _pauseResumeButton; }
    QAction *cancelAction() { return _cancelButton; }

public slots:
    /** Display, monitor and give user ability to control a job*/
    void manageJob(KJob *job);

protected slots:
    void slotPauseResumeButtonClicked();
    void slotCancelButtonClicked();
    void slotPercent(KJob *, unsigned long percent);
    void slotDescription(KJob*,const QString &description, const QPair<QString,QString> &field1,
                         const QPair<QString,QString> &field2);
    void slotFinished(KJob *);
    void slotSuspended(KJob *);
    void slotResumed(KJob *);

private:
    void setCurrentJob(KJob *job);

    KJob *_currentJob;

    KToolBarPopupAction *_menuAction;
    QProgressBar *_progressBar;
    QAction *_pauseResumeButton;
    QAction *_cancelButton;

    QAction *_progressAction;

    static const QString sDefaultToolTip;
};

#endif // JOBMAN_H
