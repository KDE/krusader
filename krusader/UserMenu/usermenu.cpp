/***************************************************************************
                      _expressionsu.cpp  -  description
                         -------------------
begin                : Sat Dec 6 2003
copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usermenu.h"
#include "usermenuaddimpl.h"
#include "../krusaderview.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <qvbox.h>
#include <kurl.h>

/**

Command: %xYYY%
         x - can be either 'a' for active panel, or 'o' for other panel
         YYY - the specific command

         For example:
           %ap% - active panel path
           %op% - other panel path

In the following commands, we'll use '_' instead of 'a'/'o'. Please substitute as needed.

%_p%    - panel path
%_anf%  - number of files
%_and%  - number of folders
%_an%   - number of files and folders

%_c%    - current file (or folder). Note: current != selected
%_s%    - selected files and folders
%_cs%   - selected files including current file (if it's not already selected)
%_afd%  - all files and folders
%_af%   - all files (not including folders)
%_ad%   - all folders (not including files)
%_fm%   - filter mask (for example: *, *.cpp, *.h etc.)

*/
UMCmd UserMenu::expressions[ UserMenu::numOfExps ] = {
         {"%_p%", "panel's path", exp_p},
         {"%_anf%", "no. of files", exp_anf},
         {"%_and%", "no. of folders", exp_and},
         {"%_an%", "no. of files and folders", exp_an}
      };

#define ACTIVE    krApp->mainView->activePanel
#define OTHER     krApp->mainView->activePanel->otherPanel
#define GETPANEL  (str.lower()[1]=='a' ? ACTIVE : OTHER)

QString UserMenu::exp_p( const QString& str, const bool& useUrl ) {
   if (useUrl) return GETPANEL->func->files()->vfs_getOrigin().url();
   else return GETPANEL->func->files()->vfs_getOrigin().path();
}

QString UserMenu::exp_anf( const QString& str, const bool& useUrl ) {
   return QString("%1").arg(GETPANEL->view->numFiles());
}

QString UserMenu::exp_and( const QString& str, const bool& useUrl ) {
   return QString("%1").arg(GETPANEL->view->numDirs());
}

QString UserMenu::exp_an( const QString& str, const bool& useUrl ) {
   return QString("%1").arg(GETPANEL->view->numDirs()+GETPANEL->view->numFiles());
}

void UserMenu::exec() {
   // execute menu and wait for selection
   UserMenuEntry cmd = _popup->run();

   // replace %% and prepare string
   cmd = expand( cmd.cmdline, cmd.acceptURLs );
   //kdWarning() << cmd << endl;

   // ............... run the cmd from the shell .............
   //QString save = getcwd( 0, 0 );
   //===> chdir( panelPath.local8Bit() ); // run the command in the panel's path

   // run in  terminal
   //proc.setUseShell(true);
   //proc << "konsole" << "--noclose" << "-e" << cmd;
   //proc.start( KProcess::DontCare );

   UserMenuProc *proc = new UserMenuProc( UserMenuProc::Terminal, true );
   proc->start( cmd.cmdline );
   //===> chdir( save.local8Bit() ); // chdir back
}

QString UserMenu::expand( QString str, bool useUrl ) {
   QString result = QString::null, exp = QString::null;
   int beg, end, i;
   unsigned int idx = 0;
   while ( idx < str.length() ) {
      if ( ( beg = str.find( '%', idx ) ) == -1 ) break;
      if ( ( end = str.find( '%', beg + 1 ) ) == -1 ) {
         kdWarning() << "Error: unterminated % in UserMenu::expand" << endl;
         return QString::null;
      }
      result += str.mid( idx, beg - idx ); // copy until the start of %exp%

      // get the expression, and expand it using the correct expander function
      // ... replace first char with _ to ease the checking
      exp = str.mid( beg, end - beg + 1 );
      for ( i = 0; i < numOfExps; ++i )
         if ( str.mid( beg, end - beg + 1 ).replace( 1, 1, '_' ) == expressions[ i ].expression ) {
            result += ( expressions[ i ].expFunc ) ( exp, useUrl );
            break;
         }
      if ( i == numOfExps ) { // didn't find an expander
         kdWarning() << "Error: unrecognized " << exp << " in UserMenu::expand" << endl;
         return QString::null;
      }
      idx = end + 1;
   }
   // copy the rest of the string
   result += str.mid( idx );

   return result;
}

UserMenu::UserMenu( QWidget * parent, const char * name ) : QWidget( parent, name ) {
   _popup = new UserMenuGui(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserMenuEntry::UserMenuEntry(QString data) {
   // the data should look like:
   // {name,cmdline,execType,separateStderr,acceptURLs,acceptRemote,showEverywhere,showIn}
   // name
   int sidx = data.find('{') + 1;
   int eidx = data.find(',', sidx);
   name = data.mid(sidx, eidx-sidx);
   // cmdline
   sidx = eidx+1;
   eidx = data.find(',', sidx);
   cmdline = data.mid(sidx, eidx-sidx);
   // execType
   sidx = eidx+1;
   eidx = data.find(',', sidx);
   execType = (UserMenuProc::ExecType)data.mid(sidx, eidx-sidx).toInt();
   // separateStderr
   sidx = eidx+1;
   eidx = data.find(',', sidx);
   separateStderr = data.mid(sidx, eidx-sidx).toInt();
   // acceptUrls
   sidx = eidx+1;
   eidx = data.find(',', sidx);
   acceptURLs = data.mid(sidx, eidx-sidx).toInt();
   // showEverywhere
   sidx = eidx+1;
   eidx = data.find(',', sidx);
   showEverywhere = data.mid(sidx, eidx-sidx).toInt();
   // showIn
   sidx = eidx+1;
   eidx = data.find('}', sidx);
   showIn = QStringList::split(";", data.mid(sidx, eidx-sidx));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserMenuGui::UserMenuGui( UserMenu *menu, QWidget * parent ) : KPopupMenu( parent ) {
   createMenu();

   // create the 'add entry' gui
   _addgui = new UserMenuAddImpl(menu, this);
   connect(_addgui, SIGNAL(newEntry(QString, QString, UserMenuProc::ExecType, bool, bool, bool, QStringList )),
               this, SLOT(addEntry(QString, QString, UserMenuProc::ExecType, bool, bool, bool, QStringList )));
}

void UserMenuGui::createMenu() {
   clear();
   insertTitle( "User Menu" );

   // read entries from config file.
   readEntries();

   // add the "add new entry" command
   insertSeparator();
   insertItem( "Add new entry", 0 );
}

void UserMenuGui::readEntries() {
   // Note: entries are marked 1..n, so that entry 0 is always
   // available. It is used by the "add new entry" command.
   QString filename = locateLocal( "data", "krusader/krusermenu.dat" );
   _entries.clear();
   int idx = 1;

   QFile file( filename );
   if ( file.open( IO_ReadOnly ) ) {
      QTextStream stream( &file );
      QString line;

      while ( !stream.atEnd() ) {
         line = stream.readLine();
         if (!line.simplifyWhiteSpace().isEmpty())
            _entries.append(UserMenuEntry(line));
            insertItem( _entries.last().name, idx++ );
      }
      file.close();
   }
}

UserMenuEntry UserMenuGui::run() {
   // disable the entries that should not appear in this folder
   for (uint i=1; i<count(); ++i) {
      if (_entries[i-1].showEverywhere)
         setItemVisible(i, true); // enable the item, it is displayed everywhere
      else { // see if the current path is in the listpanel
         bool showHere = false;
         QString currentPath = ACTIVE->func->files()->vfs_getOrigin().path();
         for ( QStringList::Iterator it = _entries[i-1].showIn.begin();
               it != _entries[i-1].showIn.end();
               ++it ) {
            // is the folder == current path? (don't forget to remove trailing /s
            if (currentPath == ((*it).right(1)=="/" ? (*it).left((*it).length()-1) : (*it))) {
               showHere = true;
               break;
            }
         }
         setItemVisible(i, showHere);
      }
   }


   int idx = exec();
   if ( idx == -1 ) return QString::null; // nothing was selected
   if ( idx == 0 ) {
      _addgui->exec();
      return QString::null;
   }

   // don't forget: index 0 is reserved for the 'add entry' command. that's why
   // the entries is 0..n-1 and actual menu is 1..n
   return _entries[idx-1];
}

void UserMenuGui::addEntry(QString name, QString cmdline, UserMenuProc::ExecType execType, bool separateStderr,
                  bool acceptURLs, bool showEverywhere, QStringList showIn) {
   // save a new entry in the following form:
   // {"name","cmdline",execType,separateStderr,acceptURLs,acceptRemote,showEverywhere,showIn}
   QString filename = locateLocal( "data", "krusader/krusermenu.dat" );

   QFile file( filename );
   if ( file.open( IO_WriteOnly | IO_Append ) ) {
      QTextStream stream( &file );
      QString line = QString("{%1,%2,%3,%4,%5,%6,%7}").arg(name)
                                                      .arg(cmdline)
                                                      .arg(execType)
                                                      .arg(separateStderr)
                                                      .arg(acceptURLs)
                                                      .arg(showEverywhere)
                                                      .arg(showIn.join(";"));
      stream << line << endl;
      file.close();
   }
   // update the menu, by re-reading the entries
   createMenu();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserMenuProcDlg::UserMenuProcDlg( QString caption, bool enableStderr, QWidget *parent ) :
KDialogBase( parent, 0, false, caption, KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Cancel ) {

   setButtonOKText( "Close", i18n( "Close this window" ) );
   enableButtonOK( false ); // disable the close button, until the process finishes

   setButtonCancelText( "Kill", i18n( "Kill the running process" ) );

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

void UserMenuProcDlg::addStderr( KProcess *proc, char *buffer, int buflen ) {
   _stderr->append( QString::fromLatin1( buffer, buflen ) );
}

void UserMenuProcDlg::addStdout( KProcess *proc, char *buffer, int buflen ) {
   _stdout->append( QString::fromLatin1( buffer, buflen ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


UserMenuProc::UserMenuProc( ExecType execType, bool enableStderr ) : QObject(), _execType( execType ),
_enableStderr( enableStderr ), _proc( new KProcess() ), _output( 0 ) {
   _proc->setUseShell( true );

   connect( _proc, SIGNAL( processExited( KProcess* ) ),
            this, SLOT( processExited( KProcess* ) ) ) ;
}


UserMenuProc::~UserMenuProc() {
   delete _proc;
}

bool UserMenuProc::start( QString cmdLine ) {
   _proc->clearArguments();

   // run in terminal or in custom-window
   if ( _execType == Terminal ) {
      ( *_proc ) << "konsole" << "--noclose" << "-e" << cmdLine;
   } else if ( _execType == OutputOnly ) { // output only - collect output
      ( *_proc ) << cmdLine;
      _output = new UserMenuProcDlg( cmdLine, _enableStderr );
      // connect the output to the dialog
      connect( _proc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
               _output, SLOT( addStderr( KProcess*, char *, int ) ) );
      connect( _proc, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
               _output, SLOT( addStdout( KProcess*, char *, int ) ) );
      connect( _output, SIGNAL( cancelClicked() ), this, SLOT( kill() ) );
      _output->show();
   } else { // no terminal, no output collection
      ( *_proc ) << cmdLine;
   }
   return _proc->start( KProcess::NotifyOnExit, ( KProcess::Communication ) ( KProcess::Stdout | KProcess::Stderr ) );
}

void UserMenuProc::processExited( KProcess *proc ) {
   // enable the 'close' button on the dialog (if active), disable 'kill' button
   if ( _output ) {
      _output->enableButtonOK( true );
      _output->enableButtonCancel( false);
   }
   delete this; // banzai!!
}
