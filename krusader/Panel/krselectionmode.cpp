#include "krselectionmode.h"
#include "../krusader.h"
#include "../defaults.h"

static KrSelectionMode *__currentSelectionMode = 0; // uninitiated, at first


KonqSelectionMode konqSelectionMode;
OriginalSelectionMode originalSelectionMode;
TCSelectionMode tcSelectionMode;
UserSelectionMode userSelectionMode;

KrSelectionMode* KrSelectionMode::getSelectionHandler()
{
	if (__currentSelectionMode) { // don't check krConfig every time
		return __currentSelectionMode;
	} else { // nothing yet, set the correct one
	KConfigGroup group( krConfig, "Look&Feel" );
   	QString mode = group.readEntry("Mouse Selection", QString("") );
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
			case 3:
				__currentSelectionMode = &userSelectionMode;
				userSelectionMode.init();
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

void UserSelectionMode::init() {
	KConfigGroup group( krConfig, "Custom Selection Mode");
	_useQTSelection = group.readEntry("QT Selection", _QtSelection);
	_leftButtonSelects = group.readEntry("Left Selects", _LeftSelects);
	_leftButtonPreservesSelection = group.readEntry("Left Preserves", _LeftPreserves);
	_shiftCtrlLeftButtonSelects = group.readEntry("ShiftCtrl Left Selects", _ShiftCtrlLeft);
	_rightButtonSelects = group.readEntry("Right Selects", _RightSelects);
	_rightButtonPreservesSelection = group.readEntry("Right Preserves", _RightPreserves);
	_shiftCtrlRightButtonSelects = group.readEntry("ShiftCtrl Right Selects", _ShiftCtrlRight);
	_spaceMovesDown = group.readEntry("Space Moves Down", _SpaceMovesDown);
	_spaceCalculatesDiskSpace = group.readEntry("Space Calc Space", _SpaceCalcSpace);
	_insertMovesDown = group.readEntry("Insert Moves Down", _InsertMovesDown);
	_showContextMenu = (group.readEntry("Immediate Context Menu", _ImmediateContextMenu) ? -1 : 500);
}

