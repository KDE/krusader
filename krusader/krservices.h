/***************************************************************************
                          krservices.h  -  description
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

#ifndef KRSERVICES_H
#define KRSERVICES_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrServices {
public: 
	KrServices(){}
	~KrServices(){}

	static bool         cmdExist(QString cmdName);
	static QString      detectFullPathName( QString name );
	static QString      fullPathName( QString name, QString confName = QString::null );
	static QStringList  separateArgs( QString args );
	static QString      registerdProtocol(QString mimetype);
	static void         clearProtocolCache();

private:
	static QMap<QString,QString>* slaveMap;

};

#endif
