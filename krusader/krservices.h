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

class QTextStream;
class QFile;

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrServices {
public: 
	KrServices(){}
	~KrServices(){}

	static bool         cmdExist(QString cmdName);
	static QString      detectFullPathName( QString name );
	static QString      fullPathName( QString name, QString confName = QString() );
	static QStringList  separateArgs( QString args );
	static QString      registerdProtocol(QString mimetype);
	static void         clearProtocolCache();
	static bool         fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines=false);
	static bool         fileToStringList(QFile *file, QStringList& target, bool keepEmptyLines=false);
	static QString		  quote( QString name );
	static QStringList  quote( const QStringList& names );

protected:
	static QString 	  escape( QString name );

private:
	static QMap<QString,QString>* slaveMap;

};


// TODO: make KrServices a namespace and move it there

#endif
