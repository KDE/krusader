#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "krbookmarkhandler.h"
#include <klocale.h>
#include <q3header.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QGridLayout>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kdebug.h>

KrAddBookmarkDlg::KrAddBookmarkDlg(QWidget *parent, KUrl url):
	KDialog(parent) {
	setButtons( KDialog::User1 | KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	setCaption( i18n("Add Bookmark") );
	setWindowModality( Qt::WindowModal );
	// create the 'new folder' button
	setButtonText(KDialog::User1, i18n("New Folder"));
	showButton(KDialog::User1, false); // hide it until _createIn is shown
	connect( this, SIGNAL( user1Clicked() ), this, SLOT(newFolder()));
	connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
	connect( this, SIGNAL( cancelClicked() ), this, SLOT( reject() ) );

	// create the main widget
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QGridLayout *layout = new QGridLayout(page); // expanding
	layout->setSpacing( spacingHint() );
	layout->setContentsMargins( 0, 0, 0, 0 );
	// name and url
	QLabel *lb1 = new QLabel(i18n("Name:"), page);
	_name = new KLineEdit(page);
	_name->setText(url.prettyUrl()); // default name is the url
	_name->selectAll(); // make the text selected
	layout->addWidget(lb1, 0, 0);	
	layout->addWidget(_name, 0, 1);
	
	QLabel *lb2 = new QLabel(i18n("URL:"), page);
	_url = new KLineEdit(page);
	layout->addWidget(lb2, 1, 0);	
	layout->addWidget(_url, 1, 1);
	_url->setText(url.prettyUrl()); // set the url in the field

	// create in linedit and button
	QLabel *lb3 = new QLabel(i18n("Create in:"), page);
	_folder = new KLineEdit(page);
	layout->addWidget(lb3, 2, 0);
	layout->addWidget(_folder, 2, 1);
	_folder->setReadOnly(true);

	_createInBtn = new QToolButton(page);
	_createInBtn->setPixmap(krLoader->loadIcon("down", KIconLoader::Small));
	_createInBtn->setToggleButton(true);
	connect(_createInBtn, SIGNAL(toggled(bool)), this, SLOT(toggleCreateIn(bool )));
	layout->addWidget(_createInBtn, 2, 2);

	setDetailsWidget(createInWidget());
	
	_name->setFocus();
}

void KrAddBookmarkDlg::toggleCreateIn(bool show) {
	_createInBtn->setPixmap(krLoader->loadIcon(show ? "up" :"down", KIconLoader::Small));
	showButton(KDialog::User1, show);
	setDetailsWidgetVisible(show);
}

// creates the widget that lets you decide where to put the new bookmark
QWidget *KrAddBookmarkDlg::createInWidget() {
	_createIn = new K3ListView(this);
	_createIn->addColumn("Folders");
	_createIn->header()->hide();
	_createIn->setRootIsDecorated(true);
	_createIn->setAlternateBackground(QColor()); // disable alternate coloring 
	
	K3ListViewItem *item = new K3ListViewItem(_createIn, i18n("Bookmarks"));
	item->setOpen(true);
	item->setSelected(true);
	_xr[item] = krBookMan->_root;

	populateCreateInWidget(krBookMan->_root, item);
	_createIn->setCurrentItem(item);
	createInSelection(item);
	connect(_createIn, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(createInSelection(Q3ListViewItem*)));
	
	return _createIn;
}

void KrAddBookmarkDlg::createInSelection(Q3ListViewItem *item) {
	if (item) {
		_folder->setText(_xr[static_cast<K3ListViewItem*>(item)]->text());
	}
}

void KrAddBookmarkDlg::populateCreateInWidget(KrBookmark *root, K3ListViewItem *parent) {
	QListIterator<KrBookmark *> it( root->children() );
	while (it.hasNext())
	{
		KrBookmark *bm = it.next();
		if (bm->isFolder()) {
			K3ListViewItem *item = new K3ListViewItem(parent, bm->text());
			item->setOpen(true);
			_xr[item] = bm;
			populateCreateInWidget(bm, item);
		}
	}
}

void KrAddBookmarkDlg::newFolder() {
	// get the name
	QString newFolder = KInputDialog::getText(i18n("New Folder"), i18n("Folder name:"), QString(), 0, this);
	if (newFolder == QString())
		return;
	// add to the list in bookman
	KrBookmark *bm = new KrBookmark(newFolder);
	krBookMan->addBookmark(bm, _xr[static_cast<K3ListViewItem*>(_createIn->selectedItem())]);
	// fix the gui
	K3ListViewItem *item = new K3ListViewItem(_createIn->selectedItem(), bm->text());
	_xr[item] = bm;

	_createIn->setCurrentItem(item);
	item->setSelected(true);
}

#include "kraddbookmarkdlg.moc"
