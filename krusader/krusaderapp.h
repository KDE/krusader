#ifndef KRUSADERAPP_H
#define KRUSADERAPP_H

#include <kapplication.h>
#include "X11/Xlib.h"

// declare a dummy kapplication, just to get the X focusin focusout events
class KrusaderApp: public KApplication {
	Q_OBJECT
public:
	KrusaderApp();
	bool x11EventFilter ( XEvent *e );

signals:
	void windowActive();
	void windowInactive();
};


#endif // KRUSADERAPP_H
