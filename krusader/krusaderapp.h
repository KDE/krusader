#ifndef KRUSADERAPP_H
#define KRUSADERAPP_H

#include <kapplication.h>
#include "X11/Xlib.h"

// declare a dummy kapplication, just to get the X focusin focusout events
class KrusaderApp: public KApplication {
	Q_OBJECT
public:
	KrusaderApp(): KApplication() {}
	bool x11EventFilter ( XEvent *e ) {
		switch (e->type) {
			case FocusIn:
				emit windowActive();
				break;
			case FocusOut:
				emit windowInactive();
				break;
			
		}
		//return false; // event should be processed normally
		return KApplication::x11EventFilter(e);
	}
signals:
	void windowActive();
	void windowInactive();
};


#endif // KRUSADERAPP_H
