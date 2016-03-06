/***************************************************************************
                         kgarchives.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "kgarchives.h"

#include <QtCore/QPointer>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>

#include "krresulttable.h"
#include "krresulttabledialog.h"
#include "searchobject.h"
#include "../defaults.h"
#include "../VFS/krarchandler.h"

KgArchives::KgArchives(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    QGridLayout *kgArchivesLayout = new QGridLayout(innerWidget);
    kgArchivesLayout->setSpacing(6);

    //  ------------------------ KRARC GROUPBOX --------------------------------

    QGroupBox *krarcGrp = createFrame(i18n("krarc ioslave"), innerWidget);
    QGridLayout *krarcGrid = createGridLayout(krarcGrp);

    KONFIGURATOR_CHECKBOX_PARAM krarcOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"kio_krarc", "EnableWrite", false, i18n("Enable Write Support"), false, i18n("Enable writing to archives using the krarc ioslave.")}
    };

    KonfiguratorCheckBoxGroup *krarcCheckBoxes = createCheckBoxGroup(1, 0, krarcOptions, 1, krarcGrp);

    krarcGrid->addWidget(krarcCheckBoxes, 1, 0);
    krarcGrid->addWidget(
            new QLabel(i18n("<b>Caution when moving into archives:</b><br/>"
                            "<b>Failure during the process might result in data loss.</b><br/>"
                            "<b>Moving archives into themselves will delete them.</b>"), krarcGrp),
            2, 0);

    kgArchivesLayout->addWidget(krarcGrp, 1 , 0);

    //  ------------------------ BROWSE GROUPBOX --------------------------------

    QGroupBox *browseGrp = createFrame(i18n("archives handling"), innerWidget);
    QGridLayout *browseGrid = createGridLayout(browseGrp);

    KONFIGURATOR_CHECKBOX_PARAM browseOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"Archives", "ArchivesAsDirectories", _ArchivesAsDirectories, i18n("Browse Archives As Directories"), false, i18n("Krusader will browse archives as folders.")}
    };

    KonfiguratorCheckBoxGroup *browseCheckBoxes = createCheckBoxGroup(1, 0, browseOptions, 1, browseGrp);

    browseGrid->addWidget(browseCheckBoxes, 1, 0);

    kgArchivesLayout->addWidget(browseGrp, 2 , 0);

    //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

    QGroupBox *fineTuneGrp = createFrame(i18n("Fine-Tuning"), innerWidget);
    QGridLayout *fineTuneGrid = createGridLayout(fineTuneGrp);

    KONFIGURATOR_CHECKBOX_PARAM finetuners[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"Archives", "Test Archives",           _TestArchives,    i18n("Test archive after packing"), false,  i18n("Check the archive's integrity after packing it.")},
        {"Archives", "Test Before Unpack",      _TestBeforeUnpack, i18n("Test archive before unpacking"), false,  i18n("Some corrupted archives might cause a crash; therefore, testing is suggested.")}
    };

    KonfiguratorCheckBoxGroup *finetunes = createCheckBoxGroup(1, 0, finetuners, 2, fineTuneGrp);

    disableNonExistingPackers();
    fineTuneGrid->addWidget(finetunes, 1, 0);

    kgArchivesLayout->addWidget(fineTuneGrp, 3 , 0);

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
    group.writeEntry("Supported Packers", KRarcHandler::supportedPackers());
}


