/***************************************************************************
                                 kgarchivesimpl.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P" 88  `8D 88    88 88"  YP d8" `8b 88  `8D 88"     88  `8D
     88,8P   88oobY" 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY"
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P" `8888Y" YP   YP Y8888D" Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include <kgarchivesimpl.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/krarchandler.h"

kgArchivesImpl::kgArchivesImpl(bool f, QWidget *parent, const char *name ) :
  kgArchives(parent,name), firstTime(f) {
  if (firstTime) slotAutoConfigure();
  else slotSetup();
}
kgArchivesImpl::~kgArchivesImpl(){
}

void kgArchivesImpl::slotSetup() {
  #define PS(x) lst.contains(x)>0

  QStringList lst=KRarcHandler::supportedPackers(); // get list of availble packers
  krConfig->setGroup("Archives");
  // first, set the check-marks according to the config
  kgArj->setChecked(krConfig->readBoolEntry("Do Unarj",_DoUnarj) && PS("unarj"));
  kgBZip2->setChecked(krConfig->readBoolEntry("Do BZip2",_DoBZip2) && PS("bzip2"));
  kgGZip->setChecked(krConfig->readBoolEntry("Do GZip",_DoGZip) && PS("gzip"));
  kgRar->setChecked(krConfig->readBoolEntry("Do UnRar",_DoUnRar) && PS("unrar"));
  kgAce->setChecked(krConfig->readBoolEntry("Do UnAce",_DoUnAce) && PS("unace"));
  kgRpm->setChecked(krConfig->readBoolEntry("Do RPM",_DoRPM) && PS("rpm") && PS("cpio"));
  kgTar->setChecked(krConfig->readBoolEntry("Do Tar",_DoTar) && PS("tar"));
  kgZip->setChecked(krConfig->readBoolEntry("Do UnZip",_DoUnZip) && PS("unzip"));
  kgMoveIntoArchives->setChecked(krConfig->readBoolEntry("Allow Move Into Archive",_MoveIntoArchive));
  kgTestArchives->setChecked(krConfig->readBoolEntry("Test Archives",_TestArchives));

  // now, disable the unsupported packers
  kgTar->setEnabled(PS("tar"));
  kgGZip->setEnabled(PS("gzip"));
  kgBZip2->setEnabled(PS("bzip2"));
  kgZip->setEnabled(PS("unzip"));
  kgRpm->setEnabled(PS("rpm") || PS("cpio"));
  kgRar->setEnabled(PS("unrar"));
  kgAce->setEnabled(PS("unace"));
  kgArj->setEnabled(PS("unarj"));
}

void kgArchivesImpl::slotDefaultSettings() {
  krConfig->setGroup("Archives");
  krConfig->writeEntry("Do Unarj",_DoUnarj);
  krConfig->writeEntry("Do BZip2",_DoBZip2);
  krConfig->writeEntry("Do GZip",_DoGZip);
  krConfig->writeEntry("Do UnRar",_DoUnRar);
  krConfig->writeEntry("Do UnAce",_DoUnAce);
  krConfig->writeEntry("Do RPM",_DoRPM);
  krConfig->writeEntry("Do Tar",_DoTar);
  krConfig->writeEntry("Do UnZip",_DoUnZip);
  krConfig->writeEntry("Test Archives",_TestArchives);
  krConfig->writeEntry("Allow Move Into Archive",_MoveIntoArchive);
  krConfig->writeEntry("Supported Packers",KRarcHandler::supportedPackers());

  krConfig->sync();
}

void kgArchivesImpl::slotApplyChanges() {
  krConfig->setGroup("Archives");
  krConfig->writeEntry("Do Unarj",kgArj->isChecked());
  krConfig->writeEntry("Do BZip2",kgBZip2->isChecked());
  krConfig->writeEntry("Do GZip",kgGZip->isChecked());
  krConfig->writeEntry("Do UnRar",kgRar->isChecked());
  krConfig->writeEntry("Do UnAce",kgAce->isChecked());
  krConfig->writeEntry("Do RPM",kgRpm->isChecked());
  krConfig->writeEntry("Do Tar",kgTar->isChecked());
  krConfig->writeEntry("Do UnZip",kgZip->isChecked());
  krConfig->writeEntry("Test Archives",kgTestArchives->isChecked());
  krConfig->writeEntry("Allow Move Into Archive",kgMoveIntoArchives->isChecked());
  krConfig->writeEntry("Supported Packers",KRarcHandler::supportedPackers());

  krConfig->sync();
}

void kgArchivesImpl::slotAutoConfigure() {
  #define PS(x) lst.contains(x)>0
  QString info;
  QStringList lst=KRarcHandler::supportedPackers(); // get list of availble packers

  info+=i18n("Search results:\n\n");
  if (PS("tar")) info+=i18n("tar: found, packing and unpacking enabled.\n");
  else info+=i18n("tar: NOT found, packing and unpacking DISABLED.\n==> tar can be obtained at www.gnu.org\n");
  if (PS("gzip")) info+=i18n("gzip: found, packing and unpacking enabled.\n");
  else info+=i18n("gzip: NOT found, packing and unpacking DISABLED.\n==> gzip can be obtained at www.gnu.org\n");
  if (PS("bzip2")) info+=i18n("bzip2: found, packing and unpacking enabled.\n");
  else info+=i18n("bzip2: NOT found, packing and unpacking DISABLED.\n==> bzip2 can be obtained at www.gnu.org\n");
  if (PS("unzip")) info+=i18n("unzip: found, unpacking enabled.\n");
  else info+=i18n("unzip: NOT found, unpacking DISABLED.\n==> unzip can be obtained at www.info-zip.org\n");
  if (PS("zip")) info+=i18n("zip: found, packing enabled.\n");
  else info+=i18n("zip: NOT found, packing DISABLED.\n==> zip can be obtained at www.info-zip.org\n");
  if (PS("rpm") && PS("cpio")) info+=i18n("rpm: found, unpacking enabled.\n");
  else if (PS("rpm") && !PS("cpio")) info+=i18n("rpm found but cpio NOT found: unpacking DISABLED.\n==>cpio can be obtained at www.gnu.org\n");
  else info+=i18n("rpm: NOT found, unpacking is DISABLED.\n==> rpm can be obtained at www.gnu.org\n");
  if (PS("unrar")) info+=i18n("unrar: found, unpacking is enabled.\n");
  else info+=i18n("unrar: NOT found, unpacking is DISABLED.\n==> unrar can be obtained at www.rarsoft.com\n");
  if (PS("rar")) info+=i18n("rar: found, packing is enabled.\n");
  else info+=i18n("rar: NOT found, packing is DISABLED.\n==> rar can be obtained at www.rarsoft.com\n");
  if (PS("unarj")) info+=i18n("unarj: found, unpacking is enabled.\n");
  else info+=i18n("unarj: NOT found, unpacking is DISABLED.\n==> unarj can be obtained at www.arjsoft.com\n");
  if (PS("unace")) info+=i18n("unace: found, unpacking is enabled.\n");
  else info+=i18n("unace: NOT found, unpacking is DISABLED.\n==> unace can be obtained at www.winace.com\n");

  info+=i18n("\nIf you install new packers, please install them");
  info+=i18n("\nto your path. (ie: /usr/bin, /usr/local/bin etc.)");
  info+=i18n("\nThanks for flying Krusader :-)");
  KMessageBox::information(0,info,i18n("Results"));
  slotSetup();
}

#include "kgarchivesimpl.moc"
