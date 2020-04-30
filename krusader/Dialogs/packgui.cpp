/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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

#include "packgui.h"
#include "../krglobal.h"
#include "../defaults.h"

// QtCore
#include <QStringList>
// QtWidgets
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>

#include "../GUI/krhistorycombobox.h"

#define PS(x) lst.contains(x)>0

// clear the statics first
QString PackGUI::filename = nullptr;
QString PackGUI::destination = nullptr;
QString PackGUI::type = nullptr;
QMap<QString, QString> PackGUI::extraProps;

PackGUI::PackGUI(const QString& defaultName, const QString& defaultPath, int noOfFiles, const QString& filename) :
        PackGUIBase(nullptr)
{
    // first, fill the WhatToPack textfield with information
    if (noOfFiles == 1)
        TextLabel1->setText(i18n("Pack %1", filename));
    else
        TextLabel1->setText(i18np("Pack %1 file", "Pack %1 files", noOfFiles));

    // now, according to the Konfigurator, fill the combobox with the information
    // about what kind of packing we can do
    KConfigGroup group(krConfig, "Archives");
    QStringList lst = group.readEntry("Supported Packers", QStringList());
    // now, clear the type combo and begin...
    typeData->clear();
    if (PS("tar")) typeData->addItem("tar");
    if (PS("tar") && PS("gzip")) typeData->addItem("tar.gz");
    if (PS("tar") && PS("bzip2")) typeData->addItem("tar.bz2");
    if (PS("tar") && PS("lzma")) typeData->addItem("tar.lzma");
    if (PS("tar") && PS("xz")) typeData->addItem("tar.xz");
    if (PS("zip")) typeData->addItem("zip");
    if (PS("zip")) typeData->addItem("cbz");
    if (PS("rar")) typeData->addItem("rar");
    if (PS("rar")) typeData->addItem("cbr");
    if (PS("lha")) typeData->addItem("lha");
    if (PS("arj")) typeData->addItem("arj");
    if (PS("7z")) typeData->addItem("7z");
    // set the last used packer as the top one
    QString tmp = group.readEntry("lastUsedPacker", QString());
    if (!tmp.isEmpty()) {
        for (int i = 0; i < typeData->count(); ++i)
            if (typeData->itemText(i) == tmp) {
                typeData->removeItem(i);
                typeData->insertItem(0, tmp);
                typeData->setCurrentIndex(0);
                break;
            }
    }
    checkConsistency();

    // and go on with the normal stuff
    dirData->setText(defaultPath);
    nameData->setText(defaultName);
    nameData->setFocus();
    if (typeData->count() == 0) // if no packers are available
        okButton->setEnabled(false);
    setGeometry(krMainWindow->x() + krMainWindow->width() / 2 - width() / 2, krMainWindow->y() + krMainWindow->height() / 2 - height() / 2, width(), height());
    exec();
}

void PackGUI::browse()
{
    QString temp = QFileDialog::getExistingDirectory(nullptr, i18n("Please select a folder"), dirData->text());
    if (!temp.isEmpty()) {
        dirData->setText(temp);
    }
}

void PackGUI::accept()
{
    if (!extraProperties(extraProps))
        return;

    filename = nameData->text();
    destination = dirData->text();
    type = typeData->currentText();
    // write down the packer chosen, to be lastUsedPacker
    KConfigGroup group(krConfig, "Archives");
    group.writeEntry("lastUsedPacker", type);

    group.writeEntry("Command Line Switches", commandLineSwitches->historyItems());
    krConfig->sync();
    PackGUIBase::accept();
}

void PackGUI::reject()
{
    filename.clear();
    destination.clear();
    type.clear();
    // If e.g. the user has deleted a command line switch from the list, that's
    // taken into account even if a file is not packed afterwards
    KConfigGroup group(krConfig, "Archives");
    group.writeEntry("Command Line Switches", commandLineSwitches->historyItems());

    PackGUIBase::reject();
}
