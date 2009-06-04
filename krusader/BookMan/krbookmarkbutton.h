#ifndef KRBOOKMARKBUTTON_H
#define KRBOOKMARKBUTTON_H

#include <QtGui/QToolButton>

#include <kactionmenu.h>

#include "krbookmarkhandler.h"

class KrBookmarkButton: public QToolButton {
	Q_OBJECT
public:
	KrBookmarkButton(QWidget *parent);
	void showMenu();

signals:
	void openUrl(const KUrl &url);
	void aboutToShow();

protected slots:
	void populate();
	
private:
	KActionMenu *acmBookmarks;
};

#endif // KRBOOKMARK_BUTTON_H
