/***************************************************************************
                         kggeneral.cpp  -  description
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

#include "kggeneral.h"
#include "../defaults.h"
#include "../krusader.h"
#include <klocale.h>
#include <qlabel.h>
#include <qhbox.h>
#include <kmessagebox.h>

KgGeneral::KgGeneral( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
if( first )  
    slotFindTools();

  QGridLayout *kgGeneralLayout = new QGridLayout( parent );
  kgGeneralLayout->setSpacing( 6 );
  kgGeneralLayout->setMargin( 11 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "kgGenGeneralGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  KONFIGURATOR_NAME_VALUE_TIP deleteMode[] =
  //            name            value    tooltip
    {{ i18n( "Delete files" ),  "false", i18n( "Files will be permanently deleted." ) },
     { i18n( "Move to trash" ), "true",  i18n( "Files will be moved to Trash when deleted." ) }};

  KonfiguratorRadioButtons *trashRadio = createRadioButtonGroup( "General", "Move To Trash",
      "true", 2, 0, deleteMode, 2, generalGrp, "myRadio", false );
  generalGrid->addMultiCellWidget( trashRadio, 0, 0, 0, 1 );

  KonfiguratorCheckBox *checkBox = createCheckBox( "General", "Mimetype Magic", _MimetypeMagic,
                     i18n( "Use mimetype magic" ), generalGrp, false,
                     i18n( "Mimetype magic allows better distinction of file types, but is slower" ) );
  generalGrid->addMultiCellWidget( checkBox, 1, 1, 0, 1 );

  QFrame *line1 = createLine( generalGrp, "line1" );
  generalGrid->addMultiCellWidget( line1, 2, 2, 0, 1 );

  QLabel *label1 = new QLabel( i18n( "Editor:" ), generalGrp, "EditorLabel" );
  generalGrid->addWidget( label1, 3, 0 );
  KonfiguratorURLRequester *urlReq = createURLRequester( "General", "Editor", "internal editor",
                                      generalGrp, false );
  generalGrid->addWidget( urlReq, 3, 1 );

  QLabel *label2 = new QLabel( i18n( "Hint: use 'internal editor' if you want to use Krusader's fast built-in editor" ), generalGrp, "EditorLabel" );
  generalGrid->addMultiCellWidget( label2, 4, 4, 0, 1 );

  QLabel *label3 = new QLabel( i18n( "Terminal:" ), generalGrp, "TerminalLabel" );
  generalGrid->addWidget( label3, 5, 0 );
  KonfiguratorURLRequester *urlReq2 = createURLRequester( "General", "Terminal", "konsole",
                                      generalGrp, false );
  generalGrid->addWidget( urlReq2, 5, 1 );

  QFrame *line2 = createLine( generalGrp, "line2" );
  generalGrid->addMultiCellWidget( line2, 6, 6, 0, 1 );

  QHBox *hbox = new QHBox( generalGrp, "hbox" );
  new QLabel( i18n( "Temp Directory:" ), hbox, "TempDirectory" );
  KonfiguratorURLRequester *urlReq3 = createURLRequester( "General", "Temp Directory", "/tmp/krusader.tmp",
                                      hbox, false );
  urlReq3->setMode( KFile::Directory );
  connect( urlReq3->extension(), SIGNAL( applyManually(QObject *,QString, QString) ),
           this, SLOT( applyTempDir(QObject *,QString, QString) ) );
  generalGrid->addMultiCellWidget( hbox, 7, 7, 0, 1 );

  QLabel *label4 = new QLabel( i18n( "NOTE: You must have full permissions for the temporary directory!" ),
                               generalGrp, "NoteLabel"  );
  generalGrid->addMultiCellWidget( label4, 8, 8, 0, 1 );

  kgGeneralLayout->addWidget( generalGrp, 0 ,0 );
}

void KgGeneral::applyTempDir(QObject *obj,QString cls, QString name)
{
  KonfiguratorURLRequester *urlReq = (KonfiguratorURLRequester *)obj;
  QString value = QDir(urlReq->url()).path();

  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value );
}

void KgGeneral::slotFindTools()
{
  #define PS(x) lst.contains(x)>0
  QString info;
  QStringList lst=Krusader::supportedTools(); // get list of availble tools

  info+=i18n("Searching for tools...\nSearch results:\n\n");
  if (PS("DIFF")) info+=i18n("diff: found ")+lst[lst.findIndex("DIFF") + 1]+i18n(", compare by content available.\n");
  else info+=i18n("diff: no diff frontends found. Compare by content disabled.\nhint: Krusader supports Kompare, Kdiff3 and Xxdiff\n\n");

  if (PS("MAIL")) info+=i18n("mail: found ")+lst[lst.findIndex("MAIL") + 1]+i18n(", sending files by email enabled.\n");
  else info+=i18n("mail: no compatible mail-programs found. Sending files by email is disabled.\nhint: Krusader supports Kmail\n\n");

  if (PS("RENAME")) info+=i18n("rename: found ")+lst[lst.findIndex("RENAME") + 1]+i18n(", multiple rename enabled.\n");
  else info+=i18n("rename: no compatible rename-programs found. Multiple rename is disabled.\nhint: Krusader supports Krename\n\n");


  info+=i18n("\nIf you install new tools, please install them");
  info+=i18n("\nto your path, e.g. /usr/bin, /usr/local/bin, etc.");
  info+=i18n("\nand-or (re)configure the Tools path in");
  info+=i18n("\nKonfigurator->Dependencies if needed.");
  KMessageBox::information(0,info,i18n("Results"));
}

#include "kggeneral.moc"
