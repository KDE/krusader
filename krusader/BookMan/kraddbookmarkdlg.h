#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../VFS/vfs.h"
#include <kdialog.h>
#include <kurl.h>
#include <klineedit.h>
#include <qmap.h>
#include <k3listview.h>
#include <qtoolbutton.h>

class KrAddBookmarkDlg: public KDialog {
	Q_OBJECT
public:
	KrAddBookmarkDlg(QWidget *parent, KUrl url = 0);
	KUrl url() const { return vfs::fromPathOrUrl(_url->text()); }
	QString name() const { return _name->text(); }
	KrBookmark *folder() const { return _xr[static_cast<K3ListViewItem*>(_createIn->selectedItem())]; }

protected:
	QWidget *createInWidget();
	void populateCreateInWidget(KrBookmark *root, K3ListViewItem *parent);

protected slots:
	void toggleCreateIn(bool show);
	void createInSelection(Q3ListViewItem *item);
	void newFolder();
	
private:
	KLineEdit *_name;
	KLineEdit *_url;
	KLineEdit *_folder;
	K3ListView *_createIn;
	QMap<K3ListViewItem*, KrBookmark*> _xr;
	QToolButton *_createInBtn;
};

#endif // KRADDBOOKMARKDLG_H
