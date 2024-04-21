/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOBMAN_H
#define JOBMAN_H

// QtCore
#include <QAction>
// QtWidgets
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>

#include <KJob>
#include <KToolBarPopupAction>

class KrJob;

/**
 * @brief The job manager provides a progress dialog and control over (KIO) file operation jobs.
 *
 * Job manager does not have a window (or dialog). All functions are provided via toolbar actions.
 * Icon, text and tooltip are already set but shortcuts are not set here.
 *
 * If job managers queue mode is activated, only the first job (incoming via manageJob()) is started
 * and more incoming jobs are delayed: If the running job finishes, the next job in line is
 * started.
 *
 * NOTE: The desktop system (e.g. KDE Plasma Shell) may also can control the jobs.
 * Reference: plasma-workspace/kuiserver/progresslistdelegate.h
 *
 * NOTE: If a job still exists, Krusader does not exit on quit() and waits until the job is
 * finished. If the job is paused, this takes forever. Call waitForJobs() before exit to prevent
 * this.
 *
 * About undoing jobs: If jobs are recorded (all KrJobs are, some in FileSystem) we can undo them
 * with FileUndoManager (which is a singleton) here.
 * It would be great if each job in the job list could be undone individually but FileUndoManager
 * is currently (Frameworks 5.27) only able to undo the last recorded job.
 */
class JobMan : public QObject
{
    Q_OBJECT

public:
    /** Job start mode for new jobs. */
    enum StartMode {
        /** Enqueue or start job - depending on QueueMode. */
        Default,
        /** Enqueue job. I.e. start if no other jobs are running. */
        Enqueue,
        /** Job is always started. */
        Start,
        /** Job is always not started now. */
        Delay
    };

    explicit JobMan(QObject *parent = nullptr);
    /** Toolbar action icon for pausing/starting all jobs with drop down menu showing all jobs.*/
    QAction *controlAction() const
    {
        return m_controlAction;
    }
    /** Toolbar action progress bar showing the average job progress percentage of all jobs.*/
    QAction *progressAction() const
    {
        return m_progressAction;
    }
    /** Toolbar action combo box for changing the .*/
    QAction *modeAction() const
    {
        return m_modeAction;
    }
    QAction *undoAction() const
    {
        return m_undoAction;
    }

    /** Wait for all jobs to terminate (blocking!).
     *
     * Returns true immediately if there are no jobs and user input is not required. Otherwise a
     * modal UI dialog is shown and the user can abort all jobs or cancel the dialog.
     *
     * @param waitForUserInput if true dialog is only closed after user interaction (button click)
     * @return true if no jobs are running (anymore) and user wants to quit. Else false
     */
    bool waitForJobs(bool waitForUserInput);

    /** Return if queue mode is enabled or not. */
    bool isQueueModeEnabled() const
    {
        return m_queueMode;
    }

    /** Display, monitor and give user ability to control a job.
     *
     * Whether the job is started now or delayed depends on startMode (and current queue mode flag).
     */
    void manageJob(KrJob *krJob, StartMode startMode = Default);
    /**
     * Like manageJob(), but for already started jobs.
     */
    void manageStartedJob(KrJob *krJob, KJob *job);

protected slots:
    void slotKJobStarted(KJob *krJob);
    void slotControlActionTriggered();
    void slotPercent(KJob *, unsigned long);
    void slotDescription(KJob *, const QString &description, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);
    void slotTerminated(KrJob *krJob);
    void slotUpdateControlAction();
    void slotUndoTextChange(const QString &text);
    void slotUpdateMessageBox();

private:
    void managePrivate(KrJob *job, KJob *kJob = nullptr);
    void cleanupMenu(); // remove old entries if menu is too long
    void updateUI();
    bool jobsAreRunning();

    QList<KrJob *> m_jobs; // all jobs not terminated (finished or canceled) yet
    bool m_queueMode;

    KToolBarPopupAction *m_controlAction;
    QProgressBar *m_progressBar;
    QAction *m_progressAction;
    QAction *m_modeAction;
    QAction *m_undoAction;

    QMessageBox *m_messageBox;
    bool m_autoCloseMessageBox;

    static const QString sDefaultToolTip;
};

#endif // JOBMAN_H
