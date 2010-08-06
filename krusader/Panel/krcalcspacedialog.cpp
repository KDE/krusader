/***************************************************************************
                          krcalcspacedialog.cpp  -  description
                             -------------------
    begin                : Fri Jan 2 2004
    copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
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

#include "krcalcspacedialog.h"

#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QVBoxLayout>

#include <klocale.h>
#include <kcursor.h>

#include "krpanel.h"
#include "panelfunc.h"
#include "../krglobal.h"
#include "../VFS/krpermhandler.h"

/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */
KrCalcSpaceDialog::CalcThread::CalcThread(KrCalcSpaceDialog * parent, KrPanel * panel, const QStringList & items)
        : m_totalSize(0), m_currentSize(0), m_totalFiles(0), m_totalDirs(0), m_items(items), m_files(panel->func->files()),
        m_view(panel->view), m_parent(parent), m_threadInUse(true), m_stop(false) {}

void KrCalcSpaceDialog::CalcThread::cleanUp()
{
    if (m_threadInUse || !isFinished())
        m_synchronizeUsageAccess.unlock();
    else {
        m_synchronizeUsageAccess.unlock(); // prevents a resource leak
        // otherwise: no one needs this instance any more: delete it
        delete this;
    }
}

void KrCalcSpaceDialog::CalcThread::deleteInstance()
{
    // synchronize to avoid race condition.
    m_synchronizeUsageAccess.lock();
    m_threadInUse = false;
    cleanUp();
}

void KrCalcSpaceDialog::CalcThread::run()
{
    if (!m_items.isEmpty())   // if something to do: do the calculation
        for (QStringList::ConstIterator it = m_items.begin(); it != m_items.end(); ++it) {
            m_currentSize = 0;
            m_files->vfs_calcSpace(*it, &m_currentSize, &m_totalFiles, &m_totalDirs , & m_stop);
            if (m_stop)
                break;
            KrViewItem * viewItem = m_view->findItemByName(*it);
            if (viewItem) {
                viewItem->setSize(m_currentSize);
            }
            m_totalSize += m_currentSize;
            m_currentSize = 0;
        }
    // synchronize to avoid race condition.
    m_synchronizeUsageAccess.lock();
    cleanUp(); // this does not need the instance any more
}

void KrCalcSpaceDialog::CalcThread::stop()
{
    // cancel was pressed
    m_stop = true;
}

KrCalcSpaceDialog::KrCalcSpaceDialog(QWidget *parent, KrPanel * files, const QStringList & items, bool autoclose) :
        KDialog(parent), m_autoClose(autoclose), m_canceled(false), m_timerCounter(0)
{
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setWindowTitle(i18n("Calculate Occupied Space"));
    setWindowModality(Qt::WindowModal);
    // the dialog: The Ok button is hidden until it is needed
    showButton(KDialog::Ok, false);
    m_thread = new CalcThread(this, files, items);
    m_pollTimer = new QTimer(this);
    QWidget * mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    QVBoxLayout *topLayout = new QVBoxLayout(mainWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(spacingHint());

    m_label = new QLabel("", mainWidget);
    showResult(); // fill m_label with something useful
    topLayout->addWidget(m_label);
    topLayout->addStretch(10);

    connect(this, SIGNAL(cancelClicked()), this, SLOT(reject()));
}

void KrCalcSpaceDialog::calculationFinished()
{
    // close dialog if auto close is true
    if (m_autoClose) {
        done(0);
        return;
    }
    // otherwise hide cancel and show ok button
    showButton(KDialog::Cancel, false);
    showButton(KDialog::Ok, true);
    showResult(); // and show final result
}

/* This timer has two jobs: it polls the thread if it is finished. Polling is
  better here as it might finish while the dialog builds up. Secondly it refreshes
  the displayed result.
 */
void KrCalcSpaceDialog::timer()
{
    // thread finished?
    if (m_thread->isFinished()) {
        // close dialog or switch buttons
        calculationFinished();
        m_pollTimer->stop(); // stop the polling. No longer needed
        return;
    }

    // Every 10 pollings (1 second) refresh the displayed result
    if (++m_timerCounter > 10) {
        m_timerCounter = 0;
        showResult();
    }
}

void KrCalcSpaceDialog::showResult()
{
    if (!m_thread) return;
    QString msg;
    QString fileName = ((m_thread->getItems().count() == 1) ? (i18n("Name: %1\n", m_thread->getItems().first())) : QString(""));
    msg = fileName + i18n("Total occupied space: %1", KIO::convertSize(m_thread->getTotalSize()));
    if (m_thread->getTotalSize() >= 1024)
        msg += i18n(" (bytes)", KRpermHandler::parseSize(m_thread->getTotalSize()));
    msg += '\n';
    msg += i18np("in %1 directory", "in %1 directories", m_thread->getTotalDirs());
    msg += ' ';
    msg += i18np("and %1 file", "and %1 files", m_thread->getTotalFiles());
    m_label->setText(msg);
}

void KrCalcSpaceDialog::slotCancel()
{
    m_thread->stop(); // notify the thread to stop
    m_canceled = true; // set the cancel flag
    KDialog::reject(); // close the dialog
}

KrCalcSpaceDialog::~KrCalcSpaceDialog()
{
    CalcThread * tmp = m_thread;
    m_thread = 0; // do not access the thread anymore or core dump if smoe piece of code wrongly does
    tmp->deleteInstance(); // Notify the thread, that the dialog does not need anymore.
}

void KrCalcSpaceDialog::exec()
{
    m_thread->start(); // start the thread
    if (m_autoClose) { // autoclose
        // set the cursor to busy mode and wait 3 seconds or until the thread finishes
        krMainWindow->setCursor(Qt::WaitCursor);
        bool result = m_thread->wait(3000);
        krMainWindow->setCursor(Qt::ArrowCursor);    // return the cursor to normal mode
        if (result) return;// thread finished: do not show the dialog
        showResult(); // fill the invisible dialog with useful data
    }
    // prepare and start the poll timer
    connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(timer()));
    m_pollTimer->start(100);
    KDialog::exec(); // show the dialog
}
/* --=={ End of patch by Heiner <h.eichmann@gmx.de> }==-- */


#include "krcalcspacedialog.moc"
