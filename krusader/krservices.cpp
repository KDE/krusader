/***************************************************************************
                          krservices.cpp  -  description
                             -------------------
    begin                : Thu Aug 8 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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

#include <stdlib.h>
#include <unistd.h>
// QT includes
#include <qstringlist.h>
#include <qdir.h>
// KDE includes
#include <kdebug.h>
// Krusader includes
#include "krservices.h"

bool KrServices::cmdExist(QString cmdName){
  QStringList path = QStringList::split(":",getenv("PATH"));

	for ( QStringList::Iterator it = path.begin(); it != path.end(); ++it ) {
    if( QDir(*it).exists(cmdName) ){
			return true;
		}
	}
	
	return false;
}
