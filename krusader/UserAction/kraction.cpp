//
// C++ Implementation: kraction
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <q3textedit.h>
#include <qboxlayout.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>
#include <kaction.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kvbox.h>
#include <kpushbutton.h>
#include "kraction.h"
#include "expander.h"
#include "useraction.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../defaults.h"

//for the availabilitycheck:
#include <kmimetype.h>
#include <qregexp.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  KrActionProcDlg  /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include <qlayout.h>
KrActionProcDlg::KrActionProcDlg( QString caption, bool enableStderr, QWidget *parent ) :
        KDialog( parent ), _stdout(0), _stderr(0), _currentTextEdit(0) {
   setCaption( caption );
   setButtons( KDialog::User1 | KDialog::Ok | KDialog::Cancel );
   setDefaultButton( KDialog::Cancel );
   setWindowModality( Qt::NonModal );

   setButtonGuiItem( KDialog::Ok, KGuiItem( i18n( "Close" ) ) );
   enableButtonOk( false ); // disable the close button, until the process finishes

   setButtonGuiItem( KDialog::Cancel, KGuiItem(i18n("Kill"), i18n( "Kill the running process" )) );

   setButtonText(KDialog::User1, i18n("Save as") );

   KVBox *page = new KVBox( this );
   setMainWidget( page );
   // do we need to separate stderr and stdout?
   if ( enableStderr ) {
      QSplitter *splitt = new QSplitter( Qt::Vertical, page );
      // create stdout
      QWidget *stdoutWidget = new QWidget( splitt );
      QVBoxLayout *stdoutBox = new QVBoxLayout( stdoutWidget );

      stdoutBox->setSpacing( 6 );
      stdoutBox->addWidget( new QLabel( i18n( "Standard Output (stdout)" ), stdoutWidget ) );
      _stdout = new Q3TextEdit( stdoutWidget );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
      stdoutBox->addWidget( _stdout );
      // create stderr
      QWidget *stderrWidget = new QWidget( splitt );
      QVBoxLayout *stderrBox = new QVBoxLayout( stderrWidget );

      stderrBox->setSpacing( 6 );
      stderrBox->addWidget( new QLabel( i18n( "Standard Error (stderr)" ), stderrWidget ) );
      _stderr = new Q3TextEdit( stderrWidget );
      _stderr->setReadOnly( true );
      _stderr->setMinimumWidth( fontMetrics().maxWidth() * 40 );
      stderrBox->addWidget( _stderr );
   } else {
      // create stdout
      new QLabel( i18n( "Output" ), page );
      _stdout = new Q3TextEdit( page );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
   }

   _currentTextEdit = _stdout;
   connect( _stdout, SIGNAL( clicked(int, int) ), SLOT( currentTextEditChanged() ) );
   if (_stderr)
      connect( _stderr, SIGNAL( clicked(int, int) ), SLOT( currentTextEditChanged() ) );

   KConfigGroup group( krConfig, "UserActions" );
   normalFont = group.readEntry( "Normal Font", *_UserActions_NormalFont );
   fixedFont = group.readEntry( "Fixed Font", *_UserActions_FixedFont );
   bool startupState = group.readEntry( "Use Fixed Font", _UserActions_UseFixedFont );
   toggleFixedFont( startupState );

   // HACK This fetches the layout of the buttonbox from KDialog, although it is not accessable with KDialog's API
   // None the less it's quite save to use since this implementation hasn't changed since KDE-3.3 (I haven't looked at earlier
   // versions since we don't support them) and now all work is done in KDE-4.
   QWidget* buttonBox = static_cast<QWidget*>( button(KDialog::Ok)->parent() );
   QBoxLayout* buttonBoxLayout = static_cast<QBoxLayout*>( buttonBox->layout() );
   QCheckBox* useFixedFont = new QCheckBox( i18n("Use font with fixed width"), buttonBox );
   buttonBoxLayout->insertWidget( 0, useFixedFont );
   useFixedFont->setChecked( startupState );
   connect( useFixedFont, SIGNAL( toggled(bool) ), SLOT( toggleFixedFont(bool) ) );

   connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotUser1() ) );
}

void KrActionProcDlg::addStderr( K3Process *, char *buffer, int buflen ) {
   if (_stderr)
      _stderr->append( QString::fromLatin1( buffer, buflen ) );
   else {
      _stdout->setItalic(true);
      _stdout->append( QString::fromLatin1( buffer, buflen ) );
      _stdout->setItalic(false);
   }
}

void KrActionProcDlg::addStdout( K3Process *, char *buffer, int buflen ) {
   _stdout->append( QString::fromLatin1( buffer, buflen ) );
}

void KrActionProcDlg::toggleFixedFont( bool state ) {
   if ( state ) {
      _stdout->setFont( fixedFont );
      if ( _stderr )
         _stderr->setFont( fixedFont );
   }
   else {
      _stdout->setFont( normalFont );
      if ( _stderr )
         _stderr->setFont( normalFont );
   }
}

void KrActionProcDlg::slotUser1() {
   QString filename = KFileDialog::getSaveFileName(QString(), i18n("*.txt|Text files\n*|all files"), this);
   if ( filename.isEmpty() )
      return;
   QFile file( filename );
   int answer = KMessageBox::Yes;
   if ( file.exists() )
      answer = KMessageBox::warningYesNoCancel( this,	//parent
      		i18n("This file already exists.\nDo you want to overwrite it or append the output?"),	//text
      		i18n("Overwrite or append?"),	//caption
      		KGuiItem( i18n("Overwrite") ),	//label for Yes-Button
      		KGuiItem( i18n("Append") )	//label for No-Button
      	);
   if ( answer == KMessageBox::Cancel )
      return;
   bool open;
   if ( answer == KMessageBox::No ) // this means to append
      open = file.open( QIODevice::WriteOnly | QIODevice::Append );
   else
      open = file.open( QIODevice::WriteOnly );

   if ( ! open ) {
      KMessageBox::error( this,
      		i18n("Can't open %1 for writing!\nNothing exported.", filename),
      		i18n("Export failed!")
      	);
      return;
   }

   Q3TextStream stream( &file );
   stream << _currentTextEdit->text();
   file.close();
}

void KrActionProcDlg::currentTextEditChanged() {
   if ( _stderr && _stderr->hasFocus() )
      _currentTextEdit = _stderr;
   else
      _currentTextEdit = _stdout;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  KrActionProc  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrActionProc::KrActionProc( KrActionBase* action ) : QObject(), _action( action ), _proc( new K3Process(this) ), _output( 0 ) {
   _proc->setUseShell( true );

   connect( _proc, SIGNAL( processExited( K3Process* ) ),
            this, SLOT( processExited( K3Process* ) ) ) ;
}

KrActionProc::~KrActionProc() {
   delete _proc;
}

void KrActionProc::start( QString cmdLine ) {
   QStringList list;
   list << cmdLine;
   start( list );
}

void KrActionProc::start( QStringList cmdLineList ) {
   _proc->clearArguments();
   QString cmd;

   if ( ! _action->startpath().isEmpty() )
      _proc->setWorkingDirectory( _action->startpath() );

   if ( _action->execType() == KrAction::Terminal && cmdLineList.count() > 1)
      KMessageBox::sorry( 0, i18n("Support for more than one command doesn't work in a terminal. Only the first is executed in the terminal.") );

   if ( _action->execType() == KrAction::RunInTE
         && ( MAIN_VIEW->konsole_part == NULL || MAIN_VIEW->konsole_part->widget() == NULL ) ) {
      KMessageBox::sorry( 0, i18n("Embedded terminal emulator does not work, using output collection instead.") );
   }

   if( _action->execType() == KrAction::Normal || _action->execType() == KrAction::Terminal
       || ( _action->execType() == KrAction::RunInTE && MAIN_VIEW->konsole_part && MAIN_VIEW->konsole_part->widget() )
   ) { // not collect output
      //TODO option to run them in paralell (not available for: collect output)
      for ( QStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
         if ( ! cmd.isEmpty() )
            cmd += " ; ";	//TODO make this separator configurable (users may want && or || for spec. actions)
         cmd += *it;
      }
      //run in TE
      if ( _action->execType() == KrAction::RunInTE ) {
         //send the commandline contents to the terminal emulator
         QKeyEvent keyEvent( QEvent::KeyPress, 0, -1, 0,  cmd+"\n");
         QApplication::sendEvent( MAIN_VIEW->konsole_part->widget(), &keyEvent );     
      } else { // will start a new process
         // run in terminal
         if ( _action->execType() == KrAction::Terminal ) {
           KConfigGroup group( krConfig, "UserActions" );
           QString term = group.readEntry( "Terminal", _UserActions_Terminal );

            if ( _action->user().isEmpty() )
               ( *_proc ) << term << cmd;
            else
//                ( *_proc )  << "kdesu" << "-u" << *_properties->user() << "-c" << K3Process::quote("konsole --noclose -e " + K3Process::quote(cmd) );
               ( *_proc )  << "kdesu" << "-u" << _action->user() << "-c" << K3Process::quote( term + " " + cmd );
         } else { // no terminal, no output collection, start&forget
            if ( _action->user().isEmpty() )
               ( *_proc ) << cmd;
            else
               ( *_proc ) << "kdesu" << "-u" << _action->user() << "-c" << K3Process::quote(cmd);
         }
         _proc->start( K3Process::NotifyOnExit, ( K3Process::Communication ) ( K3Process::Stdout | K3Process::Stderr ) );
      }
   }
   else { // collect output
      bool separateStderr = false;
      if ( _action->execType() == KrAction::CollectOutputSeparateStderr )
         separateStderr = true;
      _output = new KrActionProcDlg( _action->text(), separateStderr );
      // connect the output to the dialog
      connect( _proc, SIGNAL( receivedStderr( K3Process*, char*, int ) ), _output, SLOT( addStderr( K3Process*, char *, int ) ) );
      connect( _proc, SIGNAL( receivedStdout( K3Process*, char*, int ) ), _output, SLOT( addStdout( K3Process*, char *, int ) ) );
      connect( _output, SIGNAL( cancelClicked() ), this, SLOT( kill() ) );
      _output->show();
      for ( QStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
         if ( ! cmd.isEmpty() )
            cmd += " ; ";	//TODO make this separator configurable (users may want && or ||)
         //TODO: read header fom config or action-properties and place it on top of each command
         if ( cmdLineList.count() > 1 )
            cmd += "echo --------------------------------------- ; ";
         cmd += *it;
      }
      if ( _action->user().isEmpty() )
         ( *_proc ) << cmd;
      else
         // "-t" is nessesary that kdesu displays the terminal-output of the command
         ( *_proc ) << "kdesu" << "-t" << "-u" << _action->user() << "-c" << K3Process::quote(cmd);
      _proc->start( K3Process::NotifyOnExit, ( K3Process::Communication ) ( K3Process::Stdout | K3Process::Stderr ) );
   }

}

void KrActionProc::processExited( K3Process * ) {
   // enable the 'close' button on the dialog (if active), disable 'kill' button
   if ( _output ) {
      _output->enableButtonOk( true );
      _output->enableButtonCancel( false);
   }
   delete this; // banzai!!
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  KrAction  ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrAction::KrAction( KActionCollection *parent, const char * name ) : KAction( (QObject *)parent ) {
   if( name != 0 )
     _name = name;
   else
     _name = "";

   connect(this, SIGNAL(triggered()), this, SLOT(exec()) );
}

KrAction::~KrAction() {
   foreach (QWidget *w, associatedWidgets())
     w->removeAction(this);
   krUserAction->removeKrAction( this ); // Importent! Else Krusader will crash when writing the actions to file
}

bool KrAction::isAvailable( const KUrl& currentURL ) {
   bool available = true; //show per default (FIXME: make the default an attribute of <availability>)
   
   //check protocol
   if ( ! _showonlyProtocol.empty() ) {
      available = false;
      for ( QStringList::Iterator it = _showonlyProtocol.begin(); it != _showonlyProtocol.end(); ++it ) {
         //kDebug() << "KrAction::isAvailable currendProtocol: " << currentURL.protocol() << " =?= " << *it << endl;
         if ( currentURL.protocol() == *it ) {  // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
            available = true;
            break;
         }
      }
   } //check protocol: done
      
   //check the Path-list:
   if ( ! _showonlyPath.empty() ) {
      available = false;
      for ( QStringList::Iterator it = _showonlyPath.begin(); it != _showonlyPath.end(); ++it ) {
         if ( (*it).right(1) == "*" ){
             if ( currentURL.path().find( (*it).left( (*it).length() - 1 ) ) == 0 ) {
               available = true;
               break;
            }
         } else
         if ( currentURL.directory() == *it ) {  // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
            available = true;
            break;
         }
      }
   } //check the Path-list: done
   
   //check mime-type
   if ( ! _showonlyMime.empty() ) {
      available = false;
      KMimeType::Ptr mime = KMimeType::findByUrl( currentURL );
      for ( QStringList::Iterator it = _showonlyMime.begin(); it != _showonlyMime.end(); ++it ) {
         if ( (*it).contains("/") ) {
            if ( mime->is( *it ) ) {  // don't use ==; use 'is()' instead, which is aware of inheritence (ie: text/x-makefile is also text/plain)
               available = true;
               break;
            }
         }
         else {
            if ( mime->name().find( *it ) == 0 ) {  // 0 is the beginning, -1 is not found
               available = true;
               break;
            }
         }
      } //for
   } //check the mime-type: done
   
   //check filename
   if ( ! _showonlyFile.empty() ) {
      available = false;
      for ( QStringList::Iterator it = _showonlyFile.begin(); it != _showonlyFile.end(); ++it ) {
         QRegExp regex = QRegExp( *it, false, QRegExp::Wildcard ); // case-sensitive = false; wildcards = true
         if ( regex.exactMatch( currentURL.fileName() ) ) {
            available = true;
            break;
         }
      }
   } //check the filename: done
   
   return available;
}


bool KrAction::xmlRead( const QDomElement& element ) {
/*
   This has to be done elsewhere!!

   if ( element.tagName() != "action" )
      return false;

   Also the name has to be checked before the action is created!

   setName( element.attribute( "name" ).toLatin1() );
*/

   for ( QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
      QDomElement e = node.toElement();
      if ( e.isNull() )
         continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

      if ( e.tagName() == "title" )
         setText( e.text() );
      else
      if ( e.tagName() == "tooltip" )
         setToolTip( e.text() );
      else
      if ( e.tagName() == "icon" )
         setIcon( KIcon( _iconName = e.text() ) );
      else
      if ( e.tagName() == "category" )
         setCategory( e.text() );
      else
      if ( e.tagName() == "description" )
         setWhatsThis( e.text() );
      else
      if (e.tagName() == "command")
         readCommand( e );
      else
      if ( e.tagName() == "startpath" )
         setStartpath( e.text() );
      else
      if (e.tagName() == "availability")
         readAvailability( e );
      else
      if ( e.tagName() == "defaultshortcut" )
         setShortcut(  KShortcut( e.text() ) );
      else

      // unknown but not empty
      if ( ! e.tagName().isEmpty() )
         krOut << "KrAction::xmlRead() - unrecognized tag found: <action name=\"" << name() << "\"><" << e.tagName() << ">" << endl;

   } // for ( QDomNode node = action->firstChild(); !node.isNull(); node = node.nextSibling() )

   return true;
} //KrAction::xmlRead

QDomElement KrAction::xmlDump( QDomDocument& doc ) const {
   QDomElement actionElement = doc.createElement("action");
   actionElement.setAttribute( "name", (const char *)_name );

#define TEXT_ELEMENT( TAGNAME, TEXT ) \
   { \
   QDomElement e = doc.createElement( TAGNAME ); \
   e.appendChild( doc.createTextNode( TEXT ) ); \
   actionElement.appendChild( e ); \
   }

   TEXT_ELEMENT( "title", text() )

   if ( ! toolTip().isEmpty() )
      TEXT_ELEMENT( "tooltip", toolTip() )

   if ( ! _iconName.isEmpty() )
      TEXT_ELEMENT( "icon", _iconName )

   if ( ! category().isEmpty() )
      TEXT_ELEMENT( "category", category() )

   if ( ! whatsThis().isEmpty() )
      TEXT_ELEMENT( "description", whatsThis() )

   actionElement.appendChild( dumpCommand( doc ) );

   if ( ! startpath().isEmpty() )
      TEXT_ELEMENT( "startpath", startpath() )

   QDomElement availabilityElement = dumpAvailability( doc );
   if ( availabilityElement.hasChildNodes() )
      actionElement.appendChild( availabilityElement );

   if ( ! shortcut().isEmpty() )
      TEXT_ELEMENT( "defaultshortcut", shortcut().toString() )  //.toString() would return a localised string which can't be read again

   return actionElement;
} //KrAction::xmlDump

void KrAction::readCommand( const QDomElement& element ) {
   QString attr;

   attr = element.attribute( "executionmode", "normal" ); // default: "normal"
   if ( attr == "normal")
      setExecType( Normal );
   else
   if ( attr == "terminal" )
      setExecType( Terminal );
   else if ( attr == "collect_output")
      setExecType( CollectOutput );
   else if ( attr == "collect_output_separate_stderr")
      setExecType( CollectOutputSeparateStderr );
   else
      krOut << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << name() << "\"><command executionmode=\"" << attr << "\""<< endl;

   attr = element.attribute( "accept", "local" ); // default: "local"
   if ( attr == "local" )
      setAcceptURLs( false );
   else if ( attr == "url")
      setAcceptURLs( true );
   else
        krOut << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << name() << "\"><command accept=\"" << attr << "\""<< endl;

   attr = element.attribute( "confirmexecution", "false" ); // default: "false"
   if ( attr == "true" )
      setConfirmExecution( true );
   else
      setConfirmExecution( false );

   setUser( element.attribute( "run_as" ) );

   setCommand( element.text() );

} //KrAction::readCommand

QDomElement KrAction::dumpCommand( QDomDocument& doc ) const {
   QDomElement commandElement = doc.createElement("command");

   switch ( execType() ) {
   case Terminal:
      commandElement.setAttribute( "executionmode", "terminal" );
      break;
   case CollectOutput:
      commandElement.setAttribute( "executionmode", "collect_output" );
      break;
   case CollectOutputSeparateStderr:
      commandElement.setAttribute( "executionmode", "collect_output_separate_stderr" );
      break;
   default:
      // don't write the default to file
      break;
   }

   if ( acceptURLs() )
      commandElement.setAttribute( "accept", "url" );

   if ( confirmExecution() )
      commandElement.setAttribute( "confirmexecution", "true" );

   if ( ! user().isEmpty() )
      commandElement.setAttribute( "run_as", user() );

   commandElement.appendChild( doc.createTextNode( command() ) );

   return commandElement;
} //KrAction::dumpCommand

void KrAction::readAvailability( const QDomElement& element ) {
   for ( QDomNode node = element.firstChild(); ! node.isNull(); node = node.nextSibling() ) {
      QDomElement e = node.toElement();
      if ( e.isNull() )
         continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

      QStringList* showlist = 0;

      if ( e.tagName() == "protocol" )
         showlist = &_showonlyProtocol;
      else
      if ( e.tagName() == "path" )
         showlist = &_showonlyPath;
      else
      if ( e.tagName() == "mimetype" )
         showlist = & _showonlyMime;
      else
      if ( e.tagName() == "filename" )
         showlist = & _showonlyFile;
      else {
         krOut << "KrAction::readAvailability() - unrecognized element found: <action name=\"" << name() << "\"><availability><" << e.tagName() << ">"<< endl;
         showlist = 0;
      }

      if ( showlist ) {
          for ( QDomNode subnode = e.firstChild(); ! subnode.isNull(); subnode = subnode.nextSibling() ) {
            QDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" )
              showlist->append( subelement.text() );
          } // for
        } // if ( showlist )

   } // for
} //KrAction::readAvailability

QDomElement KrAction::dumpAvailability( QDomDocument& doc ) const {
   QDomElement availabilityElement = doc.createElement("command");

# define LIST_ELEMENT( TAGNAME, LIST ) \
   { \
   QDomElement e = doc.createElement( TAGNAME ); \
   for ( QStringList::const_iterator it = LIST.constBegin(); it != LIST.constEnd(); ++it ) { \
      QDomElement show = doc.createElement( "show" ); \
      show.appendChild( doc.createTextNode( *it ) ); \
      e.appendChild( show ); \
      } \
   availabilityElement.appendChild( e ); \
   }

   if ( ! _showonlyProtocol.isEmpty() )
      LIST_ELEMENT( "protocol", _showonlyProtocol )

   if ( ! _showonlyPath.isEmpty() )
      LIST_ELEMENT( "path", _showonlyPath )

   if ( ! _showonlyMime.isEmpty() )
      LIST_ELEMENT( "mimetype", _showonlyMime )

   if ( ! _showonlyFile.isEmpty() )
      LIST_ELEMENT( "filename", _showonlyFile )

   return availabilityElement;
} //KrAction::dumpAvailability

#include "kraction.moc"
