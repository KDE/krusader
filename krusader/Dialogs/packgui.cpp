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
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <khistorycombobox.h>

#define PS(x) lst.contains(x)>0

// clear the statics first
QString PackGUI::filename=0;
QString PackGUI::destination=0;
QString PackGUI::type=0;
QMap<QString, QString> PackGUI::extraProps;

PackGUI::PackGUI(QString defaultName, QString defaultPath, int noOfFiles, QString filename) :
    PackGUIBase(0,0,true) {
  // first, fill the WhatToPack textfield with information
  if(noOfFiles == 1)
    TextLabel1->setText( i18n("Pack %1").arg(filename) );
  else
    TextLabel1->setText( i18np("Pack %1 file", "Pack %1 files", noOfFiles) );

  // now, according to the Konfigurator, fill the combobox with the information
  // about what kind of packing we can do
  krConfig->setGroup("Archives");
  QStringList lst=krConfig->readListEntry("Supported Packers");
  // now, clear the type combo and begin...
  typeData->clear();
  if (PS("tar")) typeData->insertItem("tar");
  if (PS("tar") && PS("gzip")) typeData->insertItem("tar.gz");
  if (PS("tar") && PS("bzip2")) typeData->insertItem("tar.bz2");
  if (PS("zip")) typeData->insertItem("zip");
  if (PS("rar")) typeData->insertItem("rar");
  if (PS("lha")) typeData->insertItem("lha");
  if (PS("arj")) typeData->insertItem("arj");
  if (PS("7z")) typeData->insertItem("7z");
  // set the last used packer as the top one
  QString tmp=krConfig->readEntry("lastUsedPacker",QString());
  if (tmp!=QString()) {
    for (unsigned int i=0; i< typeData->count(); ++i)
      if (typeData->itemText( i ) == tmp) {
        typeData->removeItem(i);
        typeData->insertItem(tmp,0);
        break;
      }
  }
  checkConsistency();

  // and go on with the normal stuff
  dirData->setText(defaultPath);
  nameData->setText(defaultName);
  nameData->setFocus();
  if (typeData->count()==0) // if no packers are availble
    okButton->setEnabled(false);
  setGeometry(krApp->x()+krApp->width()/2-width()/2,krApp->y()+krApp->height()/2-height()/2,width(),height());
	exec();
}

void PackGUI::browse() {
  QString temp=KFileDialog::getExistingDirectory(dirData->text(),0,i18n("Please select a directory"));
  if (temp != QString())
			dirData->setText(temp);
}

void PackGUI::accept() {
  if( !extraProperties( extraProps ) )
    return;

  filename=nameData->text();
  destination=dirData->text();
  type=typeData->currentText();
  // write down the packer chosen, to be lastUsedPacker
  krConfig->setGroup("Archives");
  krConfig->writeEntry("lastUsedPacker",type);
  krConfig->sync();
  PackGUIBase::accept();
}

void PackGUI::reject() {
  filename=QString();
  destination=QString();
  type=QString();
  PackGUIBase::reject();
}

#include "packgui.moc"
