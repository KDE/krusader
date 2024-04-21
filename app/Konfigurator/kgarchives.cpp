/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgarchives.h"

// QtCore
#include <QPointer>
// QtWidgets
#include <QGridLayout>
#include <QPushButton>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../Archive/krarchandler.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "krresulttable.h"
#include "krresulttabledialog.h"
#include "searchobject.h"

KgArchives::KgArchives(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgArchivesLayout = new QGridLayout(innerWidget);
    kgArchivesLayout->setSpacing(6);

    //  ------------------------ KRARC GROUPBOX --------------------------------

    QGroupBox *krarcGrp = createFrame(i18n("krarc ioslave"), innerWidget);
    QGridLayout *krarcGrid = createGridLayout(krarcGrp);

    KONFIGURATOR_CHECKBOX_PARAM krarcOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
        {{"kio_krarc", "EnableWrite", false, i18n("Enable Write Support"), false, i18n("Enable writing to archives using the krarc ioslave.")}};

    KonfiguratorCheckBoxGroup *krarcCheckBoxes = createCheckBoxGroup(1, 0, krarcOptions, 1, krarcGrp);

    krarcGrid->addWidget(krarcCheckBoxes, 1, 0);
    krarcGrid->addWidget(new QLabel(i18n("<b>Caution when moving into archives:</b><br/>"
                                         "<b>Failure during the process might result in data loss.</b><br/>"
                                         "<b>Moving archives into themselves will delete them.</b>"),
                                    krarcGrp),
                         2,
                         0);

    kgArchivesLayout->addWidget(krarcGrp, 1, 0);

    //  ------------------------ BROWSE GROUPBOX --------------------------------

    QGroupBox *browseGrp = createFrame(i18n("Archives handling"), innerWidget);
    QGridLayout *browseGrid = createGridLayout(browseGrp);

    KONFIGURATOR_CHECKBOX_PARAM browseOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
        {{"Archives",
          "ArchivesAsDirectories",
          _ArchivesAsDirectories,
          i18n("Browse Archives As Folders"),
          false,
          i18n("Krusader will browse archives as folders.")}};

    KonfiguratorCheckBoxGroup *browseCheckBoxes = createCheckBoxGroup(1, 0, browseOptions, 1, browseGrp);

    browseGrid->addWidget(browseCheckBoxes, 1, 0);

    kgArchivesLayout->addWidget(browseGrp, 2, 0);

    //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

    QGroupBox *fineTuneGrp = createFrame(i18n("Fine-Tuning"), innerWidget);
    QGridLayout *fineTuneGrid = createGridLayout(fineTuneGrp);

    KONFIGURATOR_CHECKBOX_PARAM finetuners[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
        {{"Archives", "Test Archives", _TestArchives, i18n("Test archive after packing"), false, i18n("Check the archive's integrity after packing it.")},
         {"Archives",
          "Test Before Unpack",
          _TestBeforeUnpack,
          i18n("Test archive before unpacking"),
          false,
          i18n("Some corrupted archives might cause a crash; therefore, testing is suggested.")}};

    KonfiguratorCheckBoxGroup *finetunes = createCheckBoxGroup(1, 0, finetuners, 2, fineTuneGrp);

    disableNonExistingPackers();
    fineTuneGrid->addWidget(finetunes, 1, 0);

    kgArchivesLayout->addWidget(fineTuneGrp, 3, 0);

    if (first)
        slotAutoConfigure();
}

void KgArchives::slotAutoConfigure()
{
    QPointer<KrResultTableDialog> dlg = new KrResultTableDialog(this,
                                                                KrResultTableDialog::Archiver,
                                                                i18n("Search results"),
                                                                i18n("Searching for packers..."),
                                                                "utilities-file-archiver",
                                                                i18n("Make sure to install new packers in your <code>$PATH</code> (e.g. /usr/bin)"));
    dlg->exec();

    disableNonExistingPackers();

    delete dlg;
}

void KgArchives::disableNonExistingPackers()
{
    KConfigGroup group(krConfig, "Archives");
    group.writeEntry("Supported Packers", KrArcHandler::supportedPackers());
}
