#include "krselectionmode.h"
#include "../krusader.h"

static KrSelectionMode *__currentSelectionMode = 0; // uninitiated, at first


KonqSelectionMode konqSelectionMode;
OriginalSelectionMode originalSelectionMode;
TCSelectionMode tcSelectionMode;


KrSelectionMode* KrSelectionMode::getSelectionHandler()
{
	if (__currentSelectionMode) { // don't check krConfig every time
		return __currentSelectionMode;
	} else { // nothing yet, set the correct one
		krConfig->setGroup( "Look&Feel" );
   	QString mode = krConfig->readEntry("Mouse Selection", "");
		switch (mode.toInt()) {
			case 0:
				__currentSelectionMode = &originalSelectionMode;
				break;
			case 1:
				__currentSelectionMode = &konqSelectionMode;
				break;
			case 2:
				__currentSelectionMode = &tcSelectionMode;
				break;
			default:
				break;
		}
		// init and return
		__currentSelectionMode->init();
		return __currentSelectionMode;
	}
}

void KrSelectionMode::resetSelectionHandler() {
	__currentSelectionMode = 0;
}

