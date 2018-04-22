/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2005-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "kurllistrequester.h"
#include "../FileSystem/filesystem.h"
#include "../icon.h"

// QtGui
#include <QPixmap>
#include <QCursor>
#include <QKeyEvent>
// QtWidgets
#include <QFileDialog>
#include <QLayout>
#include <QGridLayout>
#include <QMenu>

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#define DELETE_ITEM_ID    100

KURLListRequester::KURLListRequester(Mode requestMode, QWidget *parent)
    : QWidget(parent), mode(requestMode)
{
    // Creating the widget

    QGridLayout *urlListRequesterGrid = new QGridLayout(this);
    urlListRequesterGrid->setSpacing(0);
    urlListRequesterGrid->setContentsMargins(0, 0, 0, 0);

    urlLineEdit = new KLineEdit(this);
    urlListRequesterGrid->addWidget(urlLineEdit, 0, 0);

    urlListBox = new KrListWidget(this);
    urlListBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    urlListRequesterGrid->addWidget(urlListBox, 1, 0, 1, 3);

    urlAddBtn = new QToolButton(this);
    urlAddBtn->setText("");
    urlAddBtn->setIcon(Icon("arrow-down"));
    urlListRequesterGrid->addWidget(urlAddBtn, 0, 1);

    urlBrowseBtn = new QToolButton(this);
    urlBrowseBtn->setText("");
    urlBrowseBtn->setIcon(Icon("folder"));
    urlListRequesterGrid->addWidget(urlBrowseBtn, 0, 2);

    // add shell completion

    completion.setMode(KUrlCompletion::FileCompletion);
    urlLineEdit->setCompletionObject(&completion);

    // connection table

    connect(urlAddBtn, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(urlBrowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    connect(urlLineEdit, SIGNAL(returnPressed(QString)), this, SLOT(slotAdd()));
    connect(urlListBox, SIGNAL(itemRightClicked(QListWidgetItem*,QPoint)), this,
            SLOT(slotRightClicked(QListWidgetItem*,QPoint)));
    connect(urlLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
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
    QList<QListWidgetItem *> delList = urlListBox->selectedItems();
    for (int i = 0; i != delList.count(); i++)
        delete delList[ i ];
    emit changed();
}

void KURLListRequester::slotRightClicked(QListWidgetItem *item, const QPoint &pos)
{
    if (item == 0)
        return;

    QMenu popupMenu(this);
    QAction * menuAction = popupMenu.addAction(i18n("Delete"));

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
    QList<QUrl> urls;

    QString text = urlLineEdit->text().simplified();
    if (!text.isEmpty()) {
        QString error;
        emit checkValidity(text, error);
        if (error.isNull())
            urls.append(QUrl::fromUserInput(text, QString(), QUrl::AssumeLocalFile));
    }

    for (int i = 0; i != urlListBox->count(); i++) {
        QListWidgetItem *item = urlListBox->item(i);

        QString text = item->text().simplified();

        QString error;
        emit checkValidity(text, error);
        if (error.isNull())
            urls.append(QUrl::fromUserInput(text, QString(), QUrl::AssumeLocalFile));
    }

    return urls;
}

void KURLListRequester::setUrlList(QList<QUrl> urlList)
{
    urlLineEdit->clear();
    urlListBox->clear();

    QList<QUrl>::iterator it;

    for (it = urlList.begin(); it != urlList.end(); ++it)
        urlListBox->addItem(it->toDisplayString(QUrl::PreferLocalFile));

    emit changed();
}

