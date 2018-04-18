/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#include "kgstartup.h"
#include "../defaults.h"
#include "../GUI/profilemanager.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>

#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>

KgStartup::KgStartup(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent), profileCombo(0)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    QGridLayout *kgStartupLayout = new QGridLayout(innerWidget);
    kgStartupLayout->setSpacing(6);

    //  --------------------------- PANELS GROUPBOX ----------------------------------

    QGroupBox *panelsGrp = createFrame(i18n("General"), innerWidget);
    QGridLayout *panelsGrid = createGridLayout(panelsGrp);

    QString s = "<p><img src='toolbar|user-identity'></p>" + i18n("Defines the panel profile used at startup. A panel profile contains:<ul><li>all the tabs paths</li><li>the current tab</li><li>the active panel</li></ul><b>&lt;Last session&gt;</b> is a special panel profile which is saved automatically when Krusader is closed.");
    QLabel *label = addLabel(panelsGrid, 0, 0, i18n("Startup profile:"), panelsGrp);
    label->setWhatsThis(s);
    panelsGrp->setWhatsThis(s);

    QStringList profileList = ProfileManager::availableProfiles("Panel");
    profileList.push_front(i18n("<Last session>"));

    const int profileListSize = profileList.size();
    KONFIGURATOR_NAME_VALUE_PAIR *comboItems = new KONFIGURATOR_NAME_VALUE_PAIR[ profileListSize ];
    for (int i = 0; i != profileListSize; i++)
        comboItems[ i ].text = comboItems[ i ].value = profileList [ i ];
    comboItems[ 0 ].value = "";

    profileCombo = createComboBox("Startup", "Starter Profile Name", comboItems[ 0 ].value, comboItems, profileListSize, panelsGrp, false, false);
    profileCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panelsGrid->addWidget(profileCombo, 0, 1);

    delete [] comboItems;

    //------------------------------------------------
    panelsGrid->addWidget(createLine(panelsGrp), 1, 0, 1, 2);

    KONFIGURATOR_CHECKBOX_PARAM settings[] = { //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel", "Show splashscreen",  _ShowSplashScreen, i18n("Show splashscreen"), false,  i18n("Display a splashscreen when starting Krusader.") },
        {"Look&Feel", "Single Instance Mode", _SingleInstanceMode, i18n("Single instance mode"), false,  i18n("Only one Krusader instance is allowed to run.") }
    };

    KonfiguratorCheckBoxGroup* cbs = createCheckBoxGroup(2, 0, settings, 2 /* settings count */, panelsGrp);
    panelsGrid->addWidget(cbs, 2, 0, 1, 2);

    QHBoxLayout *iconThemeLayout = new QHBoxLayout();
    QLabel *iconThemeLabel = new QLabel(i18n("Fallback Icon Theme:"));
    iconThemeLabel->setWhatsThis(i18n("Whenever icon is not found in system icon theme, "
                                      "this theme will be used as a fallback. "
                                      "If fallback theme doesn't contain the icon, "
                                      "Breeze or Oxygen will be used if any of these are present."));
    iconThemeLayout->addWidget(iconThemeLabel);
    KonfiguratorEditBox *iconThemeEditBox = createEditBox("Startup", "Fallback Icon Theme", "",
                                                          panelsGrp, true);
    iconThemeLayout->addWidget(iconThemeEditBox);
    iconThemeLayout->addStretch(1);
    panelsGrid->addLayout(iconThemeLayout, 3, 0, 1, 2);

    kgStartupLayout->addWidget(panelsGrp, 0, 0);

    //  ------------------------ USERINTERFACE GROUPBOX ------------------------------

    QGroupBox *uiGrp = createFrame(i18n("User Interface"), innerWidget);
    QGridLayout *uiGrid = createGridLayout(uiGrp);

    KONFIGURATOR_CHECKBOX_PARAM uiSettings[] = {
        // cfg_class cfg_name default text restart tooltip
        {"Startup", "Remember Position", _RememberPos,i18n("Save last position, size and panel settings"), false,
            i18n("<p>At startup, the main window will resize itself to the size it was when last shutdown. "
                 "It will also appear in the same location of the screen, having panels sorted and aligned as they were before.</p> "
                 "<p>If this option is disabled, you can use the menu <i>Window -> Save Position</i> option "
                 "to manually set the main window's size and position at startup.</p>") },
        {"Startup", "Update Default Panel Settings", _RememberPos, i18n("Update default panel settings"), true,
            i18n("When settings of a panel are changed, save them as the default for new panels of the same type.") },
        {"Startup", "Start To Tray", _StartToTray, i18n("Start to tray"), false,
            i18n("Krusader starts to tray, without showing the main window") },
    };

    KonfiguratorCheckBoxGroup *uiSettingsGroup = createCheckBoxGroup(1, 0, uiSettings, 3, uiGrp);
    uiGrid->addWidget(uiSettingsGroup, 1, 0);

   KONFIGURATOR_CHECKBOX_PARAM uiCheckBoxes[] = { //   cfg_class, cfg_name, default, text, restart, ToolTip
        {"Startup", "UI Save Settings", _UiSave, i18n("Save component settings on exit"), false,
            i18n("Check the state of the user interface components and restore them to their condition when last shutdown.") },
        {"Startup", "Show FN Keys",  _ShowFNkeys, i18n("Show function keys"), false, i18n("Function keys will be visible after startup.") },
        {"Startup", "Show Cmd Line",  _ShowCmdline,  i18n("Show command line"), false, i18n("Command line will be visible after startup.") },
        {"Startup", "Show Terminal Emulator", _ShowTerminalEmulator, i18n("Show embedded terminal"),  false,
            i18n("Embedded terminal will be visible after startup.") },
    };

    uiCbGroup = createCheckBoxGroup(1, 0, uiCheckBoxes, 4, uiGrp);
    connect(uiCbGroup->find("UI Save Settings"), SIGNAL(stateChanged(int)), this, SLOT(slotDisable()));

    uiGrid->addWidget(uiCbGroup, 2, 0);

    slotDisable();

    kgStartupLayout->addWidget(uiGrp, 1, 0);
}

void KgStartup::slotDisable()
{
    bool isUiSave   = !uiCbGroup->find("UI Save Settings")->isChecked();

    int i = 1;
    while (uiCbGroup->find(i))
        uiCbGroup->find(i++)->setEnabled(isUiSave);
}

