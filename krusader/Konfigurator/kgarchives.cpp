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
#include "../defaults.h"
#include "../VFS/krarchandler.h"
#include "../krusader.h"
#include <klocale.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qstringlist.h>
#include <kmessagebox.h>

KgArchives::KgArchives( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgArchivesLayout = new QGridLayout( parent );
  kgArchivesLayout->setSpacing( 6 );
  kgArchivesLayout->setMargin( 11 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "generalGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  addLabel( generalGrid, 0, 0, i18n( "Krusader transparently handles the following types of archives:" ),
            generalGrp, "KgLabel1" );

  KONFIGURATOR_CHECKBOX_PARAM packers[] =
  //   cfg_class  cfg_name   default   text             restart tooltip
    {{"Archives","Do Tar",   _DoTar,   i18n( "Tar" ),   false,  ""},
     {"Archives","Do GZip",  _DoGZip,  i18n( "GZip" ),  false,  ""},
     {"Archives","Do BZip2", _DoBZip2, i18n( "BZip2" ), false,  ""},
     {"Archives","Do UnZip", _DoUnZip, i18n( "Zip" ),   false,  ""},
     {"Archives","Do UnRar", _DoUnRar, i18n( "Rar" ),   false,  ""},
     {"Archives","Do Unarj", _DoArj,   i18n( "Arj" ),   false,  ""},
     {"Archives","Do RPM",   _DoRPM,   i18n( "Rpm" ),   false,  ""},
     {"Archives","Do UnAce", _DoUnAce, i18n( "Ace" ),   false,  ""},
     {"Archives","Do Lha",   _DoLha,   i18n( "Lha" ),   false,  ""}};

  cbs = createCheckBoxGroup( 3, 0, packers, 9, generalGrp );
  generalGrid->addWidget( cbs, 1, 0 );

  addLabel( generalGrid, 2, 0, i18n( "The archives that are \"greyed-out\" were unavailable on your\nsystem last time Krusader checked. If you wish Krusader to\nsearch again, click the 'Auto Configure' button." ),
            generalGrp, "KgLabel2" );

  QHBox *hbox = new QHBox( generalGrp );
  createSpacer( hbox, "spacer1" );
  QPushButton *btnAutoConfigure = new QPushButton( i18n( "Auto Configure" ), hbox, "kgAutoConfigure" );
  createSpacer( hbox, "spacer2" );
  generalGrid->addWidget( hbox, 3, 0 );
  connect( btnAutoConfigure, SIGNAL( clicked() ), this, SLOT( slotAutoConfigure() ) );

  kgArchivesLayout->addWidget( generalGrp, 0 ,0 );

  //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

  QGroupBox *fineTuneGrp = createFrame( i18n( "Fine-Tuning" ), parent, "fineTuneGrp" );
  QGridLayout *fineTuneGrid = createGridLayout( fineTuneGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM finetuners[] =
  //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {//{"Archives","Allow Move Into Archive", _MoveIntoArchive, i18n( "Allow moving into archives" ),         false,  i18n( "This action can be tricky, since system failure during the process\nmight result in misplaced files. If this happens,\nthe files are stored in a temp directory inside /tmp." )},
     {"Archives","Test Archives",           _TestArchives,    i18n( "Test archive when finished packing" ), false,  i18n( "If checked, Krusader will test the archive's integrity after packing it." )},
     {"Archives","Test Before Unpack",      _TestBeforeUnpack,i18n( "Test archive before unpacking" ), false,  i18n( "Some corrupted archives might cause a crash, therefore testing is recommended." )}};

  KonfiguratorCheckBoxGroup *finetunes = createCheckBoxGroup( 1, 0, finetuners, 2, fineTuneGrp );

  disableNonExistingPackers();
  fineTuneGrid->addWidget( finetunes, 1, 0 );

  kgArchivesLayout->addWidget( fineTuneGrp, 1 ,0 );
  
  if( first )
    slotAutoConfigure();

}

void KgArchives::slotAutoConfigure()
{
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
  if (PS("lha")) info+=i18n("lha: found, packing and unpacking enabled.\n");
  else info+=i18n("lha: NOT found, packing and unpacking DISABLED.\n==> lha can be obtained at www.gnu.org\n");
  if (PS("rpm") && PS("cpio")) info+=i18n("rpm: found, unpacking enabled.\n");
  else if (PS("rpm") && !PS("cpio")) info+=i18n("rpm found but cpio NOT found: unpacking DISABLED.\n==>cpio can be obtained at www.gnu.org\n");
  else info+=i18n("rpm: NOT found, unpacking is DISABLED.\n==> rpm can be obtained at www.gnu.org\n");
  if (PS("unrar")) info+=i18n("unrar: found, unpacking is enabled.\n");
  else {
    if( PS("rar")) info+=i18n("unrar: NOT found.\n");
    else           info+=i18n("unrar: NOT found, unpacking is DISABLED.\n==> unrar can be obtained at www.rarsoft.com\n");
  }
  if (PS("rar")) info+=i18n("rar: found, packing and unpacking is enabled.\n");
  else info+=i18n("rar: NOT found, packing is DISABLED.\n==> rar can be obtained at www.rarsoft.com\n");
  if (PS("unarj")) info+=i18n("unarj: found, unpacking is enabled.\n");
  else {
    if( PS("arj")) info+=i18n("unarj: NOT found.\n");
    else           info+=i18n("unarj: NOT found, unpacking is DISABLED.\n==> unarj can be obtained at www.arjsoft.com\n");
  }
  if (PS("arj")) info+=i18n("arj: found, packing and unpacking is enabled.\n");
  else info+=i18n("arj: NOT found, packing is DISABLED.\n==> arj can be obtained at www.arjsoft.com\n");
  if (PS("unace")) info+=i18n("unace: found, unpacking is enabled.\n");
  else info+=i18n("unace: NOT found, unpacking is DISABLED.\n==> unace can be obtained at www.winace.com\n");

  info+=i18n("\nIf you install new packers, please install them");
  info+=i18n("\nto your path. (ie: /usr/bin, /usr/local/bin etc.)");
  info+=i18n("\nThanks for flying Krusader :-)");
  KMessageBox::information(0,info,i18n("Results"));
  disableNonExistingPackers();
}

void KgArchives::disableNonExistingPackers()
{
  #define PS(x) lst.contains(x)>0

  QStringList lst=KRarcHandler::supportedPackers(); // get list of availble packers
  cbs->find( "Do Tar" )->setEnabled(PS("tar"));
  cbs->find( "Do GZip" )->setEnabled(PS("gzip"));
  cbs->find( "Do BZip2" )->setEnabled(PS("bzip2"));
  cbs->find( "Do UnZip" )->setEnabled(PS("unzip"));
  cbs->find( "Do Lha" )->setEnabled(PS("lha"));
  cbs->find( "Do RPM" )->setEnabled(PS("rpm") || PS("cpio"));
  cbs->find( "Do UnRar" )->setEnabled(PS("unrar") || PS("rar") );
  cbs->find( "Do UnAce" )->setEnabled(PS("unace"));
  cbs->find( "Do Unarj" )->setEnabled(PS("unarj") || PS("arj") );

  krConfig->setGroup( "Archives" );
  krConfig->writeEntry( "Supported Packers", lst );
}

bool KgArchives::apply()
{
  krConfig->setGroup( "Archives" );
  krConfig->writeEntry("Supported Packers",KRarcHandler::supportedPackers());
  return KonfiguratorPage::apply();
}

void KgArchives::setDefaults()
{
  krConfig->setGroup( "Archives" );
  krConfig->writeEntry("Supported Packers",KRarcHandler::supportedPackers());
  return KonfiguratorPage::setDefaults();
}

#include "kgarchives.moc"

