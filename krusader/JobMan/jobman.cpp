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
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KI18n/KLocalizedString>

#include <../krglobal.h>


/** The menu action entry in the popup menu for a job.*/
class JobMenuAction : public QWidgetAction {
    Q_OBJECT

public:
    JobMenuAction(KJob *job) : QWidgetAction(job) {
        _job = job;

        QWidget *container = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->addWidget(&_description);
        layout->addWidget(&_progressBar);

        setDefaultWidget(container);

        connect(job, &KJob::description, this, &JobMenuAction::slotDescription);
        connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
                SLOT(slotPercent(KJob *, unsigned long)));
    }

    KJob *job() { return _job; }

protected slots:
    void slotDescription(KJob*,const QString &description, const QPair<QString,QString> &field1,
                         const QPair<QString,QString> &field2) {
        _description.setText(QString("%1 - %2: %3").arg(description, field2.first, field2.second));
        setToolTip(QString("%1 %2").arg(field1.second, field1.second));
    }

    void slotPercent(KJob *, unsigned long percent) {
        _progressBar.setValue(percent);
    }

private:

    KJob *_job;
    QLabel _description;
    QProgressBar _progressBar;
};

#include "jobman.moc" // required for class definitions with Q_OBJECT macro in implementation files


const QString JobMan::sDefaultToolTip = i18n("No jobs");

JobMan::JobMan(QObject *parent) : QObject(parent), _currentJob(0)
{
    _menuAction = new KToolBarPopupAction(QIcon::fromTheme("filename-group-length"),
                                          i18n("&Job List"), this);
    _menuAction->setEnabled(false);

    QMenu *menu = new QMenu(krMainWindow);
    menu->setMinimumWidth(300);
    _menuAction->setMenu(menu);
    //layout->addWidget(_menuButton);

    _progressBar = new QProgressBar();
    _progressBar->setToolTip(sDefaultToolTip);
    _progressBar->setEnabled(false);
    // listen to clicks on progress bar
    _progressBar->installEventFilter(this);

    QWidgetAction *progressAction = new QWidgetAction(krMainWindow);
    progressAction->setText("Job Progress Bar");
    progressAction->setDefaultWidget(_progressBar);
    _progressAction = progressAction;

    _pauseResumeButton = new QAction(QIcon::fromTheme("media-playback-pause"),
                                     i18n("Play/Pause Job"), this);
    _pauseResumeButton->setEnabled(false);
    connect(_pauseResumeButton, &QAction::triggered, this, &JobMan::slotPauseResumeButtonClicked);

    _cancelButton = new QAction(QIcon::fromTheme("edit-clear"), i18n("Cancel/Clear Job"), this);
    _cancelButton->setEnabled(false);
    connect(_cancelButton, &QAction::triggered, this, &JobMan::slotCancelButtonClicked);
}

void JobMan::manageJob(KJob *job)
{
    JobMenuAction *menuAction = new JobMenuAction(job);
    _menuAction->menu()->addAction(menuAction);
    // Note: this must be done before setting current job
    connect(job, &KJob::finished, [=](KJob *) { _menuAction->menu()->removeAction(menuAction); });

    if (!_currentJob)
        setCurrentJob(job);
}

void JobMan::slotPauseResumeButtonClicked()
{
    if (!_currentJob)
        return;

    if (_currentJob->isSuspended())
        _currentJob->resume();
    else
        _currentJob->suspend();
}

void JobMan::slotCancelButtonClicked()
{
    if (_currentJob) {
        _currentJob->kill();
    } else {
        // clear view
        _progressBar->reset();
        _progressBar->setToolTip(sDefaultToolTip);
        _cancelButton->setEnabled(false);
    }
}

void JobMan::slotPercent(KJob *, unsigned long percent)
{
    _progressBar->setValue(percent);
}

void JobMan::slotFinished(KJob *)
{
    _progressBar->setEnabled(false);
    _pauseResumeButton->setEnabled(false);
    _cancelButton->setIcon(QIcon::fromTheme("edit-clear"));

    // Note: by default KJobs are deleting its after finished
    _currentJob = 0;

    QList<QAction*> actions = _menuAction->menu()->actions();
    if (actions.isEmpty()) {
        _menuAction->setEnabled(false);
    } else {
        // set next job
        setCurrentJob(qobject_cast<JobMenuAction *>(actions[0])->job());
    }
}

void JobMan::slotDescription(KJob*,const QString &description, const QPair<QString,QString> &field1,
                     const QPair<QString,QString> &field2) {
    _progressBar->setToolTip(QString("%1\n%2: %3\n%4: %5").arg(description,
                                                               field1.first, field1.second,
                                                               field2.first, field2.second));
}

void JobMan::slotSuspended(KJob *)
{
    _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-start"));
}

void JobMan::slotResumed(KJob *)
{
    _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-pause"));
}

void JobMan::setCurrentJob(KJob *job)
{
    _currentJob = job;
    _progressBar->reset();
    _progressBar->setEnabled(true);
     _menuAction->setEnabled(true);
    _pauseResumeButton->setEnabled(true);
    if (_currentJob->isSuspended())
        slotSuspended(job);
    else
        slotResumed(job);
    _cancelButton->setEnabled(true);
    _cancelButton->setIcon(QIcon::fromTheme("remove"));

    // KJob has two percent() functions
    connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
            SLOT(slotPercent(KJob *, unsigned long)));
    connect(job, &KJob::description, this, &JobMan::slotDescription);
    connect(job, &KJob::finished, this, &JobMan::slotFinished);
    connect(job, &KJob::suspended, this, &JobMan::slotSuspended);
    connect(job, &KJob::resumed, this, &JobMan::slotResumed);
}
