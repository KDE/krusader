#include "krusaderapp.h"

KrusaderApp::KrusaderApp(): KApplication() {}

bool KrusaderApp::x11EventFilter ( XEvent *e ) {
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
