#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "krbookmarkhandler.h"
#include <klocale.h>
#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kdebug.h>

KrAddBookmarkDlg::KrAddBookmarkDlg(QWidget *parent, KURL url):
	KDialogBase(KDialogBase::Swallow, i18n("Add Bookmark"),
				 KDialogBase::User1 | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent) {
	// create the 'new folder' button
	setButtonText(KDialogBase::User1, i18n("New Folder"));
	showButton(KDialogBase::User1, false); // hide it until _createIn is shown
	connect(this, SIGNAL(user1Clicked()), this, SLOT(newFolder()));

	// create the main widget
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QGridLayout *layout = new QGridLayout(page, 1, 1, 0, spacingHint()); // expanding
	// name and url
	QLabel *lb1 = new QLabel(i18n("Name:"), page);
	_name = new KLineEdit(page);
	layout->addWidget(lb1, 0, 0);	
	layout->addWidget(_name, 0, 1);
	
	QLabel *lb2 = new QLabel(i18n("URL:"), page);
	_url = new KLineEdit(page);
	layout->addWidget(lb2, 1, 0);	
	layout->addWidget(_url, 1, 1);
	_url->setText(url.url()); // set the url in the field

	// create in linedit and button
	QLabel *lb3 = new QLabel(i18n("Create in:"), page);
	_folder = new KLineEdit(page);
	layout->addWidget(lb3, 2, 0);
	layout->addWidget(_folder, 2, 1);
	_folder->setReadOnly(true);

	_createInBtn = new QToolButton(page);
	_createInBtn->setPixmap(krLoader->loadIcon("down", KIcon::Small));
	_createInBtn->setToggleButton(true);
	connect(_createInBtn, SIGNAL(toggled(bool)), this, SLOT(toggleCreateIn(bool )));
	layout->addWidget(_createInBtn, 2, 2);

	setDetailsWidget(createInWidget());
	
	_name->setFocus();
}

void KrAddBookmarkDlg::toggleCreateIn(bool show) {
	_createInBtn->setPixmap(krLoader->loadIcon(show ? "up" :"down", KIcon::Small));
	showButton(KDialogBase::User1, show);
	setDetails(show);
}

// creates the widget that lets you decide where to put the new bookmark
QWidget *KrAddBookmarkDlg::createInWidget() {
	_createIn = new KListView(this);
	_createIn->addColumn("Folders");
	_createIn->header()->hide();
	_createIn->setRootIsDecorated(true);
	_createIn->setAlternateBackground(QColor()); // disable alternate coloring 
	
	KListViewItem *item = new KListViewItem(_createIn, i18n("Bookmarks"));
	item->setOpen(true);
	item->setSelected(true);
	_xr[item] = krBookMan->_root;

	populateCreateInWidget(krBookMan->_root, item);
	_createIn->setCurrentItem(item);
	createInSelection(item);
	connect(_createIn, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(createInSelection(QListViewItem*)));
	
	return _createIn;
}

void KrAddBookmarkDlg::createInSelection(QListViewItem *item) {
	if (item) {
		_folder->setText(_xr[static_cast<KListViewItem*>(item)]->text());
	}
}

void KrAddBookmarkDlg::populateCreateInWidget(KrBookmark *root, KListViewItem *parent) {
	for (KrBookmark *bm = root->children().first(); bm; bm = root->children().next()) {
		if (bm->isFolder()) {
			KListViewItem *item = new KListViewItem(parent, bm->text());
			item->setOpen(true);
			_xr[item] = bm;
			populateCreateInWidget(bm, item);
		}
	}
}

void KrAddBookmarkDlg::newFolder() {
	// get the name
	QString newFolder = KInputDialog::getText(i18n("New Folder"), i18n("Folder name:"), QString::null, 0, this);
	if (newFolder == QString::null)
		return;
	// add to the list in bookman
	KrBookmark *bm = new KrBookmark(newFolder);
	krBookMan->addBookmark(bm, _xr[static_cast<KListViewItem*>(_createIn->selectedItem())]);
	// fix the gui
	KListViewItem *item = new KListViewItem(_createIn->selectedItem(), bm->text());
	_xr[item] = bm;

	_createIn->setCurrentItem(item);
	item->setSelected(true);
}
