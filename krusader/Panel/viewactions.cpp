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

#include "PanelView/krview.h"
#include "../krmainwindow.h"

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KToggleAction>

ViewActions::ViewActions(QObject *parent, KrMainWindow *mainWindow) :
    ActionsBase(parent, mainWindow)
{
    // zoom
    actZoomIn = action(i18n("Zoom In"), "zoom-in", 0, SLOT(zoomIn()), "zoom_in");
    actZoomOut = action(i18n("Zoom Out"), "zoom-out", 0, SLOT(zoomOut()), "zoom_out");
    actDefaultZoom = action(i18n("Default Zoom"), "zoom-original", 0, SLOT(defaultZoom()), "default_zoom");

    // filter
    action(i18n("&All Files"), 0, Qt::SHIFT + Qt::Key_F10, SLOT(allFilter()), "all files");
    //actExecFilter = new QAction( i18n( "&Executables" ), SHIFT + Qt::Key_F11,
    //                             SLOTS, SLOT(execFilter()), actionCollection(), "exec files" );
    action(i18n("&Custom"), 0, Qt::SHIFT + Qt::Key_F12, SLOT(customFilter()), "custom files");

    // selection
    actSelect = action(i18n("Select &Group..."), "edit-select", Qt::CTRL + Qt::Key_Plus, SLOT(markGroup()), "select group");
    actSelectAll = action(i18n("&Select All"), "edit-select-all", Qt::ALT + Qt::Key_Plus, SLOT(markAll()), "select all");
    actUnselect = action(i18n("&Unselect Group..."), "kr_unselect", Qt::CTRL + Qt::Key_Minus, SLOT(unmarkGroup()), "unselect group");
    actUnselectAll = action(i18n("U&nselect All"), "edit-select-none", Qt::ALT + Qt::Key_Minus, SLOT(unmarkAll()), "unselect all");
    actInvert = action(i18n("&Invert Selection"), "edit-select-invert", Qt::ALT + Qt::Key_Asterisk, SLOT(invertSelection()), "invert");
    actRestoreSelection = action(i18n("Restore Selection"), 0, 0, SLOT(restoreSelection()), "restore_selection");
    actMarkSameBaseName = action(i18n("Select Files with the Same Name"), 0, 0, SLOT(markSameBaseName()), "select_same_base_name");
    actMarkSameExtension = action(i18n("Select Files with the Same Extension"), 0, 0, SLOT(markSameExtension()), "select_same_extension");

    // other stuff
    action(i18n("Show View Options Menu"), 0, 0, SLOT(showOptionsMenu()), "show_view_options_menu");
    action(i18n("Set Focus to the Panel"), 0, 0, SLOT(focusPanel()), "focus_panel");
    action(i18n("Apply settings to other tabs"), 0, 0, SLOT(applySettingsToOthers()), "view_apply_settings_to_others");
    actTogglePreviews = toggleAction(i18n("Show Previews"), 0, 0, SLOT(togglePreviews(bool)), "toggle previews");
    QAction *actSaveaveDefaultSettings = action(i18n("Save settings as default"), 0, 0, SLOT(saveDefaultSettings()), "view_save_default_settings");

    // tooltips
    actSelect->setToolTip(i18n("Select group"));
    actSelectAll->setToolTip(i18n("Select all files in the current folder"));
    actUnselectAll->setToolTip(i18n("Unselect all"));
    actSaveaveDefaultSettings->setToolTip(i18n("Save settings as default for new instances of this view type"));
}

inline KrView *ViewActions::view()
{
    return _mainWindow->activeView();
}

// zoom

void ViewActions::zoomIn()
{
    view()->zoomIn();
}

void ViewActions::zoomOut()
{
    view()->zoomOut();
}

void ViewActions::defaultZoom()
{
    view()->setDefaultFileIconSize();
}

// filter

void ViewActions::allFilter()
{
    view()->setFilter(KrViewProperties::All);
}
#if 0
void ViewActions::execFilter()
{
    view()->setFilter(KrViewProperties::All);
}
#endif
void ViewActions::customFilter()
{
    view()->setFilter(KrViewProperties::Custom);
}

void ViewActions::showOptionsMenu()
{
    view()->showContextMenu();
}

// selection

void ViewActions::markAll()
{
    view()->changeSelection(KRQuery("*"), true);
}

void ViewActions::unmarkAll()
{
    view()->unselectAll();
}

void ViewActions::markGroup()
{
    view()->customSelection(true);
}

void ViewActions::unmarkGroup()
{
    view()->customSelection(false);
}

void ViewActions::invertSelection()
{
    view()->invertSelection();
}

void ViewActions::restoreSelection()
{
    view()->restoreSelection();
}

void ViewActions::markSameBaseName()
{
    view()->markSameBaseName();
}

void ViewActions::markSameExtension()
{
    view()->markSameExtension();
}

// other stuff

void ViewActions::saveDefaultSettings()
{
    view()->saveDefaultSettings();
}

void ViewActions::applySettingsToOthers()
{
    view()->applySettingsToOthers();
}

void ViewActions::focusPanel()
{
    view()->widget()->setFocus();
}

void ViewActions::togglePreviews(bool show)
{
    view()->showPreviews(show);
}

void ViewActions::refreshActions()
{
    actDefaultZoom->setEnabled(view()->defaultFileIconSize() != view()->fileIconSize());
    int idx = KrView::iconSizes.indexOf(view()->fileIconSize());
    actZoomOut->setEnabled(idx > 0);
    actZoomIn->setEnabled(idx < (KrView::iconSizes.count() - 1));
    actRestoreSelection->setEnabled(view()->canRestoreSelection());
    actTogglePreviews->setChecked(view()->previewsShown());
}
