/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#include "kraddbookmarkdlg.h"
#include "../krglobal.h"
#include "../icon.h"
#include "krbookmarkhandler.h"

// QtWidgets
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>

#include <KI18n/KLocalizedString>


KrAddBookmarkDlg::KrAddBookmarkDlg(QWidget *parent, QUrl url):
        QDialog(parent)
{
    setWindowModality(Qt::WindowModal);
    setWindowTitle(i18n("Add Bookmark"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QGridLayout *layout = new QGridLayout; // expanding
    // name and url
    QLabel *lb1 = new QLabel(i18n("Name:"), this);
    _name = new KLineEdit(this);
    _name->setText(url.toDisplayString()); // default name is the url
    _name->selectAll(); // make the text selected
    layout->addWidget(lb1, 0, 0);
    layout->addWidget(_name, 0, 1);

    QLabel *lb2 = new QLabel(i18n("URL:"), this);
    _url = new KLineEdit(this);
    layout->addWidget(lb2, 1, 0);
    layout->addWidget(_url, 1, 1);
    _url->setText(url.toDisplayString()); // set the url in the field

    // create in linedit and button
    QLabel *lb3 = new QLabel(i18n("Create in:"), this);
    _folder = new KLineEdit(this);
    layout->addWidget(lb3, 2, 0);
    layout->addWidget(_folder, 2, 1);
    _folder->setReadOnly(true);

    _createInBtn = new QToolButton(this);
    _createInBtn->setIcon(Icon("go-down"));
    _createInBtn->setCheckable(true);
    connect(_createInBtn, SIGNAL(toggled(bool)), this, SLOT(toggleCreateIn(bool)));
    layout->addWidget(_createInBtn, 2, 2);

    mainLayout->addLayout(layout);

    detailsWidget = createInWidget();
    detailsWidget->setVisible(false);
    mainLayout->addWidget(detailsWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    newFolderButton = new QPushButton(i18n("New Folder"));
    buttonBox->addButton(newFolderButton, QDialogButtonBox::ActionRole);
    newFolderButton->setVisible(false);// hide it until _createIn is shown
    connect(newFolderButton, SIGNAL(clicked()), this, SLOT(newFolder()));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    _name->setFocus();
    resize(sizeHint().width() * 2, sizeHint().height());
}

void KrAddBookmarkDlg::toggleCreateIn(bool show)
{
    _createInBtn->setIcon(Icon(show ? "go-up" : "go-down"));
    newFolderButton->setVisible(show);
    detailsWidget->setVisible(show);
}

// creates the widget that lets you decide where to put the new bookmark
QWidget *KrAddBookmarkDlg::createInWidget()
{
    _createIn = new KrTreeWidget(this);
    _createIn->setHeaderLabel(i18n("Folders"));
    _createIn->header()->hide();
    _createIn->setRootIsDecorated(true);
    _createIn->setAlternatingRowColors(false);   // disable alternate coloring

    QTreeWidgetItem *item = new QTreeWidgetItem(_createIn);
    item->setText(0, i18n("Bookmarks"));
    _createIn->expandItem(item);
    item->setSelected(true);
    _xr[item] = krBookMan->_root;

    populateCreateInWidget(krBookMan->_root, item);
    _createIn->setCurrentItem(item);
    slotSelectionChanged();
    connect(_createIn, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));

    return _createIn;
}

void KrAddBookmarkDlg::slotSelectionChanged()
{
    QList<QTreeWidgetItem *> items = _createIn->selectedItems();

    if (items.count() > 0) {
        _folder->setText(_xr[ items[ 0 ] ]->text());
    }
}

void KrAddBookmarkDlg::populateCreateInWidget(KrBookmark *root, QTreeWidgetItem *parent)
{
    QListIterator<KrBookmark *> it(root->children());
    while (it.hasNext()) {
        KrBookmark *bm = it.next();
        if (bm->isFolder()) {
            QTreeWidgetItem *item = new QTreeWidgetItem(parent);
            item->setText(0, bm->text());
            item->treeWidget()->expandItem(item);
            _xr[item] = bm;
            populateCreateInWidget(bm, item);
        }
    }
}

void KrAddBookmarkDlg::newFolder()
{
    // get the name
    QString newFolder = QInputDialog::getText(this, i18n("New Folder"), i18n("Folder name:"));
    if (newFolder.isEmpty()) {
        return;
    }

    QList<QTreeWidgetItem *> items = _createIn->selectedItems();
    if (items.count() == 0)
        return;

    // add to the list in bookman
    KrBookmark *bm = new KrBookmark(newFolder);

    krBookMan->addBookmark(bm, _xr[ items[ 0 ]]);
    // fix the gui
    QTreeWidgetItem *item = new QTreeWidgetItem(items[ 0 ]);
    item->setText(0, bm->text());
    _xr[item] = bm;

    _createIn->setCurrentItem(item);
    item->setSelected(true);
}

KrBookmark * KrAddBookmarkDlg::folder() const
{
    QList<QTreeWidgetItem *> items = _createIn->selectedItems();
    if (items.count() == 0)
        return 0;

    return _xr[ items[ 0 ] ];
}

