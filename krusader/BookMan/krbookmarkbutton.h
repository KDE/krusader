#ifndef KRBOOKMARK_BUTTON_H
#define KRBOOKMARK_BUTTON_H

#include <qtoolbutton.h>
#include <kactionmenu.h>
#include "krbookmarkhandler.h"

class KrBookmarkButton: public QToolButton {
	Q_OBJECT
public:
	KrBookmarkButton(QWidget *parent);
	void openPopup();

signals:
	void openUrl(const KUrl &url);

protected slots:
	void populate();
	
private:
	KActionMenu *acmBookmarks;
};

#endif // KRBOOKMARK_BUTTON_H
