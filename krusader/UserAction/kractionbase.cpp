//
// C++ Implementation: kractionbase
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kinputdialog.h>
#include <klocale.h>

#include <qerrormessage.h>

#include "kraction.h"
#include "expander.h"
#include "kractionbase.h"

KrActionBase::~KrActionBase()
{
}

void KrActionBase::exec() {
	KrActionProc *proc;

   // replace %% and prepare string
	QStringList commandList;
	if(doSubstitution()) {
		Expander exp;
		exp.expand(command(),acceptURLs());
		if(exp.error()) {
			handleError(exp.error());
			return;
		}
		commandList=exp.result();
	} else
		commandList << command();
   //TODO: query expander for status and may skip the rest of the function

   // stop here if the commandline is empty
   if ( commandList.count() == 1 && commandList[0].trimmed().isEmpty() )
       return;

   if ( confirmExecution() ) {
       for ( QStringList::iterator it = commandList.begin(); it != commandList.end(); ++it ) {
           bool exec = true;
           *it = KInputDialog::getText(
        i18n( "Confirm execution" ),
        i18n( "Command being executed:" ),
        *it,
        &exec, 0
      );
      if ( exec ) {
        proc = actionProcFactoryMethod();
        proc->start( *it );
      }
    } //for
  } // if ( _properties->confirmExecution() )
  else {
    proc = actionProcFactoryMethod();
    proc->start( commandList );
  }

}

void KrActionBase::handleError(const Error& err)
{
	QErrorMessage::qtHandler()->message(err.what());
}

KrActionProc* KrActionBase::actionProcFactoryMethod()
{
	return new KrActionProc(this);
}
