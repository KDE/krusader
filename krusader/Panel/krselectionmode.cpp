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
	krConfig->setGroup("Custom Selection Mode");
	_useQTSelection = krConfig->readBoolEntry("QT Selection", _QtSelection);
	_leftButtonSelects = krConfig->readBoolEntry("Left Selects", _LeftSelects);
	_leftButtonPreservesSelection = krConfig->readBoolEntry("Left Preserves", _LeftPreserves);
	_shiftCtrlLeftButtonSelects = krConfig->readBoolEntry("ShiftCtrl Left Selects", _ShiftCtrlLeft);
	_rightButtonSelects = krConfig->readBoolEntry("Right Selects", _RightSelects);
	_rightButtonPreservesSelection = krConfig->readBoolEntry("Right Preserves", _RightPreserves);
	_shiftCtrlRightButtonSelects = krConfig->readBoolEntry("ShiftCtrl Right Selects", _ShiftCtrlRight);
	_spaceMovesDown = krConfig->readBoolEntry("Space Moves Down", _SpaceMovesDown);
	_spaceCalculatesDiskSpace = krConfig->readBoolEntry("Space Calc Space", _SpaceCalcSpace);
	_insertMovesDown = krConfig->readBoolEntry("Insert Moves Down", _InsertMovesDown);
	_showContextMenu = (krConfig->readBoolEntry("Immediate Context Menu", _ImmediateContextMenu) ? -1 : 500);
}

