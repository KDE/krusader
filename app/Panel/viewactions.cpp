/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewactions.h"

#include "../krmainwindow.h"
#include "PanelView/krview.h"

#include <KLocalizedString>
#include <KToggleAction>

ViewActions::ViewActions(QObject *parent, KrMainWindow *mainWindow)
    : ActionsBase(parent, mainWindow)
{
    // zoom
    actZoomIn = action(i18n("Zoom In"), "zoom-in", 0, SLOT(zoomIn()), "zoom_in");
    actZoomOut = action(i18n("Zoom Out"), "zoom-out", 0, SLOT(zoomOut()), "zoom_out");
    actDefaultZoom = action(i18n("Default Zoom"), "zoom-original", 0, SLOT(defaultZoom()), "default_zoom");

    // filter
    action(i18n("&All Files"), nullptr, Qt::SHIFT | Qt::Key_F10, SLOT(allFilter()), "all files");
    // actExecFilter = new QAction( i18n( "&Executables" ), SHIFT | Qt::Key_F11,
    //                              SLOTS, SLOT(execFilter()), actionCollection(), "exec files" );
    action(i18n("&Custom"), nullptr, Qt::SHIFT | Qt::Key_F12, SLOT(customFilter()), "custom files");

    // selection
    actSelect = action(i18n("Select &Group..."), "edit-select", Qt::CTRL | Qt::Key_Plus, SLOT(markGroup()), "select group");
    actSelectAll = action(i18n("&Select All"), "edit-select-all", Qt::ALT | Qt::Key_Plus, SLOT(markAll()), "select all");
    actUnselect = action(i18n("&Unselect Group..."), "kr_unselect", Qt::CTRL | Qt::Key_Minus, SLOT(unmarkGroup()), "unselect group");
    actUnselectAll = action(i18n("U&nselect All"), "edit-select-none", Qt::ALT | Qt::Key_Minus, SLOT(unmarkAll()), "unselect all");
    actInvert = action(i18n("&Invert Selection"), "edit-select-invert", Qt::ALT | Qt::Key_Asterisk, SLOT(invertSelection()), "invert");
    actRestoreSelection = action(i18n("Restore Selection"), nullptr, 0, SLOT(restoreSelection()), "restore_selection");
    actMarkSameBaseName = action(i18n("Select Files with the Same Name"), nullptr, 0, SLOT(markSameBaseName()), "select_same_base_name");
    actMarkSameExtension = action(i18n("Select Files with the Same Extension"), nullptr, 0, SLOT(markSameExtension()), "select_same_extension");

    // other stuff
    action(i18n("Show View Options Menu"), nullptr, 0, SLOT(showOptionsMenu()), "show_view_options_menu");
    action(i18n("Set Focus to the Panel"), nullptr, 0, SLOT(focusPanel()), "focus_panel");
    action(i18n("Apply settings to other tabs"), nullptr, 0, SLOT(applySettingsToOthers()), "view_apply_settings_to_others");
    actTogglePreviews = toggleAction(i18n("Show Previews"), nullptr, 0, SLOT(togglePreviews(bool)), "toggle previews");
    QAction *actSaveaveDefaultSettings = action(i18n("Save settings as default"), nullptr, 0, SLOT(saveDefaultSettings()), "view_save_default_settings");

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
    view()->changeSelection(KrQuery("*"), true);
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
    qsizetype idx = KrView::iconSizes.indexOf(view()->fileIconSize());
    actZoomOut->setEnabled(idx > 0);
    actZoomIn->setEnabled(idx < (KrView::iconSizes.count() - 1));
    actRestoreSelection->setEnabled(view()->canRestoreSelection());
    actTogglePreviews->setChecked(view()->previewsShown());
}
