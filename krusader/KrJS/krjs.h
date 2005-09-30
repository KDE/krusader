// Interface for our JavaScript-Interpreter
//
// Author: Jonas Bähr (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef KRJS_H
#define KRJS_H

#include <kjsembed/kjsembedpart.h>

/**
  * Our own version of KJSEmbedPart. Here are all the Krusader-specific extensions implemented.
  *
  * @author Jonas Bähr (http://jonas-baehr.de/)
*/

class KrJS: public KJSEmbed::KJSEmbedPart {
public:
   KrJS();

   /**
    * This loads and runs a file. In addition to the original runFile function it displays an popup
    * on errors and sets some variables
    *
    * @par filename The file to run
    */
   bool runFile(const QString & filename);
};

#endif //KRJS_H
