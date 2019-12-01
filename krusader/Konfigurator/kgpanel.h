/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef KGPANEL_H
#define KGPANEL_H

#include "konfiguratorpage.h"

class KrTreeWidget;
class KrViewInstance;

class KgPanel : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgPanel(bool first, QWidget* parent = nullptr);

    int activeSubPage() override;

protected:
    KonfiguratorCheckBoxGroup *panelToolbarButtonsCheckboxes;
    KonfiguratorCheckBoxGroup *buttonsCheckboxes;
    KonfiguratorRadioButtons  *mouseRadio;
    KonfiguratorCheckBoxGroup *mouseCheckboxes;
    KrTreeWidget* mousePreview;

protected slots:
    void slotEnablePanelToolbar();
    void slotSelectionModeChanged();
    void slotMouseCheckBoxChanged();

private:
    void setupGeneralTab();
    void setupPanelTab();
    void setupButtonsTab();
    void setupMouseModeTab();
    void setupMediaMenuTab();
    void setupLayoutTab();
    void setupView(KrViewInstance *instance, QWidget *parent);
    QTabWidget *tabWidget;
};

#endif /* __KGPANEL_H__ */
