#include "krselectionmode.h"
#include "../krusader.h"
#include "../defaults.h"

static KrSelectionMode *__currentSelectionMode = 0; // uninitiated, at first


KonqSelectionMode konqSelectionMode;
OriginalSelectionMode originalSelectionMode;
TCSelectionMode tcSelectionMode;
ErgonomicSelectionMode ergonomicSelectionMode;
UserSelectionMode userSelectionMode;

KrSelectionMode* KrSelectionMode::getSelectionHandlerForMode(const QString &mode)
{
	KrSelectionMode *res = NULL;
        bool isNum;
        int modenum = mode.toInt(&isNum);
	switch ( modenum ) {
		case 0:
			res = &originalSelectionMode;
			break;
		case 1:
			res = &konqSelectionMode;
			break;
		case 2:
			res = &tcSelectionMode;
			break;
		case 3:
			 //costom mode
			break;
		case 4:
			res = &ergonomicSelectionMode;
			break;
		default:
			break;
	}
        return res;
}

KrSelectionMode* KrSelectionMode::getSelectionHandler()
{
	if (__currentSelectionMode) { // don't check krConfig every time
		return __currentSelectionMode;
	} else { // nothing yet, set the correct one
		KConfigGroup group( krConfig, "Look&Feel" );
		QString mode = group.readEntry("Mouse Selection", QString("") );
		__currentSelectionMode = getSelectionHandlerForMode( mode );
		if ( __currentSelectionMode == NULL )
		{
			__currentSelectionMode = &userSelectionMode;
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

