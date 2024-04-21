/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kurllistrequester.h"
#include "../FileSystem/filesystem.h"
#include "../compat.h"
#include "../icon.h"

// QtGui
#include <QCursor>
#include <QKeyEvent>
#include <QPixmap>
// QtWidgets
#include <QFileDialog>
#include <QGridLayout>
#include <QLayout>
#include <QMenu>

#include <KLocalizedString>
#include <KMessageBox>

#define DELETE_ITEM_ID 100

KURLListRequester::KURLListRequester(Mode requestMode, QWidget *parent)
    : QWidget(parent)
    , mode(requestMode)
{
    // Creating the widget

    auto *urlListRequesterGrid = new QGridLayout(this);
    urlListRequesterGrid->setSpacing(0);
    urlListRequesterGrid->setContentsMargins(0, 0, 0, 0);

    urlLineEdit = new KLineEdit(this);
    urlListRequesterGrid->addWidget(urlLineEdit, 0, 0);

    urlAddBtn = new QToolButton(this);
    urlAddBtn->setText("");
    urlAddBtn->setIcon(Icon("arrow-down"));
    urlListRequesterGrid->addWidget(urlAddBtn, 0, 1);

    urlBrowseBtn = new QToolButton(this);
    urlBrowseBtn->setText("");
    urlBrowseBtn->setIcon(Icon("folder"));
    urlListRequesterGrid->addWidget(urlBrowseBtn, 0, 2);

    urlListBox = new KrListWidget(this);
    urlListBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    urlListRequesterGrid->addWidget(urlListBox, 1, 0, 1, 3);

    // add shell completion

    completion.setMode(KUrlCompletion::FileCompletion);
    urlLineEdit->setCompletionObject(&completion);

    // connection table

    connect(urlAddBtn, &QToolButton::clicked, this, &KURLListRequester::slotAdd);
    connect(urlBrowseBtn, &QToolButton::clicked, this, &KURLListRequester::slotBrowse);
    connect(urlLineEdit, &KLineEdit::KLINEEDIT_RETURNKEYPRESSED, this, &KURLListRequester::slotAdd);
    connect(urlListBox, &KrListWidget::itemRightClicked, this, &KURLListRequester::slotRightClicked);
    connect(urlLineEdit, &KLineEdit::textChanged, this, &KURLListRequester::changed);
}

void KURLListRequester::slotAdd()
{
    QString text = urlLineEdit->text().simplified();
    if (text.length()) {
        QString error;
        emit checkValidity(text, error);

        if (!error.isNull())
            KMessageBox::error(this, error);
        else {
            urlListBox->addItem(text);
            urlLineEdit->clear();
            emit changed();
        }
    }
}

void KURLListRequester::slotBrowse()
{
    QUrl url;
    switch (mode) {
    case RequestFiles:
        url = QFileDialog::getOpenFileUrl(this);
        break;
    case RequestDirs:
        url = QFileDialog::getExistingDirectoryUrl(this);
        break;
    }
    if (!url.isEmpty())
        urlLineEdit->setText(url.toDisplayString(QUrl::PreferLocalFile));
    urlLineEdit->setFocus();
}

void KURLListRequester::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete) {
        if (urlListBox->hasFocus()) {
            deleteSelectedItems();
            return;
        }
    }

    QWidget::keyPressEvent(e);
}

void KURLListRequester::deleteSelectedItems()
{
    const QList<QListWidgetItem *> selectedItems = urlListBox->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        delete item;
    emit changed();
}

void KURLListRequester::slotRightClicked(QListWidgetItem *item, const QPoint &pos)
{
    if (item == nullptr)
        return;

    QMenu popupMenu(this);
    QAction *menuAction = popupMenu.addAction(i18n("Delete"));

    if (menuAction == popupMenu.exec(pos)) {
        if (item->isSelected())
            deleteSelectedItems();
        else {
            delete item;
            emit changed();
        }
    }
}

QList<QUrl> KURLListRequester::urlList()
{
    QStringList lines;

    const QString lineEditText = urlLineEdit->text().simplified();
    if (!lineEditText.isEmpty()) {
        lines.append(lineEditText);
    }

    for (int i = 0; i != urlListBox->count(); i++) {
        QListWidgetItem *item = urlListBox->item(i);
        lines.append(item->text().simplified());
    }

    QList<QUrl> urls;
    for (QString text : lines) {
        QString error;
        emit checkValidity(text, error);
        if (error.isNull())
            urls.append(QUrl::fromUserInput(text, QString(), QUrl::AssumeLocalFile));
    }

    return urls;
}

void KURLListRequester::setUrlList(const QList<QUrl> &urlList)
{
    urlLineEdit->clear();
    urlListBox->clear();

    for (const QUrl &url : urlList) {
        urlListBox->addItem(url.toDisplayString(QUrl::PreferLocalFile));
    }

    emit changed();
}
