#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../VFS/vfs.h"
#include <kdialogbase.h>
#include <kurl.h>
#include <klineedit.h>
#include <qmap.h>
#include <klistview.h>
#include <qtoolbutton.h>

class KrAddBookmarkDlg: public KDialogBase {
	Q_OBJECT
public:
	KrAddBookmarkDlg(QWidget *parent, KURL url = 0);
	KURL url() const { return vfs::fromPathOrURL(_url->text()); }
	QString name() const { return _name->text(); }
	KrBookmark *folder() const { return _xr[static_cast<KListViewItem*>(_createIn->selectedItem())]; }

protected:
	QWidget *createInWidget();
	void populateCreateInWidget(KrBookmark *root, KListViewItem *parent);

protected slots:
	void toggleCreateIn(bool show);
	void createInSelection(QListViewItem *item);
	void newFolder();
	
private:
	KLineEdit *_name;
	KLineEdit *_url;
	KLineEdit *_folder;
	KListView *_createIn;
	QMap<KListViewItem*, KrBookmark*> _xr;
	QToolButton *_createInBtn;
};

#endif // KRADDBOOKMARKDLG_H
