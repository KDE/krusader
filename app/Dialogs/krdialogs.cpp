/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krdialogs.h"

// QtCore
#include <QDebug>
#include <QDir>
// QtGui
#include <QFontMetrics>
#include <QKeyEvent>
// QtWidgets
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include <KGuiItem>
#include <KLineEdit>
#include <KLocalizedString>
#include <KRecentDocument>
#include <KUrlCompletion>
#include <KUrlRequester>

#include "../FileSystem/filesystem.h"
#include "../JobMan/jobman.h"
#include "../defaults.h"
#include "../krglobal.h"

QUrl KChooseDir::getFile(const QString &text, const QUrl &url, const QUrl &cwd)
{
    return get(text, url, cwd, KFile::File);
}

QUrl KChooseDir::getDir(const QString &text, const QUrl &url, const QUrl &cwd)
{
    return get(text, url, cwd, KFile::Directory);
}

QUrl KChooseDir::get(const QString &text, const QUrl &url, const QUrl &cwd, KFile::Modes mode)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(url, text, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(mode);
    dlg->exec();
    QUrl u = dlg->selectedUrl(); // empty if cancelled
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    return u;
}

KChooseDir::ChooseResult KChooseDir::getCopyDir(const QString &text, const QUrl &url, const QUrl &cwd)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(url, text, krMainWindow, true));

    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }

    ChooseResult result;
    result.url = u;
    result.enqueue = dlg->isQueued();
    return result;
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy(const QUrl &urlName, const QString &_text, QWidget *parent, bool modal)
    : QDialog(parent)
{
    setWindowModality(modal ? Qt::WindowModal : Qt::NonModal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QLabel *label = new QLabel;
    label->setWordWrap(true);
    label->setText(_text);
    mainLayout->addWidget(label);

    urlRequester_ = new KUrlRequester(urlName, this);
    urlRequester_->setMinimumWidth(urlRequester_->sizeHint().width() * 3);
    mainLayout->addWidget(urlRequester_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    QPushButton *queueButton = new QPushButton(krJobMan->isQueueModeEnabled() ? i18n("F2 Delay Job Start") : i18n("F2 Queue"), this);
    queueButton->setToolTip(krJobMan->isQueueModeEnabled() ? i18n("Do not start the job now.")
                                                           : i18n("Enqueue the job if another job is running. Otherwise start immediately."));
    buttonBox->addButton(queueButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &KUrlRequesterDlgForCopy::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KUrlRequesterDlgForCopy::reject);
    connect(queueButton, &QPushButton::clicked, this, &KUrlRequesterDlgForCopy::slotQueueButtonClicked);
    connect(urlRequester_, &KUrlRequester::textChanged, this, &KUrlRequesterDlgForCopy::slotTextChanged);

    urlRequester_->setFocus();
    bool state = !urlName.isEmpty();
    okButton->setEnabled(state);
}

void KUrlRequesterDlgForCopy::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_F2:
        slotQueueButtonClicked();
        return;
    default:
        QDialog::keyPressEvent(e);
    }
}

void KUrlRequesterDlgForCopy::slotQueueButtonClicked()
{
    queueStart = true;
    accept();
}

void KUrlRequesterDlgForCopy::slotTextChanged(const QString &text)
{
    bool state = !text.trimmed().isEmpty();
    okButton->setEnabled(state);
}

QUrl KUrlRequesterDlgForCopy::selectedURL() const
{
    if (result() == QDialog::Accepted) {
        QUrl url = urlRequester_->url();
        qDebug() << "requester returned URL=" << url.toDisplayString();
        if (url.isValid())
            KRecentDocument::add(url);
        return url;
    } else
        return QUrl();
}

KUrlRequester *KUrlRequesterDlgForCopy::urlRequester()
{
    return urlRequester_;
}

KrGetDate::KrGetDate(QDate date, QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
{
    setWindowModality(Qt::WindowModal);
    dateWidget = new KDatePicker(this);
    dateWidget->setDate(date);
    dateWidget->resize(dateWidget->sizeHint());
    setMinimumSize(dateWidget->sizeHint());
    setMaximumSize(dateWidget->sizeHint());
    resize(minimumSize());
    connect(dateWidget, &KDatePicker::dateSelected, this, &KrGetDate::setDate);
    connect(dateWidget, &KDatePicker::dateEntered, this, &KrGetDate::setDate);

    // keep the original date - incase ESC is pressed
    originalDate = date;
}

QDate KrGetDate::getDate()
{
    if (exec() == QDialog::Rejected)
        chosenDate = QDate();
    hide();
    return chosenDate;
}

void KrGetDate::setDate(QDate date)
{
    chosenDate = date;
    accept();
}
