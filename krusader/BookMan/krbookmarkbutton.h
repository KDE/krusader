#ifndef KRBOOKMARK_BUTTON_H
#define KRBOOKMARK_BUTTON_H

#include <qtoolbutton.h>
#include "krbookmarkhandler.h"

class KrBookmarkButton: public QToolButton {
public:
	KrBookmarkButton(QWidget *parent);
	KrBookmarkHandler *handler() const { return _handler; }
	
private:
	KrBookmarkHandler *_handler;
};

#endif // KRBOOKMARK_BUTTON_H
