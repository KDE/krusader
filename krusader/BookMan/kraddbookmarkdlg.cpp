#include "kraddbookmarkdlg.h"
#include "../krusader.h"
#include "krbookmarkhandler.h"
#include <klocale.h>
#include <qheaderview.h>
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
	setWindowTitle( i18n("Add Bookmark") );
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
	_createInBtn->setIcon(krLoader->loadIcon("go-down", KIconLoader::Small));
	_createInBtn->setCheckable(true);
	connect(_createInBtn, SIGNAL(toggled(bool)), this, SLOT(toggleCreateIn(bool )));
	layout->addWidget(_createInBtn, 2, 2);

	setDetailsWidget(createInWidget());
	
	_name->setFocus();
}

void KrAddBookmarkDlg::toggleCreateIn(bool show) {
	_createInBtn->setIcon(krLoader->loadIcon(show ? "go-up" :"go-down", KIconLoader::Small));
	showButton(KDialog::User1, show);
	setDetailsWidgetVisible(show);
}

// creates the widget that lets you decide where to put the new bookmark
QWidget *KrAddBookmarkDlg::createInWidget() {
	_createIn = new KrTreeWidget(this);
	_createIn->setHeaderLabel( i18n("Folders") );
	_createIn->header()->hide();
	_createIn->setRootIsDecorated(true);
	_createIn->setAlternatingRowColors( false ); // disable alternate coloring 
	
	QTreeWidgetItem *item = new QTreeWidgetItem(_createIn);
	item->setText(0, i18n("Bookmarks"));
	_createIn->expandItem( item );
	item->setSelected(true);
	_xr[item] = krBookMan->_root;

	populateCreateInWidget(krBookMan->_root, item);
	_createIn->setCurrentItem(item);
	slotSelectionChanged();
	connect(_createIn, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
	
	return _createIn;
}

void KrAddBookmarkDlg::slotSelectionChanged() {
	QList<QTreeWidgetItem *> items = _createIn->selectedItems();
	
	if (items.count() > 0 ) {
		_folder->setText(_xr[ items[ 0 ] ]->text());
	}
}

void KrAddBookmarkDlg::populateCreateInWidget(KrBookmark *root, QTreeWidgetItem *parent) {
	QListIterator<KrBookmark *> it( root->children() );
	while (it.hasNext())
	{
		KrBookmark *bm = it.next();
		if (bm->isFolder()) {
			QTreeWidgetItem *item = new QTreeWidgetItem(parent);
			item->setText(0, bm->text());
			item->treeWidget()->expandItem( item );
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
	
	QList<QTreeWidgetItem *> items = _createIn->selectedItems();
	if( items.count() == 0 )
		return;
	
	// add to the list in bookman
	KrBookmark *bm = new KrBookmark(newFolder);
	
	krBookMan->addBookmark(bm, _xr[ items[ 0 ]]);
	// fix the gui
	QTreeWidgetItem *item = new QTreeWidgetItem( items[ 0 ] );
	item->setText(0, bm->text());
	_xr[item] = bm;

	_createIn->setCurrentItem(item);
	item->setSelected(true);
}

KrBookmark * KrAddBookmarkDlg::folder() const
{
	QList<QTreeWidgetItem *> items = _createIn->selectedItems();
	if( items.count() == 0 )
		return 0;
	
	return _xr[ items[ 0 ] ];
}


#include "kraddbookmarkdlg.moc"
