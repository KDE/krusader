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
class JobMenuAction : public QWidgetAction {
    Q_OBJECT

public:
  JobMenuAction(KJob *job) : QWidgetAction(job), _job(job)
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
      connect(_cancelButton, &QPushButton::clicked, this, &JobMenuAction::slotCancelButtonClicked);
      layout->addWidget(_cancelButton, 1, 2);

      setDefaultWidget(container);

      connect(job, &KJob::description, this, &JobMenuAction::slotDescription);
      connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
              SLOT(slotPercent(KJob *, unsigned long)));
      connect(job, &KJob::suspended, this, &JobMenuAction::slotSuspended);
      connect(job, &KJob::resumed, this, &JobMenuAction::slotResumed);

  }

  KJob *job() { return _job; }

protected slots:
  void slotDescription(KJob *, const QString &description,
                       const QPair<QString, QString> &field1,
                       const QPair<QString, QString> &field2)
  {
      _description->setText(QString("%1 - %2: %3").arg(description, field2.first, field2.second));
      // MY TODO tooltips not shown in menu
      setToolTip(QString("%1 %2").arg(field1.second, field1.second));
  }

  void slotPercent(KJob *, unsigned long percent) { _progressBar->setValue(percent); }

  void slotPauseResumeButtonClicked()
  {
      if (_job->isSuspended())
          _job->resume();
      else
          _job->suspend();
  }

  void slotCancelButtonClicked() { _job->kill(); }

  void slotSuspended(KJob *)
  {
      _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-start"));
  }

  void slotResumed(KJob *)
  {
      _pauseResumeButton->setIcon(QIcon::fromTheme("media-playback-pause"));
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

JobMan::JobMan(QObject *parent) : QObject(parent), _currentJob(0)
{
    _menuAction = new KToolBarPopupAction(QIcon::fromTheme("filename-group-length"),
                                          i18n("&Job List"), this);
    _menuAction->setEnabled(false);

    QMenu *menu = new QMenu(krMainWindow);
    menu->setMinimumWidth(300);
    _menuAction->setMenu(menu);

    _progressBar = new QProgressBar();
    _progressBar->setToolTip(sDefaultToolTip);
    _progressBar->setEnabled(false);
    // listen to clicks on progress bar
    _progressBar->installEventFilter(this);

    QWidgetAction *progressAction = new QWidgetAction(krMainWindow);
    progressAction->setText("Job Progress Bar");
    progressAction->setDefaultWidget(_progressBar);
    _progressAction = progressAction;

    _pauseResumeAction = new QAction(QIcon::fromTheme("media-playback-pause"),
                                     i18n("Play/Pause Job"), this);
    _pauseResumeAction->setEnabled(false);
    connect(_pauseResumeAction, &QAction::triggered, this, &JobMan::slotPauseResumeButtonClicked);

    _cancelAction = new QAction(QIcon::fromTheme("remove"), i18n("Cancel/Clear Job"), this);
    _cancelAction->setEnabled(false);
    connect(_cancelAction, &QAction::triggered, this, &JobMan::slotCancelButtonClicked);
}

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
        _cancelAction->setEnabled(false);
    }
}

void JobMan::slotPercent(KJob *, unsigned long percent)
{
    _progressBar->setValue(percent);
}

void JobMan::slotFinished(KJob *)
{
    _progressBar->setEnabled(false);
    _pauseResumeAction->setEnabled(false);
    _cancelAction->setIcon(QIcon::fromTheme("edit-clear"));
    _cancelAction->setToolTip(i18n("Clear"));

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
                     const QPair<QString,QString> &field2)
{
    _progressBar->setToolTip(QString("%1\n%2: %3\n%4: %5").arg(description,
                                                               field1.first, field1.second,
                                                               field2.first, field2.second));
}

void JobMan::slotSuspended(KJob *)
{
    _pauseResumeAction->setIcon(QIcon::fromTheme("media-playback-start"));
    _pauseResumeAction->setToolTip(i18n("Resume Job"));
}

void JobMan::slotResumed(KJob *)
{
    _pauseResumeAction->setIcon(QIcon::fromTheme("media-playback-pause"));
    _pauseResumeAction->setToolTip(i18n("Pause Job"));
}

void JobMan::setCurrentJob(KJob *job)
{
    _currentJob = job;
    _progressBar->setValue(job->percent());
    _progressBar->setEnabled(true);
     _menuAction->setEnabled(true);
    _pauseResumeAction->setEnabled(true);
    _pauseResumeAction->setToolTip(i18n("Pause Job"));
    if (_currentJob->isSuspended())
        slotSuspended(job);
    else
        slotResumed(job);
    _cancelAction->setEnabled(true);
    _cancelAction->setIcon(QIcon::fromTheme("remove"));
    _cancelAction->setToolTip(i18n("Cancel Job"));

    // KJob has two percent() functions
    connect(job, SIGNAL(percent(KJob *, unsigned long)), this,
            SLOT(slotPercent(KJob *, unsigned long)));
    connect(job, &KJob::description, this, &JobMan::slotDescription);
    connect(job, &KJob::finished, this, &JobMan::slotFinished);
    connect(job, &KJob::suspended, this, &JobMan::slotSuspended);
    connect(job, &KJob::resumed, this, &JobMan::slotResumed);
}
