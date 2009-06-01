/*****************************************************************************
 * Copyright (C) 2005 Jonas Bähr <jonas.baehr@web.de>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRJS_H
#define KRJS_H

#include <kjsembed/kjsembedpart.h>

/**
 * Our own version of KJSEmbedPart.
 * Here are all the Krusader-specific extensions implemented.
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
