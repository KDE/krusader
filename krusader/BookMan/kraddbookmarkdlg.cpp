#include "kraddbookmarkdlg.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>

KrAddBookmarkDlg::KrAddBookmarkDlg(QWidget *parent, KURL url):
	KDialogBase(KDialogBase::Swallow, i18n("Add Bookmark"),
					KDialogBase::Details | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent) {
	setButtonText(KDialogBase::Details, i18n("Create in"));;

	// create the main widget
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QGridLayout *layout = new QGridLayout(page, 1, 1, 0, spacingHint()); // expanding
	QLabel *lb1 = new QLabel(i18n("Name:"), page);
	layout->addWidget(lb1, 0, 0);
	_name = new KLineEdit(page);
	layout->addWidget(_name, 0, 1);
	
	QLabel *lb2 = new QLabel(i18n("URL:"), page);
	layout->addWidget(lb2, 1, 0);
	_url = new KLineEdit(page);
	layout->addWidget(_url, 1, 1);
	_url->setText(url.url());
}
