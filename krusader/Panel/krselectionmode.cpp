#include "krselectionmode.h"
#include "../krusader.h"

static KrSelectionMode *__currentSelectionMode = 0; // uninitiated, at first


KrKDESelectionMode krKDESelectionMode;
KrOriginalSelectionMode krOriginalSelectionMode;
KrNewSelectionMode krNewSelectionMode;


KrSelectionMode* KrSelectionMode::getSelectionHandler()
{
	if (__currentSelectionMode) { // don't check krConfig every time
		return __currentSelectionMode;
	} else { // nothing yet, set the correct one
		krConfig->setGroup( "Look&Feel" );
   	QString mode = krConfig->readEntry("SelectionMode", "");
   	if (mode == "KDESelectionMode") {
      	__currentSelectionMode = &krKDESelectionMode;
   	} else if (mode == "NewSelectionMode") {
      	__currentSelectionMode = &krNewSelectionMode;
   	} else __currentSelectionMode = &krOriginalSelectionMode;
		// init and return
		__currentSelectionMode->init();
		return __currentSelectionMode;
	}
}

void KrSelectionMode::resetSelectionHandler() {
	__currentSelectionMode = 0;
}

