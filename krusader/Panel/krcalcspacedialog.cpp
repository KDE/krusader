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

// QtCore
#include <QTimer>
// QtWidgets
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <KI18n/KLocalizedString>

#include "../krglobal.h"
#include "../FileSystem/sizecalculator.h"

KrCalcSpaceDialog::KrCalcSpaceDialog(QWidget *parent, SizeCalculator *calculator)
    : QDialog(parent), m_calculator(calculator)
{
    setWindowTitle(i18n("Calculate Occupied Space"));

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_label = new QLabel(this);
    layout->addWidget(m_label);
    layout->addStretch(10);

    // buttons: Cancel is replaced with Ok when calculator is finished
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    layout->addWidget(m_buttonBox);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &KrCalcSpaceDialog::slotCancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    connect(m_calculator, &SizeCalculator::finished, this, &KrCalcSpaceDialog::slotCalculatorFinished);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &KrCalcSpaceDialog::updateResult);
    m_updateTimer->start(1000);

    updateResult();
}

void KrCalcSpaceDialog::showDialog(QWidget *parent, SizeCalculator *calculator)
{
    KrCalcSpaceDialog *dialog = new KrCalcSpaceDialog(parent, calculator);
    dialog->show();
}

void KrCalcSpaceDialog::slotCancel()
{
    m_updateTimer->stop();
    m_calculator->cancel();

    reject(); // close and delete this dialog
}

void KrCalcSpaceDialog::slotCalculatorFinished()
{
    m_buttonBox->clear();
    QPushButton *okButton = m_buttonBox->addButton(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setFocus();

    m_updateTimer->stop();
    updateResult();
}

void KrCalcSpaceDialog::updateResult()
{
    const KIO::filesize_t totalSize = m_calculator->totalSize();
    const unsigned long totalFiles = m_calculator->totalFiles();
    const unsigned long totalDirs = m_calculator->totalDirs();

    const QList<QUrl> urls = m_calculator->urls();
    const QString fileName =
        urls.count() == 1 ? i18n("Name: %1\n", urls.first().fileName()) : QString("");

    QString msg = fileName + i18n("Total occupied space: %1", KIO::convertSize(totalSize));
    if (totalSize >= 1024)
        msg += i18np(" (%2 byte)", " (%2 bytes)", totalSize, QLocale().toString(totalSize));
    msg += '\n';
    msg += i18np("in %1 folder", "in %1 folders", totalDirs);
    msg += ' ';
    msg += i18np("and %1 file", "and %1 files", totalFiles);
    m_label->setText(msg);
}
