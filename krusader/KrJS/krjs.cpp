// Implementation of our JavaScript-Interpreter
//
// Author: Jonas B�r (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//

#include "krjs.h"

#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

#include <kmessagebox.h>

KrJS::KrJS() : KJSEmbed::KJSEmbedPart() {

   // make this object, the object Krusader, available for scripting as "Krusader":
   addObject( krApp, "Krusader" );

   // make this object available for scripting
   addObject( ACTIVE_MNG, "PanelManager" );
}

bool KrJS::runFile(const QString & filename) {

   KJS::ExecState *exec = globalExec();

   // set up the variable scriptDir with the directory of the current script
   putValue( "scriptDir", KJSEmbed::convertToValue(exec, KUrl(filename).directory(false) ) );

   bool ok = KJSEmbedPart::runFile(filename);

   KJS::Completion jsres = completion();

   // this is based on this example-code http://webcvs.kde.org/kdebindings/kjsembed/kjscmd.cpp?rev=1.28&view=auto
   if ( jsres.complType() != KJS::Normal) {
      switch ( jsres.complType() ) {
         case KJS::Break:
         case KJS::Continue:
            // TODO: find out what this means
            krOut << "JavaScript: " << jsres.value().toString(exec).qstring() << endl;
            break;
         case KJS::ReturnValue:
            // that's only needed when a specific function is called
            //someString = jsres.value().toString(exec).qstring();
            break;
         case KJS::Throw:
            /*
              This is the interesting part: If the JS throws an exception which is not handled inside the script,
              we can catch it here and handle it with c++
            */
            {
#if KDE_IS_VERSION(3,4,0)
            KJS::Object exception = jsres.value().toObject(exec);
            int line = int( exception.get( exec, KJS::Identifier("line") ).toNumber(exec) );
            QString type = exception.get( exec, KJS::Identifier("name") ).toString(exec).qstring();
            QString message = exception.get( exec, KJS::Identifier("message") ).toString(exec).qstring();

            krOut << "JavaScript: Uncaught " << type << " exception at line " << line <<  " in " << filename << endl;
            krOut << message << endl;

            KMessageBox::error ( 0,	//parent
		( line < 0 ?
		QString( i18n("In %1:\nUncaught JavaScript exception '%2'\n%3") ).arg(filename).arg(type).arg(message) :
		QString( i18n("In %1:\nUncaught JavaScript exception '%2' at line %3\n%4") ).arg(filename).arg(type).arg(line).arg(message)
		),	//text
		i18n("JavaScript error"), 	//caption
		KMessageBox::Dangerous) ;
#else
            KMessageBox::error ( 0,	//parent
		QString(i18n("In %1:\nThere is an error in the JavaScript")).arg(filename),	//text
		i18n("JavaScript error"), 	//caption
		KMessageBox::Dangerous) ;
#endif
            break;
            }
         default:
            krOut << "JavaScript: Unknown error." << endl;
            break;
      } // switch
   } // if


   krOut << "JS: done" << endl;

   return ok;
}

