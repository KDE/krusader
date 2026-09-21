/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <yanai@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krselectionmode.h"
#include "../defaults.h"
#include "../krglobal.h"

#include <KSharedConfig>

static KrSelectionMode *__currentSelectionMode = nullptr; // uninitiated, at first

KonqSelectionMode konqSelectionMode;
OriginalSelectionMode originalSelectionMode;
TCSelectionMode tcSelectionMode;
ErgonomicSelectionMode ergonomicSelectionMode;
UserSelectionMode userSelectionMode;

KrSelectionMode *KrSelectionMode::getSelectionHandlerForMode(const QString &mode)
{
    KrSelectionMode *res = nullptr;
    bool isNum;
    int modenum = mode.toInt(&isNum);
    switch (modenum) {
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
        // costom mode
        break;
    case 4:
        res = &ergonomicSelectionMode;
        break;
    default:
        break;
    }
    return res;
}

KrSelectionMode *KrSelectionMode::getSelectionHandler()
{
    if (__currentSelectionMode) { // don't check krConfig every time
        return __currentSelectionMode;
    } else { // nothing yet, set the correct one
        KConfigGroup group(krConfig, "Look&Feel");
        QString mode = group.readEntry("Mouse Selection", QString(""));
        __currentSelectionMode = getSelectionHandlerForMode(mode);
        if (__currentSelectionMode == nullptr) {
            __currentSelectionMode = &userSelectionMode;
        }
        // init and return
        __currentSelectionMode->init();
        return __currentSelectionMode;
    }
}

void KrSelectionMode::resetSelectionHandler()
{
    __currentSelectionMode = nullptr;
}

void UserSelectionMode::init()
{
    KConfigGroup group(krConfig, "Custom Selection Mode");
    _useQTSelection = group.readEntry("QT Selection", Defaults::qtSelection);
    _leftButtonSelects = group.readEntry("Left Selects", Defaults::leftSelects);
    _leftButtonPreservesSelection = group.readEntry("Left Preserves", Defaults::leftPreserves);
    _shiftCtrlLeftButtonSelects = group.readEntry("ShiftCtrl Left Selects", Defaults::shiftCtrlLeft);
    _rightButtonSelects = group.readEntry("Right Selects", Defaults::rightSelects);
    _rightButtonPreservesSelection = group.readEntry("Right Preserves", Defaults::rightPreserves);
    _shiftCtrlRightButtonSelects = group.readEntry("ShiftCtrl Right Selects", Defaults::shiftCtrlRight);
    _spaceMovesDown = group.readEntry("Space Moves Down", Defaults::spaceMovesDown);
    _spaceCalculatesDiskSpace = group.readEntry("Space Calc Space", Defaults::spaceCalcSpace);
    _insertMovesDown = group.readEntry("Insert Moves Down", Defaults::insertMovesDown);
    _showContextMenu = (group.readEntry("Immediate Context Menu", Defaults::immediateContextMenu) ? -1 : 500);
    _resetSelectionItems = group.readEntry("Reset Selection Items", Defaults::resetSelectionItems);
}
