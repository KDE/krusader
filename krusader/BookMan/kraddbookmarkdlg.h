#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include <kdialogbase.h>
#include <kurl.h>
#include <klineedit.h>

class KrAddBookmarkDlg: public KDialogBase {
public:
	KrAddBookmarkDlg(QWidget *parent, KURL url = 0);
	QString url() const { return _url->text(); }
	QString name() const { return _name->text(); }
	
private:
	KLineEdit *_name;
	KLineEdit *_url;
};

#endif // KRADDBOOKMARKDLG_H
