/***************************************************************************
                          viewactions.cpp
                       -------------------
copyright            : (C) 2010 by Jan Lepper
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                               S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "viewactions.h"

#include "krview.h"
#include "../krmainwindow.h"

#include <klocale.h>

ViewActions::ViewActions(QObject *parent, KrMainWindow *mainWindow) :
    ActionsBase(parent, mainWindow)
{
    actZoomIn = action(i18n("Zoom In"), "zoom-in", 0, SLOT(zoomIn()), "zoom_in");
    actZoomOut = action(i18n("Zoom Out"), "zoom-out", 0, SLOT(zoomOut()), "zoom_out");
    actDefaultZoom = action(i18n("Default Zoom"), "zoom-original", 0, SLOT(defaultZoom()), "default_zoom");
    actShowViewOptionsMenu = action(i18n("Show View Options Menu"), 0, 0, SLOT(showOptionsMenu()), "show_view_options_menu");
    actViewSaveDefaultSettings = action(i18n("Save settings as default"), 0, 0, SLOT(saveDefaultSettings()), "view_save_default_settings");
    actFocusPanel = action(i18n("Set Focus to the Panel"), 0, Qt::Key_Escape, SLOT(focusPanel()), "focus_panel");
    action(i18n("Apply settings to other tabs"), 0, 0, SLOT(applySettingsToOthers()), "view_apply_settings_to_others");

    actViewSaveDefaultSettings->setToolTip(i18n("Save settings as default for new instances of this view type"));
}

void ViewActions::zoomIn()
{
    _mainWindow->activeView()->zoomIn();
}

void ViewActions::zoomOut()
{
    _mainWindow->activeView()->zoomOut();
}

void ViewActions::defaultZoom()
{
    _mainWindow->activeView()->setDefaultFileIconSize();
}

void ViewActions::showOptionsMenu()
{
    _mainWindow->activeView()->showContextMenu();
}

void ViewActions::saveDefaultSettings()
{
    _mainWindow->activeView()->saveDefaultSettings();
}

void ViewActions::applySettingsToOthers()
{
    _mainWindow->activeView()->applySettingsToOthers();
}

void ViewActions::focusPanel()
{
    _mainWindow->activeView()->widget()->setFocus();
}
