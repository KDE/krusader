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

#include <QtCore/QDir>
#include <QtGui/QFontMetrics>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>

// TODO KF5 - these headers are from deprecated KDE4LibsSupport : remove them
#include <KDE/KLocale>
#include <KDE/KStandardGuiItem>
#include <kdeversion.h>

#include <KIOWidgets/KUrlCompletion>
#include <KCompletion/KLineEdit>
#include <KIOWidgets/KUrlRequester>
#include <KIOCore/KRecentDocument>

#include "../krglobal.h"
#include "../VFS/vfs.h"
#include "../defaults.h"


QUrl KChooseDir::getFile(QString text, const QUrl& url, const QUrl& cwd)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(vfs::ensureTrailingSlash(url), text, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::File);
    dlg->exec();
    QUrl u = dlg->selectedUrl(); // empty if cancelled
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    return u;
}


QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(vfs::ensureTrailingSlash(url), text, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedUrl();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, false, krMainWindow));
    dlg->hidePreserveAttrs();
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    queue = dlg->enqueue();
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue, bool &preserveAttrs)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, preserveAttrs, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    preserveAttrs = dlg->preserveAttrs();
    queue = dlg->enqueue();
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue, bool &preserveAttrs, QUrl &baseURL)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, preserveAttrs, krMainWindow,
                                                                            true, baseURL));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    if (dlg->copyDirStructure()) {
        baseURL = dlg->baseURL();
    } else {
        baseURL = QUrl();
    }
    preserveAttrs = dlg->preserveAttrs();
    queue = dlg->enqueue();
    return u;
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy(const QUrl &urlName, const QString& _text, bool /*presAttrs*/, QWidget *parent,
        bool modal, QUrl baseURL)
        :   KDialog(parent), baseUrlCombo(0), copyDirStructureCB(0), queue(false)
{
    setButtons(KDialog::Ok | KDialog::User1 | KDialog::User2 | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setButtonGuiItem(KDialog::User1, KGuiItem(i18n("F2 Queue")));
    setButtonGuiItem(KDialog::User2, KStandardGuiItem::clear());
    setWindowModality(modal ? Qt::WindowModal : Qt::NonModal);
    showButtonSeparator(true);

    QWidget * widget = new QWidget(this);
    QVBoxLayout * topLayout = new QVBoxLayout(widget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(spacingHint());

    QLabel * label = new QLabel(_text, widget);
    topLayout->addWidget(label);

    urlRequester_ = new KUrlRequester(urlName, widget);
    urlRequester_->setMinimumWidth(urlRequester_->sizeHint().width() * 3);
    topLayout->addWidget(urlRequester_);
//     preserveAttrsCB = new QCheckBox(i18n("Preserve attributes (only for local targets)"), widget);
//     preserveAttrsCB->setChecked(presAttrs);
//     topLayout->addWidget(preserveAttrsCB);
    if (!baseURL.isEmpty()) {
        QFrame *line = new QFrame(widget);
        line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        topLayout->addWidget(line);
        copyDirStructureCB = new QCheckBox(i18n("Keep virtual directory structure"), widget);
        connect(copyDirStructureCB, SIGNAL(toggled(bool)), this, SLOT(slotDirStructCBChanged()));
        copyDirStructureCB->setChecked(false);
        topLayout->addWidget(copyDirStructureCB);
        QWidget *hboxWidget = new QWidget(widget);
        QHBoxLayout * hbox = new QHBoxLayout(hboxWidget);
        QLabel * lbl = new QLabel(i18n("Base URL:"),  hboxWidget);
        hbox->addWidget(lbl);

        baseUrlCombo = new QComboBox(hboxWidget);
        baseUrlCombo->setMinimumWidth(baseUrlCombo->sizeHint().width() * 3);
        baseUrlCombo->setEnabled(copyDirStructureCB->isChecked());
        hbox->addWidget(baseUrlCombo);

        QUrl temp = baseURL, tempOld;
        do {
            QString baseURLText = temp.toDisplayString(QUrl::PreferLocalFile);
            baseUrlCombo->addItem(baseURLText);
            tempOld = temp;
            temp = KIO::upUrl(temp);
        } while (!tempOld.matches(temp, QUrl::StripTrailingSlash));
        baseUrlCombo->setCurrentIndex(0);

        topLayout->addWidget(hboxWidget);
    }
    widget->setLayout(topLayout);
    setMainWidget(widget);

    urlRequester_->setFocus();
    connect(urlRequester_->lineEdit(), SIGNAL(textChanged(const QString&)),
            SLOT(slotTextChanged(const QString&)));
    bool state = !urlName.isEmpty();
    enableButtonOk(state);
    enableButton(KDialog::User2, state);
    connect(this, SIGNAL(user2Clicked()), SLOT(slotClear()));
    connect(this, SIGNAL(user1Clicked()), SLOT(slotQueue()));
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy()
{
}

void KUrlRequesterDlgForCopy::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_F2:
        slotQueue();
        return;
    default:
        QDialog::keyPressEvent(e);
    }
}

bool KUrlRequesterDlgForCopy::preserveAttrs()
{
//     return preserveAttrsCB->isChecked();
    return true;
}

bool KUrlRequesterDlgForCopy::copyDirStructure()
{
    if (copyDirStructureCB == 0)
        return false;
    return copyDirStructureCB->isChecked();
}

void KUrlRequesterDlgForCopy::slotTextChanged(const QString & text)
{
    bool state = !text.trimmed().isEmpty();
    enableButtonOk(state);
    enableButton(KDialog::User2, state);
}

void KUrlRequesterDlgForCopy::slotQueue()
{
    queue = true;
    accept();
}

void KUrlRequesterDlgForCopy::slotClear()
{
    urlRequester_->clear();
}

void KUrlRequesterDlgForCopy::slotDirStructCBChanged()
{
    baseUrlCombo->setEnabled(copyDirStructureCB->isChecked());
}

QUrl KUrlRequesterDlgForCopy::selectedURL() const
{
    if (result() == QDialog::Accepted) {
        QUrl url = urlRequester_->url();
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

QUrl KUrlRequesterDlgForCopy::baseURL() const
{
    if (baseUrlCombo == 0)
        return QUrl();
    return QUrl::fromUserInput(baseUrlCombo->currentText(), QString(), QUrl::AssumeLocalFile);
}

KRGetDate::KRGetDate(QDate date, QWidget *parent) : KDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
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
    if (exec() == QDialog::Rejected) chosenDate.setYMD(0, 0, 0);
    hide();
    return chosenDate;
}

void KRGetDate::setDate(QDate date)
{
    chosenDate = date;
    accept();
}

#include "krdialogs.moc"
