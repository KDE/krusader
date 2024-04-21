/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgadvanced.h"
#include "../defaults.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>

#include <KLocalizedString>
#include <KProtocolInfo>

KgAdvanced::KgAdvanced(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgAdvancedLayout = new QGridLayout(innerWidget);
    kgAdvancedLayout->setSpacing(6);

    //  -------------------------- GENERAL GROUPBOX ----------------------------------

    QGroupBox *generalGrp = createFrame(i18n("General"), innerWidget);
    QGridLayout *generalGrid = createGridLayout(generalGrp);

    KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
        //   cfg_class  cfg_name             default              text                                                        restart tooltip
        {{"Advanced",
          "AutoMount",
          _AutoMount,
          i18n("Automount filesystems"),
          false,
          i18n("When stepping into a folder which is defined as a mount point in the <b>fstab</b>, try mounting it with the defined parameters.")}};

    KonfiguratorCheckBoxGroup *generals = createCheckBoxGroup(1, 0, generalSettings, 1, generalGrp);

    generalGrid->addWidget(generals, 1, 0);

    QLabel *labelNonMount = addLabel(generalGrid, 2, 0, i18n("MountMan will not (un)mount the following mount-points:"), generalGrp);
    KonfiguratorEditBox *nonMountPoints = createEditBox("Advanced", "Nonmount Points", _NonMountPoints, labelNonMount, generalGrp, false);
    generalGrid->addWidget(nonMountPoints, 2, 1);

#ifdef BSD
    generals->find("AutoMount")->setEnabled(false); /* disable AutoMount on BSD */
#endif

    kgAdvancedLayout->addWidget(generalGrp, 0, 0);

    //  ----------------------- CONFIRMATIONS GROUPBOX -------------------------------

    QGroupBox *confirmGrp = createFrame(i18n("Confirmations"), innerWidget);
    QGridLayout *confirmGrid = createGridLayout(confirmGrp);

    addLabel(confirmGrid, 0, 0, i18n("\nRequest user confirmation for the following operations:\n"), confirmGrp);

    KONFIGURATOR_CHECKBOX_PARAM confirmations[] =
        //   cfg_class  cfg_name                default             text                                          restart ToolTip
        {{"Advanced", "Confirm Unempty Dir", _ConfirmUnemptyDir, i18n("Deleting non-empty folders"), false, ""},
         {"Advanced", "Confirm Delete", _ConfirmDelete, i18n("Deleting files"), false, ""},
         {"Advanced", "Confirm Copy", _ConfirmCopy, i18n("Copying files"), false, ""},
         {"Advanced", "Confirm Move", _ConfirmMove, i18n("Moving files"), false, ""},
         {"Advanced",
          "Confirm Feed to Listbox",
          _ConfirmFeedToListbox,
          i18n("Confirm feed to listbox"),
          false,
          i18n("Ask for a result name when feeding items to the listbox. By default the standard value is used.")},
         {"Notification Messages", "Confirm Remove UserAction", true, i18n("Removing Useractions"), false, ""}};

    KonfiguratorCheckBoxGroup *confWnd = createCheckBoxGroup(2, 0, confirmations, 6, confirmGrp);

    confirmGrid->addWidget(confWnd, 1, 0);

    kgAdvancedLayout->addWidget(confirmGrp, 1, 0);

    //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

    QGroupBox *fineTuneGrp = createFrame(i18n("Fine-Tuning"), innerWidget);
    QGridLayout *fineTuneGrid = createGridLayout(fineTuneGrp);
    fineTuneGrid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    const QString cacheTip =
        i18n("The icon cache size influences how fast the contents of a panel can be displayed. However, too large a cache might consume your memory.");
    QLabel *label = new QLabel(i18n("Icon cache size (KB):"), fineTuneGrp);
    fineTuneGrid->addWidget(label, 0, 0);
    KonfiguratorSpinBox *spinBox = createSpinBox("Advanced", "Icon Cache Size", _IconCacheSize, 1, 8192, label, fineTuneGrp, false, cacheTip);
    spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fineTuneGrid->addWidget(spinBox, 0, 1);

    QLabel *labelArgUpdate = addLabel(fineTuneGrid, 1, 0, i18n("Arguments of updatedb:"), fineTuneGrp);
    KonfiguratorEditBox *updatedbArgs = createEditBox("Locate", "UpdateDB Arguments", "", labelArgUpdate, fineTuneGrp, false);
    fineTuneGrid->addWidget(updatedbArgs, 1, 1);

    kgAdvancedLayout->addWidget(fineTuneGrp, 2, 0);
}
