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

#include <kdialogbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <qtextedit.h>
#include <qvbox.h>
#include <qlabel.h>
#include <kaction.h>
#include <kurl.h>
#include <kmessagebox.h>
#include "kraction.h"
#include "expander.h"
#include "useractionproperties.h"
#include "../krusader.h"

//for the availabilitycheck:
#include <kmimetype.h>
#include <qregexp.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  KrActionProcDlg  /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrActionProcDlg::KrActionProcDlg( QString caption, bool enableStderr, QWidget *parent ) :
KDialogBase( parent, 0, false, caption, KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Cancel ) {

   setButtonOK( i18n( "Close" ) );
   enableButtonOK( false ); // disable the close button, until the process finishes

   setButtonCancel( KGuiItem(i18n("Kill"), i18n( "Kill the running process" )) );

   QVBox *page = makeVBoxMainWidget();
   // do we need to separate stderr and stdout?
   if ( enableStderr ) {
      // create stdout
      new QLabel( i18n( "Standard Output (stdout)" ), page );
      _stdout = new QTextEdit( page );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
      // create stderr
      new QLabel( i18n( "Standard Error (stderr)" ), page );
      _stderr = new QTextEdit( page );
      _stderr->setReadOnly( true );
      _stderr->setMinimumWidth( fontMetrics().maxWidth() * 40 );
   } else {
      // create stdout
      new QLabel( i18n( "Output" ), page );
      _stdout = new QTextEdit( page );
      _stdout->setReadOnly( true );
      _stdout->setMinimumWidth( fontMetrics().maxWidth() * 40 );
   }
}

void KrActionProcDlg::addStderr( KProcess *, char *buffer, int buflen ) {
   _stderr->append( QString::fromLatin1( buffer, buflen ) );
}

void KrActionProcDlg::addStdout( KProcess *, char *buffer, int buflen ) {
   _stdout->append( QString::fromLatin1( buffer, buflen ) );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  KrActionProc  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

KrActionProc::KrActionProc( UserActionProperties* prop ) : QObject(), _properties( prop ), _proc( new KProcess() ), _output( 0 ) {
   _proc->setUseShell( true );

   connect( _proc, SIGNAL( processExited( KProcess* ) ),
            this, SLOT( processExited( KProcess* ) ) ) ;
}

KrActionProc::~KrActionProc() {
   delete _proc;
}

void KrActionProc::start( QString cmdLine ) {
   QStringList list = cmdLine;
   start( list );
}

void KrActionProc::start( QStringList cmdLineList ) {
   _proc->clearArguments();
   QString cmd;
   
   if ( ! _properties->startpath()->isEmpty() )
      _proc->setWorkingDirectory( *_properties->startpath() );
   
   if ( _properties->execType() == UserActionProperties::Terminal && cmdLineList.count() > 1)
      KMessageBox::sorry( 0, "Support for more then one command doesn't work in a terminal. Only the first is executed in the terminal" );

   if ( _properties->execType() != UserActionProperties::CollectOutput ) {
      //TODO option to run them in paralell (not available for: collect output)
      for ( QStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
         if ( ! cmd.isEmpty() )
            cmd += " ; ";	//TODO make this separator configurable (users may want && or || for spec. actions)
         cmd += *it;
      }
      // run in terminal
      if ( _properties->execType() == UserActionProperties::Terminal ) {
         // TODO read terminal-setting from config
         if ( _properties->user()->isEmpty() )
            ( *_proc ) << "konsole" << "--noclose" << "-e" << cmd;
         else
//             ( *_proc )  << "kdesu" << "-u" << *_properties->user() << "-c" << KProcess::quote("konsole --noclose -e " + KProcess::quote(cmd) );
            ( *_proc )  << "kdesu" << "-u" << *_properties->user() << "-c" << KProcess::quote("konsole --noclose -e " + cmd );
      } else { // no terminal, no output collection
         if ( _properties->user()->isEmpty() )
            ( *_proc ) << cmd;
         else
            ( *_proc ) << "kdesu" << "-u" << *_properties->user() << "-c" << KProcess::quote(cmd);
      }
     _proc->start( KProcess::NotifyOnExit, ( KProcess::Communication ) ( KProcess::Stdout | KProcess::Stderr ) );
   }
   else { // collect output
      _output = new KrActionProcDlg( *_properties->title(), _properties->separateStderr() );
      // connect the output to the dialog
      connect( _proc, SIGNAL( receivedStderr( KProcess*, char*, int ) ), _output, SLOT( addStderr( KProcess*, char *, int ) ) );
      connect( _proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ), _output, SLOT( addStdout( KProcess*, char *, int ) ) );
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
      if ( _properties->user()->isEmpty() )
         ( *_proc ) << cmd;
      else
         // "-t" is nessesary that kdesu displays the terminal-output of the command
         ( *_proc ) << "kdesu" << "-t" << "-u" << *_properties->user() << "-c" << KProcess::quote(cmd);
      _proc->start( KProcess::NotifyOnExit, ( KProcess::Communication ) ( KProcess::Stdout | KProcess::Stderr ) );
   }

}

void KrActionProc::processExited( KProcess * ) {
   // enable the 'close' button on the dialog (if active), disable 'kill' button
   if ( _output ) {
      _output->enableButtonOK( true );
      _output->enableButtonCancel( false);
   }
   delete this; // banzai!!
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////  KrAction  ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

//KrAction::KrAction( UserActionProperties* prop, KActionCollection *parent ) : KAction( Title, Shortcut, QObject, SLOT(), parent, name ) {
KrAction::KrAction( UserActionProperties* prop, KActionCollection *parent ) : KAction( *prop->title(), 0, 0, 0, parent, prop->name()->latin1() ) {
   _properties = 0;
   setProperties( prop );
   connect(this, SIGNAL(activated()), this, SLOT(exec()) );
}

KrAction::~KrAction() {
   unplugAll();
   delete _properties;
}

void KrAction::setProperties( UserActionProperties* prop ) {
   if ( ! prop->icon()->isEmpty() )
      setIcon( *prop->icon() );
   if ( ! prop->tooltip()->isEmpty() ) {
      setToolTip( *prop->tooltip() );
      if ( prop->descriptionUseTooltip() )
         setWhatsThis( *prop->tooltip() );
   }
   if ( ! prop->description()->isEmpty() && whatsThis().isEmpty() )
      setWhatsThis( *prop->description() );
   if ( ! prop->defaultShortcut()->isNull() )
      setShortcut( *prop->defaultShortcut() );
        
   //setGroup( prop->category() ); //FIXME: what is KAction.setGroup(QString) for ???
   
   if ( _properties == 0 )
      _properties = prop;
   else
      _properties->copyFrom( prop );	//copy the values, not the pointer
}

UserActionProperties* KrAction::properties() {
   return _properties;
}

void KrAction::exec() {
   KrActionProc *proc;
   
   // replace %% and prepare string
   QStringList commandList = krExpander->expand( *_properties->command(), _properties->acceptURLs() );
   
   // stop here if the commandline is empty
   if ( commandList.count() == 1 && commandList[0].stripWhiteSpace().isEmpty() )
      return;
   
   if ( _properties->confirmExecution() ) {
      for ( QStringList::iterator it = commandList.begin(); it != commandList.end(); ++it ) {
         bool exec = true;
         *it = KInputDialog::getText(
      i18n( "Confirm execution" ),
      i18n( "Command being executed:" ),
      *it,
      &exec, 0 );
         if ( exec ) {
            proc = new KrActionProc( _properties );
            proc->start( *it );
         }
      } //for
   } // if ( _properties->confirmExecution() )
   else {
      proc = new KrActionProc( _properties );
      proc->start( commandList );
   }

}

bool KrAction::isAvailable( const KURL& currentURL ) {
   bool available = true; //show per default (FIXME: make the default an attribute of <availability>)
   
   //check protocol
   if ( ! _properties->showonlyProtocol()->empty() ) {
      available = false;
      for ( QStringList::Iterator it = _properties->showonlyProtocol()->begin(); it != _properties->showonlyProtocol()->end(); ++it ) {
         //kdDebug() << "KrAction::isAvailable currendProtocol: " << currentURL.protocol() << " =?= " << *it << endl;
         if ( currentURL.protocol() == *it ) {  // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
            available = true;
            break;
         }
      }
   } //check protocol: done
      
   //check the Path-list:
   if ( ! _properties->showonlyPath()->empty() ) {
      available = false;
      for ( QStringList::Iterator it = _properties->showonlyPath()->begin(); it != _properties->showonlyPath()->end(); ++it ) {
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
   if ( ! _properties->showonlyMime()->empty() ) {
      available = false;
      KMimeType::Ptr mime = KMimeType::findByURL( currentURL );
      for ( QStringList::Iterator it = _properties->showonlyMime()->begin(); it != _properties->showonlyMime()->end(); ++it ) {
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
   if ( ! _properties->showonlyFile()->empty() ) {
      available = false;
      for ( QStringList::Iterator it = _properties->showonlyFile()->begin(); it != _properties->showonlyFile()->end(); ++it ) {
         QRegExp regex = QRegExp( *it, false, true ); // case-sensitive = false; wildcards = true
         if ( regex.exactMatch( currentURL.fileName() ) ) {
            available = true;
            break;
         }
      }
   } //check the filename: done
   
   return available;
}


#include "kraction.moc"
