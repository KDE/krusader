/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2010-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

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
    //zoom 
    void zoomIn();
    void zoomOut();
    void defaultZoom();

    //filter
    void allFilter();
    //void execFilter();
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
