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
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <KI18n/KLocalizedString>
#include <KIOWidgets/KIO/FileUndoManager>

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
        updatePauseResumeButton();
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
        connect(job, &KJob::suspended, this, &JobMenuAction::updatePauseResumeButton);
        connect(job, &KJob::resumed, this, &JobMenuAction::updatePauseResumeButton);
        connect(job, &KJob::result, this, &JobMenuAction::slotResult);
        connect(job, &KJob::finished, this, &JobMenuAction::slotFinished);
        connect(job, &KJob::warning, this,
                [](KJob *, const QString &plain, const QString &) {
                    krOut << "unexpected job warning: " << plain;
                });
    }

protected slots:
    void slotDescription(KJob *, const QString &description,
                       const QPair<QString, QString> &field1,
                       const QPair<QString, QString> &field2)
    {
        const QPair<QString, QString> textField = !field2.first.isEmpty() ? field2 : field1;
        QString text = description;
        if (!textField.first.isEmpty()) {
            text += QString(" - %1: %2").arg(textField.first, textField.second);
        }
        _description->setText(text);

        if (!field2.first.isEmpty() && !field1.first.isEmpty()) {
            // NOTE: tooltips for QAction items in menu are not shown
            _progressBar->setToolTip(QString("%1: %2").arg(field1.first, field1.second));
        }
    }

    void slotPercent(KJob *, unsigned long percent) { _progressBar->setValue(percent); }

    void updatePauseResumeButton()
    {
        _pauseResumeButton->setIcon(QIcon::fromTheme(_job->isSuspended() ? "media-playback-start" :
                                                                           "media-playback-pause"));
    }

    void slotResult(KJob *job)
    {
        // NOTE: _job may already set to 0 now
        if(!job->error()) {
            // percent signal is not reliable, set manually
            _progressBar->setValue(100);
        }
    }

    void slotFinished()
    {
        _pauseResumeButton->setEnabled(false);
        _cancelButton->setIcon(QIcon::fromTheme("edit-clear"));
        _cancelButton->setToolTip(i18n("Clear"));

        _job = 0;
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

JobMan::JobMan(QObject *parent) : QObject(parent), _currentMode(MODE_QUEUEING)
{
    // job control action
    _controlAction = new KToolBarPopupAction(QIcon::fromTheme("media-playback-pause"),
                                          i18n("Play/Pause &Job"), this);
    _controlAction->setEnabled(false);
    connect(_controlAction, &QAction::triggered, this, &JobMan::slotControlActionTriggered);

    QMenu *menu = new QMenu(krMainWindow);
    menu->setMinimumWidth(300);
    // make scrollable if menu is too long
    menu->setStyleSheet("QMenu { menu-scrollable: 1; }");
    _controlAction->setMenu(menu);

    // progress bar action
    _progressBar = new QProgressBar();
    _progressBar->setToolTip(sDefaultToolTip);
    _progressBar->setEnabled(false);
    // listen to clicks on progress bar
    _progressBar->installEventFilter(this);

    QWidgetAction *progressAction = new QWidgetAction(krMainWindow);
    progressAction->setText(i18n("Job Progress Bar"));
    progressAction->setDefaultWidget(_progressBar);
    _progressAction = progressAction;

    // job mode action
    QComboBox *modeBox = new QComboBox(krMainWindow);
    modeBox->addItem(QIcon::fromTheme("media-playlist-repeat"), i18n("Queue"));
    modeBox->setItemData(0, i18n("Run one job simultaneously"), Qt::ToolTipRole);
    modeBox->addItem(QIcon::fromTheme("chronometer-pause"), i18n("Pause"));
    modeBox->setItemData(1, i18n("Added jobs are paused"), Qt::ToolTipRole);
    modeBox->addItem(QIcon::fromTheme("media-playlist-shuffle"), i18n("Unmanaged"));
    modeBox->setItemData(2, i18n("Run jobs at the same time"), Qt::ToolTipRole);
    modeBox->setCurrentIndex(_currentMode);
    modeBox->setToolTip(i18n("Change job manager mode"));
    connect(modeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotModeChange(int)));

    QWidgetAction *modeAction = new QWidgetAction(krMainWindow);
    modeAction->setText(i18n("Job Manager Mode"));
    modeAction->setDefaultWidget(modeBox);
    _modeAction = modeAction;

    // undo action
    KIO::FileUndoManager *undoManager = KIO::FileUndoManager::self();
    undoManager->uiInterface()->setParentWidget(krMainWindow);

    _undoAction = new QAction(QIcon::fromTheme("edit-undo"), i18n("Undo Last Job"));
    _undoAction->setEnabled(false);
    connect(_undoAction, &QAction::triggered, undoManager, &KIO::FileUndoManager::undo);
    connect(undoManager, static_cast<void(KIO::FileUndoManager::*)(bool)>(&KIO::FileUndoManager::undoAvailable),
            _undoAction, &QAction::setEnabled);
    connect(undoManager, &KIO::FileUndoManager::undoTextChanged, this, &JobMan::slotUndoTextChange);
}

void JobMan::manageJob(KJob *job)
{

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

    switch(_currentMode) {
        case MODE_QUEUEING:
            if (jobsAreRunning())
                job->suspend();
            break;
        case MODE_PAUSED:
            job->suspend();
            break;
        case MODE_UNMANAGED:
            break;
    }

    _jobs.append(job);

    updateUI();
}

// #### protected slots

void JobMan::slotControlActionTriggered()
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

void JobMan::slotModeChange(int index)
{
    _currentMode = static_cast<JobMode>(index);
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
    // Note: by default KJobs are deleted after finished
    _jobs.removeAll(job);

    if (_currentMode == MODE_QUEUEING && !_jobs.isEmpty() && !jobsAreRunning()) {
        // start next job
        _jobs.first()->resume();
    }

    updateUI();
}

void JobMan::slotUpdateControlAction()
{
    _controlAction->setEnabled(!_controlAction->menu()->isEmpty());
}

void JobMan::slotUndoTextChange(const QString &text)
{
    _undoAction->setToolTip(KIO::FileUndoManager::self()->undoAvailable() ? text :
                                                                            i18n("Undo Last Job"));
}

// #### private

void JobMan::updateUI()
{
    int totalPercent = 0;
    for (KJob *job: _jobs) {
        totalPercent += job->percent();
    }
    const bool hasJobs = !_jobs.isEmpty();
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

    const bool running = jobsAreRunning();
    _controlAction->setIcon(QIcon::fromTheme(
        !hasJobs ? "edit-clear" : running ? "media-playback-pause" : "media-playback-start"));
    _controlAction->setToolTip(!hasJobs ? i18n("Clear list") : running ? i18n("Pause All Jobs") :
                                                                         i18n("Resume All Jobs"));
}

bool JobMan::jobsAreRunning()
{
    for (KJob *job: _jobs)
        if (!job->isSuspended())
            return true;

    return false;
}
