#ifndef KRBOOKMARK_BUTTON_H
#define KRBOOKMARK_BUTTON_H

#include <qtoolbutton.h>
#include "krbookmarkhandler.h"

class KrBookmarkButton: public QToolButton {
	Q_OBJECT
public:
	KrBookmarkButton(QWidget *parent);
	KrBookmarkHandler *handler() const { return _handler; }

signals:
	void openUrl(const KURL &url);
	
private:
	KrBookmarkHandler *_handler;
	KActionMenu *acmBookmarks;
};

#endif // KRBOOKMARK_BUTTON_H
