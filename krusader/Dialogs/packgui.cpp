/***************************************************************************
                                packgui.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

#include "packgui.h"
#include <kfiledialog.h>
#include "../krusader.h"
#include "../defaults.h"
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtCore/QStringList>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <khistorycombobox.h>

#define PS(x) lst.contains(x)>0

// clear the statics first
QString PackGUI::filename = 0;
QString PackGUI::destination = 0;
QString PackGUI::type = 0;
QMap<QString, QString> PackGUI::extraProps;
bool    PackGUI::queue = false;

PackGUI::PackGUI(QString defaultName, QString defaultPath, int noOfFiles, QString filename) :
        PackGUIBase(0)
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
    if (PS("zip")) typeData->addItem("zip");
    if (PS("rar")) typeData->addItem("rar");
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

    queue = false;

    // and go on with the normal stuff
    dirData->setText(defaultPath);
    nameData->setText(defaultName);
    nameData->setFocus();
    if (typeData->count() == 0) // if no packers are available
        okButton->setEnabled(false);
    setGeometry(krApp->x() + krApp->width() / 2 - width() / 2, krApp->y() + krApp->height() / 2 - height() / 2, width(), height());
    exec();
}

void PackGUI::browse()
{
    QString temp = KFileDialog::getExistingDirectory(dirData->text(), 0, i18n("Please select a directory"));
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
    krConfig->sync();
    PackGUIBase::accept();
}

void PackGUI::reject()
{
    filename = QString();
    destination = QString();
    type = QString();
    PackGUIBase::reject();
}

void PackGUI::slotQueue()
{
    queue = true;
    PackGUIBase::slotQueue();
}

#include "packgui.moc"
