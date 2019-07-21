/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
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

#include "kgadvanced.h"
#include "../defaults.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>

#include <KI18n/KLocalizedString>
#include <KIOCore/KProtocolInfo>

KgAdvanced::KgAdvanced(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    QGridLayout *kgAdvancedLayout = new QGridLayout(innerWidget);
    kgAdvancedLayout->setSpacing(6);

    //  -------------------------- GENERAL GROUPBOX ----------------------------------

    QGroupBox *generalGrp = createFrame(i18n("General"), innerWidget);
    QGridLayout *generalGrid = createGridLayout(generalGrp);

    KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
        //   cfg_class  cfg_name             default              text                                                        restart tooltip
    {
        {"Advanced", "AutoMount",          _AutoMount,          i18n("Automount filesystems"),                            false,  i18n("When stepping into a folder which is defined as a mount point in the <b>fstab</b>, try mounting it with the defined parameters.")}
    };

    KonfiguratorCheckBoxGroup *generals = createCheckBoxGroup(1, 0, generalSettings, 1, generalGrp);

    generalGrid->addWidget(generals, 1, 0);

    addLabel(generalGrid, 2, 0, i18n("MountMan will not (un)mount the following mount-points:"),
             generalGrp);
    KonfiguratorEditBox *nonMountPoints = createEditBox("Advanced", "Nonmount Points", _NonMountPoints, generalGrp, false);
    generalGrid->addWidget(nonMountPoints, 2, 1);


#ifdef BSD
    generals->find("AutoMount")->setEnabled(false);     /* disable AutoMount on BSD */
#endif

    kgAdvancedLayout->addWidget(generalGrp, 0 , 0);

    //  ----------------------- CONFIRMATIONS GROUPBOX -------------------------------

    QGroupBox *confirmGrp = createFrame(i18n("Confirmations"), innerWidget);
    QGridLayout *confirmGrid = createGridLayout(confirmGrp);

    addLabel(confirmGrid, 0, 0, i18n("\nRequest user confirmation for the following operations:\n"),
             confirmGrp);

    KONFIGURATOR_CHECKBOX_PARAM confirmations[] =
        //   cfg_class  cfg_name                default             text                                          restart ToolTip
    {{"Advanced", "Confirm Unempty Dir",   _ConfirmUnemptyDir, i18n("Deleting non-empty folders"),       false,  ""},
        {"Advanced", "Confirm Delete",        _ConfirmDelete,     i18n("Deleting files"),                   false,  ""},
        {"Advanced", "Confirm Copy",          _ConfirmCopy,       i18n("Copying files"),                    false,  ""},
        {"Advanced", "Confirm Move",          _ConfirmMove,       i18n("Moving files"),                     false,  ""},
        {"Advanced", "Confirm Feed to Listbox",  _ConfirmFeedToListbox, i18n("Confirm feed to listbox"), false, i18n("Ask for a result name when feeding items to the listbox. By default the standard value is used.")},
        {"Notification Messages", "Confirm Remove UserAction", true, i18n("Removing Useractions"), false,  ""}
    };

    KonfiguratorCheckBoxGroup *confWnd = createCheckBoxGroup(2, 0, confirmations, 6, confirmGrp);

    confirmGrid->addWidget(confWnd, 1, 0);

    kgAdvancedLayout->addWidget(confirmGrp, 1 , 0);


    //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

    QGroupBox *fineTuneGrp = createFrame(i18n("Fine-Tuning"), innerWidget);
    QGridLayout *fineTuneGrid = createGridLayout(fineTuneGrp);
    fineTuneGrid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QLabel *label = new QLabel(i18n("Icon cache size (KB):"), fineTuneGrp);
    label->setWhatsThis(i18n("The icon cache size influences how fast the contents of a panel can be displayed. However, too large a cache might consume your memory."));
    fineTuneGrid->addWidget(label, 0, 0);
    KonfiguratorSpinBox *spinBox = createSpinBox("Advanced", "Icon Cache Size", _IconCacheSize,
                                   1, 8192, fineTuneGrp, false);
    spinBox->setWhatsThis(i18n("The icon cache size influences how fast the contents of a panel can be displayed. However, too large a cache might consume your memory."));
    spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fineTuneGrid->addWidget(spinBox, 0, 1);

    addLabel(fineTuneGrid, 1, 0, i18n("Arguments of updatedb:"),
             fineTuneGrp);
    KonfiguratorEditBox *updatedbArgs = createEditBox("Locate", "UpdateDB Arguments", "", fineTuneGrp, false);
    fineTuneGrid->addWidget(updatedbArgs, 1, 1);

    kgAdvancedLayout->addWidget(fineTuneGrp, 2 , 0);
}

