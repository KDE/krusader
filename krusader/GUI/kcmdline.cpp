/***************************************************************************
                               kcmdline.cpp
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


#include "kcmdline.h"
#include "stdlib.h"
#include <unistd.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>
#include <QKeyEvent>
#include <k3process.h>
#include <qicon.h>
#include <q3whatsthis.h>
#include <unistd.h>
#include "../krusader.h"
#include "../kicons.h"
#include "../krslots.h"
#include "../resources.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../krservices.h"
#include "../ActionMan/addplaceholderpopup.h"
#include "kcmdmodebutton.h"
#include <qdir.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qstringlist.h>
#include <qsizepolicy.h>
#include <k3tempfile.h> 

KCMDLine::KCMDLine( QWidget *parent, const char *name ) : QWidget( parent, name ) {
  Q3GridLayout * layout = new Q3GridLayout( this, 1, 4 );
  path = new QLabel( this );
  Q3WhatsThis::add
    ( path, i18n( "Name of directory where command will be processed." ) );
  path->setAlignment( Qt::AlignRight );
  path->setFrameStyle( Q3Frame::Box | Q3Frame::Sunken );
  path->setLineWidth( 1 );
  path->setFont( KGlobalSettings::generalFont() );
  int height = QFontMetrics( KGlobalSettings::generalFont() ).height();
  height =  height + 5*(height > 14) + 6;
  path->setMaximumHeight( height );
  path->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
  layout->addWidget( path, 0, 0 );

  // and editable command line
  completion.setMode( KUrlCompletion::FileCompletion );
  cmdLine = new KrHistoryCombo( this );
  cmdLine->setMaxCount(100);  // remember 100 commands
  cmdLine->setDuplicatesEnabled( false );
  cmdLine->setFont( KGlobalSettings::generalFont() );
  cmdLine->setMaximumHeight( height );
  cmdLine->setCompletionObject( &completion );
  cmdLine->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  // load the history
  KConfigGroup grpSvr = krConfig->group( "Private" );
  QStringList list = krConfig->readListEntry( "cmdline history" );
  cmdLine->setHistoryItems( list );

  connect( cmdLine, SIGNAL( returnPressed(const QString &) ), this, SLOT( slotRun() ) );
  connect( cmdLine, SIGNAL( returnPressed(const QString &) ), cmdLine, SLOT( clearEdit() ) );
  connect( cmdLine, SIGNAL( returnToPanel() ), this, SLOT( slotReturnFocus() ));

  Q3WhatsThis::add
    ( cmdLine, i18n( "<qt><p>Well, it's actually quite simple: You type your command here and Krusader obeys.</p><p><b>Tip</b>: Move within command line history with &lt;Up&gt; and &lt;Down&gt; arrows.</p></qt>" ) );
  layout->addWidget( cmdLine, 0, 1 );

  buttonAddPlaceholder = new QToolButton( this, "ButtonAddPlaceholder" );
  buttonAddPlaceholder->setFixedSize(22,20);
  buttonAddPlaceholder->adjustSize();
  buttonAddPlaceholder->setPixmap( SmallIcon( "add" ) );
  connect( buttonAddPlaceholder, SIGNAL( clicked() ), this, SLOT( addPlaceholder() ) );
  Q3WhatsThis::add( buttonAddPlaceholder, i18n( "Add <b>Placeholders</b> for the selected files in the panel." ) );

  layout->addWidget( buttonAddPlaceholder, 0, 2 );

  // a run in terminal button
  terminal = new KCMDModeButton( this );
  layout->addWidget( terminal, 0, 3 );

  layout->activate();
}

void KCMDLine::addPlaceholder() {
   AddPlaceholderPopup popup( this );
   QString exp = popup.getPlaceholder(
      buttonAddPlaceholder->mapToGlobal(  QPoint( 0, 0) )
   );
   this->addText( exp );
}

void KCMDLine::setCurrent( const QString &p ) {

  QString pathName = p;
  QFontMetrics fm(path->fontMetrics());
  int textWidth = fm.width(pathName);
  int maxWidth = ( cmdLine->width() + path->width() ) * 2 / 5;
  int letters = p.length() / 2;

  while ( letters && textWidth > maxWidth )
  {
    pathName = p.left( letters ) + "..." + p.right( letters );
    letters--;
    textWidth = fm.width(pathName);
  }
  
  path->setText( pathName + ">" );
  
  completion.setDir( p );
  // make sure our command is executed in the right directory
  // This line is important for Krusader overall functions -> do not remove !
  chdir( p.local8Bit() );
}

KCMDLine::~KCMDLine() {
   KConfigGroup grpSvr = krConfig->group( "Private" );
   QStringList list = cmdLine->historyItems();
   //krOut << list[0] << endl;
   krConfig->writeEntry( "cmdline history", list );
   krConfig->sync();
}

void KCMDLine::slotRun() {
  const QString command1(cmdLine->currentText());
  if ( command1.isEmpty() )
    return ;
  QString panelPath = path->text().left( path->text().length() - 1 );

  cmdLine->addToHistory(command1);

  if ( command1.simplified().left( 3 ) == "cd " ) { // cd command effect the active panel
    QString dir = command1.right( command1.length() - command1.find( " " ) ).trimmed();
    if ( dir == "~" )
      dir = QDir::homePath();
    else
      if ( dir.left( 1 ) != "/" && !dir.contains( ":/" ) )
        dir = panelPath + ( panelPath == "/" ? "" : "/" ) + dir;
    SLOTS->refresh( dir );
  } else {
    exec();
    cmdLine->clearEdit();
  }
}


void KCMDLine::slotReturnFocus() {
  Krusader::App->mainView->cmdLineUnFocus();
}

static const KrActionBase::ExecType execModesMenu[] = {
 KrActionBase::Normal,
 KrActionBase::CollectOutputSeparateStderr,
 KrActionBase::CollectOutput,
 KrActionBase::Terminal,
 KrActionBase::RunInTE,
};

QString KCMDLine::command() const {
  return cmdLine->currentText();
}

KrActionBase::ExecType KCMDLine::execType() const {
  KConfigGroup grp( krConfig, "Private" );
  int i = grp.readNumEntry("Command Execution Mode",0);
  return execModesMenu[i];
}

QString KCMDLine::startpath() const {
  return path->text().left( path->text().length() - 1 );
}

QString KCMDLine::user() const {
  return QString();
}

QString KCMDLine::text() const {
  return cmdLine->currentText();
}

bool KCMDLine::acceptURLs() const {
  return false;
}

bool KCMDLine::confirmExecution() const {
  return false;
}

bool KCMDLine::doSubstitution() const {
  return true;
}

void KCMDLine::setText(QString text) {
	cmdLine->setCurrentText( text );
}
 
void KrHistoryCombo::keyPressEvent( QKeyEvent *e ) {
   switch (e->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
         if (e->state() & ControlButton) {
           SLOTS->insertFileName( ( e->state() & ShiftButton ) != 0 );
           break;
         }
         KHistoryComboBox::keyPressEvent(e);
         break;
      case Qt::Key_Down:
         if (e->state()  == ( ControlButton | ShiftButton ) ) {
            MAIN_VIEW->focusTerminalEmulator();
            return;
         } else
            KHistoryComboBox::keyPressEvent(e);
         break;
      case Qt::Key_Up:
         if (e->state() == ControlButton || e->state() == ( ControlButton | ShiftButton ) ) {
            emit returnToPanel();
            return;
         }
      default:
      KHistoryComboBox::keyPressEvent(e);
   }
}

#include "kcmdline.moc"

