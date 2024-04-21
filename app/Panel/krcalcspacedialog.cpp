/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krcalcspacedialog.h"

// QtCore
#include <QTimer>
// QtWidgets
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
// QtGui
#include <QKeyEvent>

#include <KLocalizedString>

#include "../FileSystem/sizecalculator.h"
#include "../krglobal.h"

KrCalcSpaceDialog::KrCalcSpaceDialog(QWidget *parent, SizeCalculator *calculator)
    : QDialog(parent)
    , m_calculator(calculator)
{
    setWindowTitle(i18n("Calculate Occupied Space"));

    auto *layout = new QVBoxLayout;
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

void KrCalcSpaceDialog::closeEvent(QCloseEvent *event)
{
    if (event) {
        if (m_updateTimer->isActive())
            slotCancel();
        else
            accept();
    }
}

void KrCalcSpaceDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        if (m_updateTimer->isActive())
            slotCancel();
        else
            reject();
    }
}

void KrCalcSpaceDialog::showDialog(QWidget *parent, SizeCalculator *calculator)
{
    auto *dialog = new KrCalcSpaceDialog(parent, calculator);
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
    const QString fileName = urls.count() == 1 ? i18n("Name: %1\n", urls.first().fileName()) : QString("");

    QString msg = fileName + i18n("Total occupied space: %1", KIO::convertSize(totalSize));
    if (totalSize >= 1024)
        msg += i18np(" (%2 byte)", " (%2 bytes)", totalSize, QLocale().toString(totalSize));
    msg += '\n';
    msg += i18np("in %1 folder", "in %1 folders", totalDirs);
    msg += ' ';
    msg += i18np("and %1 file", "and %1 files", totalFiles);
    m_label->setText(msg);
}
