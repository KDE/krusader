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

#include "jobman.h"

// QtWidgets
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KI18n/KLocalizedString>

#include <../krglobal.h>


/** The menu action entry for a job in the popup menu.*/
class JobMenuAction : public QWidgetAction
{
    Q_OBJECT

public:
    JobMenuAction(KJob *job, QObject *parent) : QWidgetAction(parent), _job(job)
    {
        QWidget *container = new QWidget();
        QGridLayout *layout = new QGridLayout(container);
        _description = new QLabel();
        _progressBar = new QProgressBar();
        layout->addWidget(_description, 0, 0, 1, 3);
        layout->addWidget(_progressBar, 1, 0);

        _pauseResumeButton = new QPushButton();
        _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        connect(_pauseResumeButton, &QPushButton::clicked, this,
              &JobMenuAction::slotPauseResumeButtonClicked);
        layout->addWidget(_pauseResumeButton, 1, 1);

        _cancelButton = new QPushButton();
        _cancelButton->setIcon(QIcon::fromTheme("remove"));
        _cancelButton->setToolTip(i18n("Cancel/Clear Job"));
        connect(_cancelButton, &QPushButton::clicked,
                this, &JobMenuAction::slotCancelButtonClicked);
        layout->addWidget(_cancelButton, 1, 2);

        setDefaultWidget(container);

        connect(job, &KJob::description, this, &JobMenuAction::slotDescription);
        connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
              SLOT(slotPercent(KJob *, unsigned long)));
        connect(job, &KJob::suspended, this, &JobMenuAction::slotSuspended);
        connect(job, &KJob::resumed, this, &JobMenuAction::slotResumed);
        connect(job, &KJob::finished, this, &JobMenuAction::slotFinished);
    }

protected slots:
    void slotDescription(KJob *, const QString &description,
                       const QPair<QString, QString> &field1,
                       const QPair<QString, QString> &field2)
    {
        _description->setText(QString("%1 - %2: %3").arg(description, field2.first, field2.second));
        setToolTip(QString("%1 %2").arg(field1.second, field1.second));
    }

    void slotPercent(KJob *, unsigned long percent) { _progressBar->setValue(percent); }

    void slotSuspended(KJob *)
    {
        _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }

    void slotResumed(KJob *)
    {
        _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    }

    void slotFinished()
    {
        _progressBar->setValue(_job->percent()); // in case signal for '100%' is not emitted
        _job = 0;
        _pauseResumeButton->setEnabled(false);
        _cancelButton->setIcon(QIcon::fromTheme("edit-clear"));
        _cancelButton->setToolTip(i18n("Clear"));
    }

    void slotPauseResumeButtonClicked()
    {
        if (!_job)
          return;

        if (_job->isSuspended())
          _job->resume();
        else
          _job->suspend();
    }

    void slotCancelButtonClicked()
    {
        if (_job) {
          _job->kill();
        } else {
          deleteLater();
        }
    }

private:
    KJob *_job;

    QLabel *_description;
    QProgressBar *_progressBar;
    QPushButton *_pauseResumeButton;
    QPushButton *_cancelButton;
};

#include "jobman.moc" // required for class definitions with Q_OBJECT macro in implementation files


const QString JobMan::sDefaultToolTip = i18n("No jobs");

JobMan::JobMan(QObject *parent) : QObject(parent)
{
    _controlAction = new KToolBarPopupAction(QIcon::fromTheme("media-playback-pause"),
                                          i18n("Play/Pause &Job"), this);
    _controlAction->setEnabled(false);
    connect(_controlAction, &QAction::triggered, this, &JobMan::slotPauseResumeButtonClicked);

    QMenu *menu = new QMenu(krMainWindow);
    menu->setMinimumWidth(300);
    _controlAction->setMenu(menu);

    _progressBar = new QProgressBar();
    _progressBar->setToolTip(sDefaultToolTip);
    _progressBar->setEnabled(false);
    // listen to clicks on progress bar
    _progressBar->installEventFilter(this);

    QWidgetAction *progressAction = new QWidgetAction(krMainWindow);
    progressAction->setText("Job Progress Bar");
    progressAction->setDefaultWidget(_progressBar);
    _progressAction = progressAction;
}

void JobMan::manageJob(KJob *job)
{

    _jobs.append(job);

    JobMenuAction *menuAction = new JobMenuAction(job, _controlAction);
    connect(menuAction, &QObject::destroyed, this, &JobMan::slotUpdateControlAction);
    _controlAction->menu()->addAction(menuAction);
    slotUpdateControlAction();

    // KJob has two percent() functions
    connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
            SLOT(slotPercent(KJob *, unsigned long)));
    connect(job, &KJob::description, this, &JobMan::slotDescription);
    connect(job, &KJob::finished, this, &JobMan::slotFinished);
    connect(job, &KJob::suspended, this, &JobMan::updateUI);
    connect(job, &KJob::resumed, this, &JobMan::updateUI);

    updateUI();
}

void JobMan::slotPauseResumeButtonClicked()
{
    if (_jobs.isEmpty()) {
        _controlAction->menu()->clear();
        _controlAction->setEnabled(false);
        return;
    }

    bool allPaused = true;
    for (KJob *job: _jobs) {
        allPaused &= job->isSuspended();
    }

    for (KJob *job: _jobs) {
        if (allPaused)
            job->resume();
        else
            job->suspend();
    }
}

void JobMan::slotPercent(KJob *, unsigned long)
{
    updateUI();
}

void JobMan::slotDescription(KJob*,const QString &description, const QPair<QString,QString> &field1,
                     const QPair<QString,QString> &field2)
{
    // TODO cache all descriptions
    if (_jobs.length() > 1)
        return;

    _progressBar->setToolTip(
        QString("%1\n%2: %3\n%4: %5")
            .arg(description, field1.first, field1.second, field2.first, field2.second));
}

void JobMan::slotFinished(KJob *job)
{
    // Note: by default KJobs are deleting its after finished
    _jobs.removeAll(job);

    updateUI();
}

void JobMan::slotUpdateControlAction()
{
    _controlAction->setEnabled(!_controlAction->menu()->isEmpty());
}

void JobMan::updateUI()
{
    bool allPaused = true;
    int totalPercent = 0;
    for (KJob *job: _jobs) {
        totalPercent += job->percent();
        allPaused &= job->isSuspended();
    }

    bool hasJobs = !_jobs.isEmpty();
    _progressBar->setEnabled(hasJobs);
    if (hasJobs) {
        _progressBar->setValue(totalPercent / _jobs.length());
    } else {
        _progressBar->reset();
    }
    if (!hasJobs)
        _progressBar->setToolTip(i18n("No Jobs"));
    if (_jobs.length() > 1)
        _progressBar->setToolTip(i18n("%1 Jobs", _jobs.length()));

    _controlAction->setIcon(QIcon::fromTheme(
        !hasJobs ? "edit-clear" : !allPaused ? "media-playback-pause" : "media-playback-start"));
    _controlAction->setToolTip(
        !hasJobs ? i18n("Clear list") : !allPaused ? i18n("Pause All Jobs") :
                                                     i18n("Resume All Jobs"));
}
