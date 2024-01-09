/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSELECTIONMODE_H
#define KRSELECTIONMODE_H

// QtCore
#include <QString>

/**
 Every selection mode inherits this class, and has to implement init().
 Usage:
  KrSelectionMode::getSelectionHandler()->whateverFunctionYouNeed()

 \note You can call KrSelectionMode::resetSelectionHandler() if you want the
 selection mode to be re-read. This is useful after a configuration where you
 changed the selection mode. calling resetSelectionHandler() will cause the next
 call to getSelectionHandler() to (possibly) select a different mode.
*/
class KrSelectionMode
{
public:
    static KrSelectionMode *getSelectionHandlerForMode(const QString &mode);
    static KrSelectionMode *getSelectionHandler();
    static void resetSelectionHandler();

    virtual void init() = 0; // everyone must implement this in order to be a selection mode
    inline bool useQTSelection()
    {
        return _useQTSelection;
    }
    inline bool spaceMovesDown()
    {
        return _spaceMovesDown;
    }
    inline bool insertMovesDown()
    {
        return _insertMovesDown;
    }
    inline bool spaceCalculatesDiskSpace()
    {
        return _spaceCalculatesDiskSpace;
    }
    inline bool rightButtonSelects()
    {
        return _rightButtonSelects;
    }
    inline bool leftButtonSelects()
    {
        return _leftButtonSelects;
    }
    inline bool rightButtonPreservesSelection()
    {
        return _rightButtonPreservesSelection;
    }
    inline bool leftButtonPreservesSelection()
    {
        return _leftButtonPreservesSelection;
    }
    inline bool shiftCtrlRightButtonSelects()
    {
        return _shiftCtrlRightButtonSelects;
    }
    inline bool shiftCtrlLeftButtonSelects()
    {
        return _shiftCtrlLeftButtonSelects;
    }
    inline int showContextMenu()
    {
        return _showContextMenu;
    } // 0: no, -1: yes, n>0: after n milliseconds
    inline int resetSelectionItems()
    {
        return _resetSelectionItems;
    }

    virtual ~KrSelectionMode()
    {
    }

protected:
    bool _useQTSelection, _spaceMovesDown, _insertMovesDown, _spaceCalculatesDiskSpace;
    bool _rightButtonSelects, _leftButtonSelects, _rightButtonPreservesSelection;
    bool _leftButtonPreservesSelection, _shiftCtrlRightButtonSelects, _shiftCtrlLeftButtonSelects;
    int _showContextMenu;
    int _resetSelectionItems;
};

class KonqSelectionMode : public KrSelectionMode
{
public:
    void init() override
    {
        _useQTSelection = true;
        _spaceMovesDown = false;
        _insertMovesDown = true;
        _spaceCalculatesDiskSpace = false;
        _rightButtonSelects = true;
        _leftButtonSelects = true;
        _rightButtonPreservesSelection = false;
        _leftButtonPreservesSelection = false;
        _shiftCtrlRightButtonSelects = false;
        _shiftCtrlLeftButtonSelects = false;
        _showContextMenu = -1;
        _resetSelectionItems = false;
    }
};

class OriginalSelectionMode : public KrSelectionMode
{
public:
    void init() override
    {
        _useQTSelection = false;
        _spaceMovesDown = true;
        _insertMovesDown = true;
        _spaceCalculatesDiskSpace = true;
        _rightButtonSelects = true;
        _leftButtonSelects = true;
        _rightButtonPreservesSelection = false;
        _leftButtonPreservesSelection = false;
        _shiftCtrlRightButtonSelects = false;
        _shiftCtrlLeftButtonSelects = false;
        _showContextMenu = -1;
        _resetSelectionItems = false;
    }
};

class TCSelectionMode : public KrSelectionMode
{
public:
    void init() override
    {
        _useQTSelection = false;
        _spaceMovesDown = false;
        _insertMovesDown = true;
        _spaceCalculatesDiskSpace = true;
        _rightButtonSelects = true;
        _leftButtonSelects = false;
        _rightButtonPreservesSelection = true;
        _leftButtonPreservesSelection = false;
        _shiftCtrlRightButtonSelects = false;
        _shiftCtrlLeftButtonSelects = true;
        _showContextMenu = 500;
        _resetSelectionItems = false;
    }
};

class ErgonomicSelectionMode : public KrSelectionMode
{
public:
    void init() override
    {
        _useQTSelection = false;
        _spaceMovesDown = false;
        _insertMovesDown = true;
        _spaceCalculatesDiskSpace = true;
        _rightButtonSelects = false;
        _leftButtonSelects = false;
        _rightButtonPreservesSelection = true;
        _leftButtonPreservesSelection = true;
        _shiftCtrlRightButtonSelects = false;
        _shiftCtrlLeftButtonSelects = true;
        _showContextMenu = -1;
        _resetSelectionItems = false;
    }
};

class UserSelectionMode : public KrSelectionMode
{
public:
    void init() override;
};

#endif // KR_SELECTION_MODE_H
