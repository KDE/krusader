/***************************************************************************
                     feedtolistboxdialog.cpp  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#include "feedtolistboxdialog.h"
#include "synchronizer.h"
#include "synchronizergui.h"
#include "../VFS/vfs.h"
#include "../VFS/virt_vfs.h"
#include "../krglobal.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>

// TODO KF5 - these headers are from deprecated KDE4LibsSupport : remove them
#include <KDE/KLocale>

#include <KWidgetsAddons/KMessageBox>

#define  S_LEFT        0
#define  S_RIGHT       1
#define  S_BOTH        2

FeedToListBoxDialog::FeedToListBoxDialog(QWidget *parent, Synchronizer *sync,
        QTreeWidget *syncL, bool equOK) : KDialog(parent),
        synchronizer(sync), syncList(syncL), equalAllowed(equOK), accepted(false)
{

    setWindowTitle(i18n("Krusader::Feed to listbox"));
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::User1);
    setDefaultButton(KDialog::Ok);
    setWindowModality(Qt::WindowModal);
    showButtonSeparator(true);
    setButtonGuiItem(KDialog::User1, KStandardGuiItem::clear());

    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(reject()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));

    // autodetecting the parameters

    int selectedNum = 0;
    int itemNum = 0;
    int leftExistingNum = 0;
    int rightExistingNum = 0;

    QTreeWidgetItemIterator it(syncList);
    while (*it) {
        SynchronizerGUI::SyncViewItem *item = (SynchronizerGUI::SyncViewItem *) * it;
        SynchronizerFileItem *syncItem = item->synchronizerItemRef();

        if (syncItem && syncItem->isMarked()) {
            if (item->isSelected() || syncItem->task() != TT_EQUALS || equalAllowed) {
                itemNum++;
                if (item->isSelected())
                    selectedNum++;

                if (syncItem->existsInLeft())
                    leftExistingNum++;
                if (syncItem->existsInRight())
                    rightExistingNum++;
            }
        }
        it++;
    }

    if (itemNum == 0) {
        hide();
        KMessageBox::error(parent, i18n("No elements to feed."));
        return;
    }

    // guessing the collection name

    virt_vfs v(0, true);
    if (!v.vfs_refresh(QUrl("virt:/")))
        return;

    KConfigGroup group(krConfig, "Synchronize");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    do {
        queryName = i18n("Synchronize results") + QString(" %1").arg(listBoxNum++);
    } while (v.vfs_search(queryName) != 0);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    // creating the widget

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QLabel *label = new QLabel(i18n("Here you can name the file collection"), widget);
    layout->addWidget(label);

    lineEdit = new QLineEdit(widget);
    lineEdit->setText(queryName);
    lineEdit->selectAll();
    layout->addWidget(lineEdit);

    QWidget *hboxWidget = new QWidget(widget);
    QHBoxLayout * hbox = new QHBoxLayout(hboxWidget);

    QLabel *label2 = new QLabel(i18n("Side to feed:"), hboxWidget);
    label2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->addWidget(label2);

    sideCombo = new QComboBox(hboxWidget);
    sideCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sideCombo->addItem(i18n("Left"));
    sideCombo->addItem(i18n("Right"));
    sideCombo->addItem(i18n("Both"));
    hbox->addWidget(sideCombo);

    if (leftExistingNum == 0) {
        sideCombo->setCurrentIndex(1);
        sideCombo->setEnabled(false);
    } else if (rightExistingNum == 0) {
        sideCombo->setCurrentIndex(0);
        sideCombo->setEnabled(false);
    } else
        sideCombo->setCurrentIndex(2);

    QFrame *line = new QFrame(hboxWidget);
    line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(line);

    cbSelected = new QCheckBox(i18n("Selected files only"), hboxWidget);
    cbSelected->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cbSelected->setEnabled(selectedNum != 0);
    cbSelected->setChecked(selectedNum != 0);
    hbox->addWidget(cbSelected);

    layout->addWidget(hboxWidget);

    setMainWidget(widget);

    exec();
}

void FeedToListBoxDialog::slotUser1()
{
    lineEdit->clear();
}

void FeedToListBoxDialog::slotOk()
{
    int side = sideCombo->currentIndex();
    bool selected = cbSelected->isChecked();
    QString name = lineEdit->text();
    QList<QUrl> urlList;

    QTreeWidgetItemIterator it(syncList);
    for (;*it; it++) {
        SynchronizerGUI::SyncViewItem *item = (SynchronizerGUI::SyncViewItem *) * it;
        SynchronizerFileItem *syncItem = item->synchronizerItemRef();

        if (!syncItem || !syncItem->isMarked())
            continue;
        if (selected && !item->isSelected())
            continue;
        if (!equalAllowed && syncItem->task() == TT_EQUALS && (!selected || !item->isSelected()))
            continue;

        if ((side == S_BOTH || side == S_LEFT) && syncItem->existsInLeft()) {
            QString leftDirName = syncItem->leftDirectory().isEmpty() ? "" : syncItem->leftDirectory() + '/';
            QUrl leftURL = QUrl::fromUserInput(synchronizer->leftBaseDirectory() + leftDirName + syncItem->leftName(),
                                               QString(), QUrl::AssumeLocalFile);
            urlList.push_back(leftURL);
        }

        if ((side == S_BOTH || side == S_RIGHT) && syncItem->existsInRight()) {
            QString rightDirName = syncItem->rightDirectory().isEmpty() ? "" : syncItem->rightDirectory() + '/';
            QUrl leftURL = QUrl::fromUserInput(synchronizer->rightBaseDirectory() + rightDirName + syncItem->rightName(),
                                               QString(), QUrl::AssumeLocalFile);
            urlList.push_back(leftURL);
        }
    }

    QUrl url = QUrl(QString("virt:/") + name);
    virt_vfs v(0, true);
    if (!v.vfs_refresh(url)) {
        KMessageBox::error(parentWidget(), i18n("Cannot open %1.", url.toDisplayString()));
        return;
    }
    v.vfs_addFiles(&urlList, KIO::CopyJob::Copy, 0);
    ACTIVE_MNG->slotNewTab(url);
    accepted = true;
    accept();
}

#include "feedtolistboxdialog.moc"
