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
#include <QPushButton>
#include <QProgressBar>

#include <KCoreAddons/KJob>
#include <KWidgetsAddons/KToolBarPopupAction>

class KrJob;


/**
 * @brief The job manager provides a progress dialog and control over (KIO) file operation jobs.
 *
 * Job manager does not have a window (or dialog). All functions are provided via toolbar actions
 * Icon and text are already set for them, shortcuts are missing.
 *
 * Job manager mode can have three states:
 * TODO
 *  -queue: Only first job is started. If more jobs are incoming via manageJob() they are
 *      suspended (if not already suspended). If the running job finishes the next job in line is
 *      started.
 *  -unmanaged: No job state changes are automatically done.
 *  -paused: All incoming jobs are suspended (if not already suspended).
 *
 * Note that the desktop system (e.g. KDE Plasma Shell) may also has control over the jobs.
 *
 * Reference: plasma-workspace/kuiserver/progresslistdelegate.h
 *
 * TODO: if a job still exists Krusader does not exit on quit() until the job is finished. If it is
 * suspended this takes forever.
 *
 * About undoing jobs: All jobs are recorded in VFS and we listen to FileUndoManager (which is a
 * singleton) about its capabilities here.
 * It would be great if each job could be undone invividually but FileUndoManager is currently
 * (KF5.27) only able to undo the last recorded job.
 */
class JobMan : public QObject
{
    Q_OBJECT

public:
    explicit JobMan(QObject *parent = 0);
    /** Toolbar action icon for pausing/starting all jobs with drop down menu showing all jobs.*/
    QAction *controlAction() { return _controlAction; }
    /** Toolbar action progress bar showing the average job progress percentage of all jobs.*/
    QAction *progressAction() { return _progressAction; }
    /** Toolbar action combo box for changing the .*/
    QAction *modeAction() { return _modeAction; }
    QAction *undoAction() { return _undoAction; }

public slots:
    /** Display, monitor and give user ability to control a job.
    *   If enqueued the job is not started. Otherwise this depends on the job manager mode.
    */
    void manageJob(KrJob *krJob, bool enqueue);

protected slots:
    void slotKJobStarted(KJob *krJob);
    void slotControlActionTriggered();
    void slotModeChange(int index);
    void slotPercent(KJob *, unsigned long);
    void slotDescription(KJob*,const QString &description, const QPair<QString,QString> &field1,
                         const QPair<QString,QString> &field2);
    void slotTerminated(KrJob *krJob);
    void slotUpdateControlAction();
    void slotUndoTextChange(const QString &text);

private:
    /** See description above.*/
    enum JobMode {
        //MODE_LAST = -1,
        // NOTE: values used for combobox index
        MODE_QUEUEING = 0,
        MODE_LAZY = 1,
        MODE_UNMANAGED = 2,
    };

    void updateUI();
    bool jobsAreRunning();

    QList<KrJob *> _jobs; // all jobs not terminated (finished or canceled) yet
    JobMode _currentMode;

    KToolBarPopupAction *_controlAction;
    QProgressBar *_progressBar;
    QAction *_progressAction;
    QAction *_modeAction;
    QAction *_undoAction;

    static const QString sDefaultToolTip;
};

#endif // JOBMAN_H
