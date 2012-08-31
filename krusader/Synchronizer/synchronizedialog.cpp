/***************************************************************************
                     synchronizedialog.cpp  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "synchronizedialog.h"
#include "../VFS/krpermhandler.h"
#include "../krglobal.h"
#include "../defaults.h"
#include <QtGui/QLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <klocale.h>

SynchronizeDialog::SynchronizeDialog(QWidget* parent,
                                     Synchronizer *sync, int pleftCopyNr, KIO::filesize_t pleftCopySize,
                                     int prightCopyNr, KIO::filesize_t prightCopySize, int pdeleteNr,
                                     KIO::filesize_t pdeleteSize, int parThreads) : QDialog(parent),
        synchronizer(sync), leftCopyNr(pleftCopyNr),
        leftCopySize(pleftCopySize), rightCopyNr(prightCopyNr),
        rightCopySize(prightCopySize), deleteNr(pdeleteNr),
        deleteSize(pdeleteSize), parallelThreads(parThreads),
        isPause(true), syncStarted(false)
{
    setWindowTitle(i18n("Krusader::Synchronize"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(11, 11, 11, 11);
    layout->setSpacing(6);

    cbRightToLeft = new QCheckBox(i18np("Right to left: Copy 1 file", "Right to left: Copy %1 files", leftCopyNr) + ' ' +
                                  i18np("(1 byte)", "(%1 bytes)", KRpermHandler::parseSize(leftCopySize).trimmed().toInt()),
                                  this);
    cbRightToLeft->setChecked(leftCopyNr != 0);
    cbRightToLeft->setEnabled(leftCopyNr != 0);
    layout->addWidget(cbRightToLeft);

    lbRightToLeft = new QLabel(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", leftCopyNr, 0,
                                     0, KRpermHandler::parseSize(leftCopySize).trimmed()),
                               this);
    lbRightToLeft->setEnabled(leftCopyNr != 0);
    layout->addWidget(lbRightToLeft);

    cbLeftToRight = new QCheckBox(i18np("Left to right: Copy 1 file", "Left to right: Copy %1 files", rightCopyNr) + ' ' +
                                  i18np("(1 byte)", "(%1 bytes)", KRpermHandler::parseSize(rightCopySize).trimmed().toInt()),
                                  this);
    cbLeftToRight->setChecked(rightCopyNr != 0);
    cbLeftToRight->setEnabled(rightCopyNr != 0);
    layout->addWidget(cbLeftToRight);

    lbLeftToRight = new QLabel(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", rightCopyNr, 0,
                                     0, KRpermHandler::parseSize(rightCopySize).trimmed()),
                               this);
    lbLeftToRight->setEnabled(rightCopyNr != 0);
    layout->addWidget(lbLeftToRight);

    cbDeletable = new QCheckBox(i18np("Left: Delete 1 file", "Left: Delete %1 files", deleteNr) + ' ' +
                                i18np("(1 byte)", "(%1 bytes)", KRpermHandler::parseSize(deleteSize).trimmed().toInt()),
                                this);
    cbDeletable->setChecked(deleteNr != 0);
    cbDeletable->setEnabled(deleteNr != 0);
    layout->addWidget(cbDeletable);

    lbDeletable   = new QLabel(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", deleteNr, 0,
                                     0, KRpermHandler::parseSize(deleteSize).trimmed()),
                               this);
    lbDeletable->setEnabled(deleteNr != 0);
    layout->addWidget(lbDeletable);

    progress = new QProgressBar(this);
    progress->setMaximum(1000);
    progress->setMinimum(0);
    progress->setValue(0);
    progress->setMinimumWidth(400);
    layout->addWidget(progress);

    QWidget *hboxWidget = new QWidget(this);
    QHBoxLayout * hbox = new QHBoxLayout(hboxWidget);

    hbox->setSpacing(6);

    cbOverwrite = new QCheckBox(i18n("Confirm overwrites"), this);
    KConfigGroup group(krConfig, "Synchronize");
    cbOverwrite->setChecked(group.readEntry("Confirm overwrites", _ConfirmOverWrites));
    layout->addWidget(cbOverwrite);

    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->addItem(spacer);

    btnStart = new QPushButton(hboxWidget);
    btnStart->setText(i18n("&Start"));
    btnStart->setIcon(KIcon("media-playback-start"));
    hbox->addWidget(btnStart);

    btnPause = new QPushButton(hboxWidget);
    btnPause->setEnabled(false);
    btnPause->setText(i18n("&Pause"));
    btnPause->setIcon(KIcon("media-playback-pause"));
    hbox->addWidget(btnPause);

    QPushButton *btnClose = new QPushButton(hboxWidget);
    btnClose->setText(i18n("&Close"));
    btnClose->setIcon(KIcon("dialog-close"));
    hbox->addWidget(btnClose);

    layout->addWidget(hboxWidget);

    connect(btnStart,  SIGNAL(clicked()), this, SLOT(startSynchronization()));
    connect(btnPause,  SIGNAL(clicked()), this, SLOT(pauseOrResume()));
    connect(btnClose,  SIGNAL(clicked()), this, SLOT(reject()));

    exec();
}

SynchronizeDialog::~SynchronizeDialog()
{
    KConfigGroup group(krConfig, "Synchronize");
    group.writeEntry("Confirm overwrites", cbOverwrite->isChecked());
}

void SynchronizeDialog::startSynchronization()
{
    btnStart->setEnabled(false);
    btnPause->setEnabled(syncStarted = true);
    connect(synchronizer,  SIGNAL(synchronizationFinished()), this, SLOT(synchronizationFinished()));
    connect(synchronizer,  SIGNAL(processedSizes(int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t)),
            this, SLOT(processedSizes(int, KIO::filesize_t, int, KIO::filesize_t, int, KIO::filesize_t)));
    connect(synchronizer,  SIGNAL(pauseAccepted()), this, SLOT(pauseAccepted()));

    if (!cbRightToLeft->isChecked()) leftCopySize = 0;
    if (!cbLeftToRight->isChecked()) rightCopySize = 0;
    if (!cbDeletable->isChecked())   deleteSize = 0;

    synchronizer->synchronize(this, cbRightToLeft->isChecked(), cbLeftToRight->isChecked(),
                              cbDeletable->isChecked(), !cbOverwrite->isChecked(), parallelThreads);
}

void SynchronizeDialog::synchronizationFinished()
{
    QDialog::reject();
}

void SynchronizeDialog::processedSizes(int leftNr, KIO::filesize_t leftSize, int rightNr,
                                       KIO::filesize_t rightSize, int delNr, KIO::filesize_t delSize)
{
    lbRightToLeft->setText(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", leftCopyNr, leftNr,
                                KRpermHandler::parseSize(leftSize).trimmed(),
                                KRpermHandler::parseSize(leftCopySize).trimmed()));
    lbLeftToRight->setText(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", rightCopyNr, rightNr,
                                KRpermHandler::parseSize(rightSize).trimmed(),
                                KRpermHandler::parseSize(rightCopySize).trimmed()));
    lbDeletable->setText(i18np("\tReady: %2/1 file, %3/%4", "\tReady: %2/%1 files, %3/%4", deleteNr, delNr,
                              KRpermHandler::parseSize(delSize).trimmed(),
                              KRpermHandler::parseSize(deleteSize).trimmed()));

    KIO::filesize_t totalSum      = leftCopySize + rightCopySize + deleteSize;
    KIO::filesize_t processedSum  = leftSize + rightSize + delSize;

    if (totalSum == 0)
        totalSum++;

    progress->setValue((int)(((double)processedSum / (double)totalSum)*1000));
}

void SynchronizeDialog::pauseOrResume()
{
    if (isPause) {
        btnPause->setEnabled(false);
        synchronizer->pause();
    } else {
        btnPause->setText(i18n("Pause"));
        synchronizer->resume();
        isPause = true;
    }
}

void SynchronizeDialog::pauseAccepted()
{
    btnPause->setText(i18n("Resume"));
    btnPause->setEnabled(true);
    isPause = false;
}

#include "synchronizedialog.moc"
