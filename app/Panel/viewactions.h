/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include "../actionsbase.h"

// QtWidgets
#include <QAction>

class KrView;

class ViewActions : public ActionsBase
{
    Q_OBJECT
public:
    ViewActions(QObject *parent, KrMainWindow *mainWindow);

public slots:
    // zoom
    void zoomIn();
    void zoomOut();
    void defaultZoom();

    // filter
    void allFilter();
    // void execFilter();
    void customFilter();

    // selection
    void markAll();
    void unmarkAll();
    void markGroup();
    void unmarkGroup();
    void invertSelection();
    void restoreSelection();
    void markSameBaseName();
    void markSameExtension();

    void showOptionsMenu();
    void saveDefaultSettings();
    void applySettingsToOthers();
    void focusPanel();
    void togglePreviews(bool show);

    void refreshActions();

public:
    QAction *actZoomIn, *actZoomOut, *actDefaultZoom;
    QAction *actSelect, *actUnselect, *actSelectAll, *actUnselectAll, *actInvert, *actRestoreSelection;
    QAction *actMarkSameBaseName, *actMarkSameExtension;
    KToggleAction *actTogglePreviews;

protected:
    KrView *view();
};

#endif // VIEWACTIONS_H
