/*****************************************************************************
 * Copyright (C) 2008-2009 Csaba Karai <cskarai@freemail.hu>                 *
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

#include "queuedialog.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QToolButton>

#include <KConfigCore/KSharedConfig>
#include <KConfigGui/KStandardShortcut>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#include "queuewidget.h"
#include "queue_mgr.h"
#include "../krglobal.h"

class KrTimeDialog : public QDialog
{
public:
    KrTimeDialog(QWidget * parent = 0) : QDialog(parent) {
        setWindowTitle(i18n("Krusader::Queue Manager"));
        setWindowModality(Qt::WindowModal);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        setLayout(mainLayout);

        mainLayout->addWidget(new QLabel(i18n("Please enter the time to start processing the queue:")));

        QTime time = QTime::currentTime();
        time = time.addSecs(60 * 60);   // add 1 hour
        _timeEdit = new QTimeEdit(time, this);
        _timeEdit->setDisplayFormat("hh:mm:ss");
        mainLayout->addWidget(_timeEdit);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        mainLayout->addWidget(buttonBox);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

        connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

        _timeEdit->setFocus();
    }
    QTime getStartTime() {
        return _timeEdit->time();
    }

private:
    QTimeEdit * _timeEdit;
};

QueueDialog * QueueDialog::_queueDialog = 0;

QueueDialog::QueueDialog() : QDialog(), _autoHide(true)
{
    setWindowModality(Qt::NonModal);
    setWindowTitle(i18n("Krusader::Queue Manager"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout * hbox2 = new QHBoxLayout;

    QAction *act = new QAction(this);
    act->setIcon(QIcon::fromTheme("tab-new"));
    act->setToolTip(i18n("Create a new queue"));
    act->setShortcuts(QKeySequence::keyBindings(QKeySequence::AddTab));
    connect(act, SIGNAL(triggered(bool)), this, SLOT(slotNewTab()));
    _newTabButton = new QToolButton(this);
    _newTabButton->setDefaultAction(act);
    hbox2->addWidget(_newTabButton);

    _closeTabButton = new QToolButton(this);
    _closeTabButton->setIcon(QIcon::fromTheme("tab-close"));
    _closeTabButton->setToolTip(i18n("Remove the current queue"));
    _closeTabButton->setShortcut(QKeySequence::Close);
    connect(_closeTabButton, SIGNAL(clicked()), this, SLOT(slotDeleteCurrentTab()));
    hbox2->addWidget(_closeTabButton);

    _pauseButton = new QToolButton(this);
    _pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    _pauseButton->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(_pauseButton, SIGNAL(clicked()), this, SLOT(slotPauseClicked()));
    hbox2->addWidget(_pauseButton);

    _progressBar = new QProgressBar(this);
    _progressBar->setMinimum(0);
    _progressBar->setMaximum(100);
    _progressBar->setValue(0);
    _progressBar->setFormat(i18n("unused"));
    _progressBar->setTextVisible(true);
    hbox2->addWidget(_progressBar);

    _scheduleButton = new QToolButton(this);
    _scheduleButton->setIcon(QIcon::fromTheme("chronometer"));
    _scheduleButton->setToolTip(i18n("Schedule queue starting (Ctrl+S)"));
    _scheduleButton->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(_scheduleButton, SIGNAL(clicked()), this, SLOT(slotScheduleClicked()));
    hbox2->addWidget(_scheduleButton);

    mainLayout->addLayout(hbox2);

    _queueWidget = new QueueWidget(this);
    _queueWidget->setDocumentMode(true);
    connect(_queueWidget, SIGNAL(currentChanged()), this, SLOT(slotUpdateToolbar()));
    mainLayout->addWidget(_queueWidget);

    _statusLabel = new QLabel(this);
    QSizePolicy statuspolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    statuspolicy.setHeightForWidth(_statusLabel->sizePolicy().hasHeightForWidth());
    _statusLabel->setSizePolicy(statuspolicy);
    mainLayout->addWidget(_statusLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    KConfigGroup group(krConfig, "QueueManager");
    int sizeX = group.readEntry("Window Width",  -1);
    int sizeY = group.readEntry("Window Height",  -1);
    int x = group.readEntry("Window X",  -1);
    int y = group.readEntry("Window Y",  -1);

    if (sizeX != -1 && sizeY != -1)
        resize(sizeX, sizeY);
    else
        resize(300, 400);

    if (x != -1 && y != -1)
        move(x, y);
    else
        move(20, 20);

    slotUpdateToolbar();

    QAction *nextTabAction = new QAction(this);
    nextTabAction->setShortcuts(KStandardShortcut::tabNext());
    addAction(nextTabAction);
    connect(nextTabAction, SIGNAL(triggered()), SLOT(slotNextTab()));

    QAction *prevTabAction = new QAction(this);
    prevTabAction->setShortcuts(KStandardShortcut::tabPrev());
    addAction(prevTabAction);
    connect(prevTabAction, SIGNAL(triggered()), SLOT(slotPrevTab()));

    QAction *deleteQueueAction = new QAction(this);
    deleteQueueAction->setShortcut(Qt::Key_Delete);
    addAction(deleteQueueAction);
    connect(deleteQueueAction, SIGNAL(triggered()), _queueWidget, SLOT(deleteCurrent()));

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(QueueManager::instance(), SIGNAL(queueInserted(Queue *)), this, SLOT(slotUpdateToolbar()));
    connect(QueueManager::instance(), SIGNAL(percent(Queue *, int)), this, SLOT(slotPercentChanged(Queue *, int)));

    show();
    _queueWidget->currentWidget()->setFocus();

    _queueDialog = this;
}

QueueDialog::~QueueDialog()
{
    if (_queueDialog)
        saveSettings();
    _queueDialog = 0;
}

void QueueDialog::showDialog(bool autoHide)
{
    if (_queueDialog == 0)
        _queueDialog = new QueueDialog();
    else {
        _queueDialog->show();
        if (!autoHide) {
            _queueDialog->raise();
            _queueDialog->activateWindow();
        }
    }
    if (!autoHide)
        _queueDialog->_autoHide = false;
}

void QueueDialog::everyQueueIsEmpty()
{
    if (_queueDialog->_autoHide && _queueDialog != 0 && !_queueDialog->isHidden())
        _queueDialog->reject();
}

void QueueDialog::reject()
{
    _autoHide = true;
    saveSettings();
    QDialog::reject();
}

void QueueDialog::saveSettings()
{
    KConfigGroup group(krConfig, "QueueManager");
    group.writeEntry("Window Width", width());
    group.writeEntry("Window Height", height());
    group.writeEntry("Window X", x());
    group.writeEntry("Window Y", y());
}

void QueueDialog::deleteDialog()
{
    if (_queueDialog)
        delete _queueDialog;
}

void QueueDialog::slotUpdateToolbar()
{
    Queue * currentQueue = QueueManager::currentQueue();
    if (currentQueue) {
        if (currentQueue->isSuspended()) {
            _pauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            _pauseButton->setToolTip(i18n("Start processing the queue (Ctrl+P)"));
            QTime time = currentQueue->scheduleTime();
            if (time.isNull()) {
                _statusLabel->setText(i18n("The queue is paused."));
            } else {
                _statusLabel->setText(i18n("Scheduled to start at %1.",
                                           time.toString("hh:mm:ss")));
            }
        } else {
            _statusLabel->setText(i18n("The queue is running."));
            _pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            _pauseButton->setToolTip(i18n("Pause processing the queue (Ctrl+P)"));
        }

        _closeTabButton->setEnabled(_queueWidget->count() > 1);

        slotPercentChanged(currentQueue, currentQueue->getPercent());
    }
}

void QueueDialog::slotPauseClicked()
{
    Queue * currentQueue = QueueManager::currentQueue();
    if (currentQueue) {
        if (currentQueue->isSuspended())
            currentQueue->resume();
        else
            currentQueue->suspend();
    }
}

void QueueDialog::slotScheduleClicked()
{
    QPointer<KrTimeDialog> dialog = new KrTimeDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        QTime startTime = dialog->getStartTime();
        Queue * queue = QueueManager::currentQueue();
        queue->schedule(startTime);
    }
    delete dialog;
}

void QueueDialog::slotNewTab()
{
    bool ok = false;
    QString queueName = QInputDialog::getText(this, i18n("Krusader::Queue Manager"),
                                              i18n("Please enter the name of the new queue"),
                                              QLineEdit::Normal, QString(), &ok);

    if (!ok || queueName.isEmpty())
        return;

    Queue * queue = QueueManager::createQueue(queueName);
    if (queue == 0) {
        KMessageBox::error(this, i18n("A queue already exists with this name."));
    }
}

void QueueDialog::slotNextTab()
{
    int curr = _queueWidget->currentIndex();
    int count = _queueWidget->count();
    _queueWidget->setCurrentIndex((curr + 1) % count);
}

void QueueDialog::slotPrevTab()
{
    int curr = _queueWidget->currentIndex();
    int count = _queueWidget->count();
    _queueWidget->setCurrentIndex((curr + count - 1) % count);
}

void QueueDialog::slotDeleteCurrentTab()
{
    Queue * currentQueue = QueueManager::currentQueue();
    if (currentQueue) {
        if (currentQueue->count() != 0) {
            if (KMessageBox::warningContinueCancel(this,
                                                   i18n("Deleting the queue requires aborting all pending jobs. Do you wish to continue?"),
                                                   i18n("Warning")) != KMessageBox::Continue) return;
        }
        QueueManager::removeQueue(currentQueue);
    }
}

void QueueDialog::slotPercentChanged(Queue * queue, int percent)
{
    if (QueueManager::currentQueue() == queue) {
        switch (percent) {
        case PERCENT_UNUSED:
            _progressBar->setMaximum(100);
            _progressBar->setValue(0);
            _progressBar->setFormat(i18n("unused"));
            break;
        case PERCENT_UNKNOWN:
            if (_progressBar->maximum() != 0) {
                _progressBar->setMaximum(0);
                _progressBar->setFormat("");
            }
            break;
        default:
            if (_progressBar->maximum() != 100)
                _progressBar->setMaximum(100);
            _progressBar->setValue(percent);
            _progressBar->setFormat("%p%");
            break;
        }
    }
}

