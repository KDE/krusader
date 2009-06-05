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

#include <QtGui/QLabel>
#include <QtGui/QFontMetrics>
#include <QGridLayout>
#include <QFrame>
#include <QPixmap>

#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include "krresulttabledialog.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../kicons.h"

KgGeneral::KgGeneral( bool first, QWidget* parent ) :
      KonfiguratorPage( first, parent )
{
if( first )  
    slotFindTools();

  QWidget *innerWidget = new QFrame( this );
  setWidget( innerWidget );
  setWidgetResizable( true );
  QGridLayout *kgGeneralLayout = new QGridLayout( innerWidget );
  kgGeneralLayout->setSpacing( 6 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), innerWidget );
  QGridLayout *generalGrid = createGridLayout( generalGrp );

  KONFIGURATOR_NAME_VALUE_TIP deleteMode[] =
  //            name            value    tooltip
    {{ i18n( "Delete files" ),  "false", i18n( "Files will be permanently deleted." ) },
     { i18n( "Move to trash" ), "true",  i18n( "Files will be moved to trash when deleted." ) }};

  KonfiguratorRadioButtons *trashRadio = createRadioButtonGroup( "General", "Move To Trash",
      _MoveToTrash ? "true" : "false", 2, 0, deleteMode, 2, generalGrp, false );
  generalGrid->addWidget( trashRadio, 0, 0, 1, 2 );

  KonfiguratorCheckBox *checkBox = createCheckBox( "General", "Mimetype Magic", _MimetypeMagic,
                     i18n( "Use mimetype magic" ), generalGrp, false,
                     i18n( "Mimetype magic allows better distinction of file types, but is slower." ) );
  generalGrid->addWidget( checkBox, 1, 0, 1, 2 );

  QFrame *line1 = createLine( generalGrp );
  generalGrid->addWidget( line1, 2, 0, 1, 2 );

  // editor
  QLabel *label1 = new QLabel( i18n( "Editor:" ), generalGrp );
  generalGrid->addWidget( label1, 3, 0 );
  KonfiguratorURLRequester *urlReq = createURLRequester( "General", "Editor", "internal editor",
                                      generalGrp, false );
  generalGrid->addWidget( urlReq, 3, 1 );

  QLabel *label2 = new QLabel( i18n( "Hint: use 'internal editor' if you want to use Krusader's fast built-in editor" ), generalGrp );
  generalGrid->addWidget( label2, 4, 0, 1, 2 );

  QFrame *line2 = createLine( generalGrp );
  generalGrid->addWidget( line2, 5, 0, 1, 2 );

  // viewer

  QWidget * hboxWidget2 = new QWidget( generalGrp );
  QHBoxLayout * hbox2 = new QHBoxLayout( hboxWidget2 );

  QWidget * vboxWidget = new QWidget( hboxWidget2 );
  QVBoxLayout * vbox = new QVBoxLayout( vboxWidget );

  vbox->addWidget( new QLabel( i18n("Default viewer mode:"), vboxWidget) );
  
  KONFIGURATOR_NAME_VALUE_TIP viewMode[] =
  //            name            value    tooltip
    {{ i18n( "Generic mode" ),  "generic", i18n( "Use the system's default viewer" ) },
     { i18n( "Text mode" ), "text",  i18n( "View the file in text-only mode" ) }, 
     { i18n( "Hex mode" ), "hex",  i18n( "View the file in hex-mode (better for binary files)" ) },
     { i18n( "Lister mode" ), "lister",  i18n( "View the file with lister (for huge text files)" ) } };

  vbox->addWidget( createRadioButtonGroup( "General", "Default Viewer Mode",
      "generic", 0, 4, viewMode, 4, vboxWidget, false ) );

  vbox->addWidget( createCheckBox( "General", "View In Separate Window", _ViewInSeparateWindow,
                     i18n( "Internal editor and viewer opens each file in a separate window" ), vboxWidget, false,
                     i18n( "If checked, each file will open in a separate window, otherwise, the viewer will work in a single, tabbed mode" ) ) );

  hbox2->addWidget( vboxWidget );
  generalGrid->addWidget(hboxWidget2, 6, 0, 3, 2);

  // atomic extensions
  QFrame * frame21 = createLine( hboxWidget2, true );
  frame21->setMinimumWidth( 15 );
  hbox2->addWidget( frame21 );

  QWidget * vboxWidget2 = new QWidget( hboxWidget2 );
  QVBoxLayout * vbox2 = new QVBoxLayout( vboxWidget2 );

  hbox2->addWidget( vboxWidget2 );

  QWidget * hboxWidget3 = new QWidget( vboxWidget2 );
  vbox2->addWidget( hboxWidget3 );

  QHBoxLayout * hbox3 = new QHBoxLayout( hboxWidget3 );

  QLabel * atomLabel = new QLabel( i18n("Atomic extensions:"), hboxWidget3);
  hbox3->addWidget( atomLabel );

  int size = QFontMetrics( atomLabel->font() ).height();

  QToolButton *addButton = new QToolButton( hboxWidget3 );
  hbox3->addWidget( addButton );

  QPixmap icon = krLoader->loadIcon("list-add",KIconLoader::Desktop, size );
  addButton->setFixedSize( icon.width() + 4, icon.height() + 4 );
  addButton->setIcon( QIcon( icon ) );
  connect( addButton, SIGNAL( clicked() ), this, SLOT( slotAddExtension() ) );

  QToolButton *removeButton = new QToolButton( hboxWidget3 );
  hbox3->addWidget( removeButton );

  icon = krLoader->loadIcon("list-remove",KIconLoader::Desktop, size );
  removeButton->setFixedSize( icon.width() + 4, icon.height() + 4 );
  removeButton->setIcon( QIcon( icon ) );
  connect( removeButton, SIGNAL( clicked() ), this, SLOT( slotRemoveExtension() ) );

  QStringList defaultAtomicExtensions;
  defaultAtomicExtensions += ".tar.gz";
  defaultAtomicExtensions += ".tar.bz2";
  defaultAtomicExtensions += ".tar.lzma";
  defaultAtomicExtensions += ".moc.cpp";

  listBox = createListBox( "Look&Feel", "Atomic Extensions", 
      defaultAtomicExtensions, vboxWidget2, true, false );
  vbox2->addWidget( listBox );

  QFrame *line3 = createLine( generalGrp );
  generalGrid->addWidget( line3, 9, 0, 1, 2 );

	// terminal
  QLabel *label3 = new QLabel( i18n( "Terminal:" ), generalGrp );
  generalGrid->addWidget( label3, 10, 0 );
  KonfiguratorURLRequester *urlReq2 = createURLRequester( "General", "Terminal", _Terminal,
                                      generalGrp, false );
  generalGrid->addWidget( urlReq2, 10, 1 );

  KonfiguratorCheckBox *checkBox1 = createCheckBox( "General", "Send CDs", _SendCDs,
                     i18n( "Terminal Emulator sends Chdir on panel change" ), generalGrp, false,
                     i18n( "When checked, whenever the panel is changed (for example, by pressing TAB), krusader changes the current directory in the terminal emulator." ) );
  generalGrid->addWidget( checkBox1, 11, 0, 1, 2 );

  QFrame *line31 = createLine( generalGrp );
  generalGrid->addWidget( line31, 12, 0, 1, 2 );

	// temp dir
  QWidget *hboxWidget = new QWidget( generalGrp );
  QHBoxLayout * hbox = new QHBoxLayout( hboxWidget );

  hbox->addWidget( new QLabel( i18n( "Temp Directory:" ), hboxWidget ) );
  KonfiguratorURLRequester *urlReq3 = createURLRequester( "General", "Temp Directory", "/tmp/krusader.tmp",
                                      hboxWidget, false );
  urlReq3->setMode( KFile::Directory );
  connect( urlReq3->extension(), SIGNAL( applyManually(QObject *,QString, QString) ),
           this, SLOT( applyTempDir(QObject *,QString, QString) ) );
  hbox->addWidget( urlReq3 );
  generalGrid->addWidget( hboxWidget, 13, 0, 1, 2 );

  QLabel *label4 = new QLabel( i18n( "Note: you must have full permissions for the temporary directory!" ),
                               generalGrp  );
  generalGrid->addWidget( label4, 14, 0, 1, 2 );


  kgGeneralLayout->addWidget( generalGrp, 0 ,0 );
}

void KgGeneral::applyTempDir(QObject *obj,QString cls, QString name)
{
  KonfiguratorURLRequester *urlReq = (KonfiguratorURLRequester *)obj;
  QString value = QDir(urlReq->url().prettyUrl()).path();

  KConfigGroup( krConfig, cls ).writeEntry( name, value );
}

void KgGeneral::slotFindTools()
{
  KrResultTableDialog* dia = new KrResultTableDialog(this, KrResultTableDialog::Tool, i18n("Search results"), i18n("Searching for tools..."),
    "package_settings", i18n("Make sure to install new tools in your <code>$PATH</code> (e.g. /usr/bin)"));
  dia->exec();
}

void KgGeneral::slotAddExtension()
{
  bool ok;
  QString atomExt =
    KInputDialog::getText( i18n( "Add new atomic extension" ), i18n( "Extension: " ), QString(), &ok );

  if( ok )
  {
    if( !atomExt.startsWith( "." ) || atomExt.indexOf( '.', 1 ) == -1 )
      KMessageBox::error(krApp, i18n("Atomic extensions must start with '.'\n and must contain at least one more '.' character"), i18n("Error"));
    else
      listBox->addItem( atomExt );
  }
}

void KgGeneral::slotRemoveExtension()
{
  QList<QListWidgetItem *> list = listBox->selectedItems();

  for( int i=0; i != list.count(); i++ )
    listBox->removeItem( list[ i ]->text() );
}

#include "kggeneral.moc"
