/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlCompletion>
#include <KCompletion/KLineEdit>
#include <KIOWidgets/KUrlRequester>
#include <KIOCore/KRecentDocument>
#include <KWidgetsAddons/KGuiItem>

#include "../krglobal.h"
#include "../FileSystem/filesystem.h"
#include "../defaults.h"
#include "../JobMan/jobman.h"


QUrl KChooseDir::getFile(const QString &text, const QUrl& url, const QUrl& cwd)
{
    return get(text, url, cwd, KFile::File);
}

QUrl KChooseDir::getDir(const QString &text, const QUrl& url, const QUrl& cwd)
{
    return get(text, url, cwd, KFile::Directory);
}

QUrl KChooseDir::get(const QString &text, const QUrl &url, const QUrl &cwd, KFile::Modes mode)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(FileSystem::ensureTrailingSlash(url), text, krMainWindow));
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

KChooseDir::ChooseResult KChooseDir::getCopyDir(const QString &text, const QUrl &url,
                                                const QUrl &cwd)
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

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy(const QUrl &urlName, const QString &_text,
                                                 QWidget *parent, bool modal)
    : QDialog(parent)
{
    setWindowModality(modal ? Qt::WindowModal : Qt::NonModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addWidget(new QLabel(_text));

    urlRequester_ = new KUrlRequester(urlName, this);
    urlRequester_->setMinimumWidth(urlRequester_->sizeHint().width() * 3);
    mainLayout->addWidget(urlRequester_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    QPushButton *queueButton = new QPushButton(
        krJobMan->isQueueModeEnabled() ? i18n("F2 Delay Job Start") : i18n("F2 Queue"), this);
    queueButton->setToolTip(krJobMan->isQueueModeEnabled() ?
            i18n("Do not start the job now.") :
            i18n("Enqueue the job if another job is running. Otherwise start immediately."));
    buttonBox->addButton(queueButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(queueButton, SIGNAL(clicked()), SLOT(slotQueueButtonClicked()));
    connect(urlRequester_, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)));

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

void KUrlRequesterDlgForCopy::slotTextChanged(const QString & text)
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

KUrlRequester * KUrlRequesterDlgForCopy::urlRequester()
{
    return urlRequester_;
}

KRGetDate::KRGetDate(QDate date, QWidget *parent) : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
{
    setWindowModality(Qt::WindowModal);
    dateWidget = new KDatePicker(this);
    dateWidget->setDate(date);
    dateWidget->resize(dateWidget->sizeHint());
    setMinimumSize(dateWidget->sizeHint());
    setMaximumSize(dateWidget->sizeHint());
    resize(minimumSize());
    connect(dateWidget, SIGNAL(dateSelected(QDate)), this, SLOT(setDate(QDate)));
    connect(dateWidget, SIGNAL(dateEntered(QDate)), this, SLOT(setDate(QDate)));

    // keep the original date - incase ESC is pressed
    originalDate  = date;
}

QDate KRGetDate::getDate()
{
    if (exec() == QDialog::Rejected) chosenDate = QDate();
    hide();
    return chosenDate;
}

void KRGetDate::setDate(QDate date)
{
    chosenDate = date;
    accept();
}

