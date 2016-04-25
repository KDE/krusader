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
#include <QtCore/QMutexLocker>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KCursor>

#include "krpanel.h"
#include "panelfunc.h"
#include "../krglobal.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krvfshandler.h"

/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */
KrCalcSpaceDialog::CalcThread::CalcThread(QUrl url, const QStringList & items)
        : m_totalSize(0), m_currentSize(0), m_totalFiles(0), m_totalDirs(0), m_items(items), m_url(url),
        m_stop(false) {}


void KrCalcSpaceDialog::CalcThread::getStats(KIO::filesize_t  &totalSize,
                                             unsigned long &totalFiles,
                                             unsigned long &totalDirs) const
{
    QMutexLocker locker(&m_mutex);
    totalSize = m_totalSize + m_currentSize;
    totalFiles = m_totalFiles;
    totalDirs = m_totalDirs;
}

void KrCalcSpaceDialog::CalcThread::updateItems(KrView *view) const
{
    QMutexLocker locker(&m_mutex);
    for (QStringList::const_iterator it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        KrViewItem *viewItem = view->findItemByName(*it);
        if (viewItem) {
            viewItem->setSize(m_sizes[*it]);
            viewItem->redraw();
        }
    }
}

void KrCalcSpaceDialog::CalcThread::run()
{
    if (!m_items.isEmpty()) { // if something to do: do the calculation
        vfs * files = KrVfsHandler::getVfs(m_url);
        if(!files->vfs_refresh(m_url))
            return;
        for (QStringList::ConstIterator it = m_items.begin(); it != m_items.end(); ++it) {
            files->vfs_calcSpace(*it, &m_currentSize, &m_totalFiles, &m_totalDirs , & m_stop);

            if (m_stop)
                break;

            m_mutex.lock();
            m_sizes[*it] = m_currentSize;
            m_totalSize += m_currentSize;
            m_currentSize = 0;
            m_mutex.unlock();
        }
        delete files;
    }
}

void KrCalcSpaceDialog::CalcThread::stop()
{
    // cancel was pressed
    m_stop = true;
}


KrCalcSpaceDialog::KrCalcSpaceDialog(QWidget *parent, KrPanel * panel, const QStringList & items, bool autoclose) :
        QDialog(parent), m_autoClose(autoclose), m_canceled(false),
                m_timerCounter(0), m_items(items), m_view(panel->view)
{
    setWindowTitle(i18n("Calculate Occupied Space"));
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_thread = new CalcThread(panel->virtualPath(), items);
    m_pollTimer = new QTimer(this);
    m_label = new QLabel("", this);
    mainLayout->addWidget(m_label);
    showResult(); // fill m_label with something useful
    mainLayout->addStretch(10);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    // the dialog: The Ok button is hidden until it is needed
    okButton->setVisible(false);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(slotCancel()));
}

void KrCalcSpaceDialog::calculationFinished()
{
    m_thread->updateItems(m_view);
    // close dialog if auto close is true
    if (m_autoClose) {
        done(0);
        return;
    }
    // otherwise hide cancel and show ok button
    cancelButton->setVisible(false);
    okButton->setVisible(true);
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
        m_thread->updateItems(m_view);
    }
}

void KrCalcSpaceDialog::showResult()
{
    if (!m_thread)
        return;
    KIO::filesize_t totalSize;
    unsigned long totalFiles, totalDirs;

    m_thread->getStats(totalSize, totalFiles, totalDirs);

    QString msg;
    QString fileName = (m_items.count() == 1) ? i18n("Name: %1\n", m_items.first()) : QString("");
    msg = fileName + i18n("Total occupied space: %1", KIO::convertSize(totalSize));
    if (totalSize >= 1024)
        msg += i18np(" (%1 byte)", " (%1 bytes)", KRpermHandler::parseSize(totalSize));
    msg += '\n';
    msg += i18np("in %1 folder", "in %1 folders", totalDirs);
    msg += ' ';
    msg += i18np("and %1 file", "and %1 files", totalFiles);
    m_label->setText(msg);
}

void KrCalcSpaceDialog::slotCancel()
{
    m_thread->stop(); // notify the thread to stop
    m_canceled = true; // set the cancel flag
    reject(); // close the dialog
}

KrCalcSpaceDialog::~KrCalcSpaceDialog()
{
    if(m_thread->isFinished())
        delete m_thread;
    else {
        m_thread->stop();
        connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    }
}

int KrCalcSpaceDialog::exec()
{
    m_thread->start(); // start the thread
    if (m_autoClose) { // autoclose
        // set the cursor to busy mode and wait 1 second or until the thread finishes
        krMainWindow->setCursor(Qt::WaitCursor);
        m_thread->wait(1000);
        krMainWindow->setCursor(Qt::ArrowCursor);    // return the cursor to normal mode
        m_thread->updateItems(m_view);
        if(m_thread->isFinished())
            return -1;                                        // thread finished: do not show the dialog
        showResult(); // fill the invisible dialog with useful data
    }
    // prepare and start the poll timer
    connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(timer()));
    m_pollTimer->start(100);
    return QDialog::exec();                                 // show the dialog
}
/* --=={ End of patch by Heiner <h.eichmann@gmx.de> }==-- */

