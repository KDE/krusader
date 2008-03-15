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
#include <qdir.h>
#include <qtextstream.h>
// KDE includes
#include <kdebug.h>
// Krusader includes
#include "krservices.h"
#include "krusader.h"

QMap<QString,QString>* KrServices::slaveMap=0;

bool KrServices::cmdExist(QString cmdName)
{
  KConfigGroup group( krConfig, "Dependencies" );
  if( QFile( group.readEntry( cmdName, QString() )).exists() )
    return true;

  return !detectFullPathName( cmdName ).isEmpty();  
}

QString KrServices::detectFullPathName(QString name)
{
  QStringList path = QStringList::split(":",getenv("PATH"));

  for ( QStringList::Iterator it = path.begin(); it != path.end(); ++it )
  {
    if( QDir(*it).exists( name ) )
    {
      QString dir = *it;
      if( !dir.endsWith( "/" ) )
        dir+="/";
        
      return dir+name;
    }
  }

  return "";
}

QString KrServices::fullPathName( QString name, QString confName )
{
  QString supposedName;

  if( confName.isNull() )
    confName = name;

  KConfigGroup config( krConfig, "Dependencies" );
  if( QFile( supposedName = config.readEntry( confName, QString() )).exists() )
    return supposedName;

  if( ( supposedName = detectFullPathName( name ) ).isEmpty() )
    return "";

  config.writeEntry( confName, supposedName );
  return supposedName;
}

// TODO: Document me!
QStringList KrServices::separateArgs( QString args )
{
  QStringList argList;
  int   pointer = 0, tokenStart, len = args.length();
  bool  quoted = false;
  QChar quoteCh;

  do{
      while( pointer < len && args[ pointer ].isSpace() )
        pointer++;

      if( pointer >= len )
        break;

      tokenStart = pointer;

      QString result="";
      
      for(; pointer < len && ( quoted || !args[ pointer ].isSpace()) ; pointer++)
      {
        if( !quoted && ( args[pointer] == '"' || args[pointer] == '\'' ) ) {
          quoted = true, quoteCh = args[pointer];
          continue;
        }
        else if( quoted && args[pointer] == quoteCh ) {
          quoted = false;
          continue;
        }
        else if( !quoted && args[pointer] == '\\' )
        {
          pointer++;
          if(pointer>=len) break;
        }

        result += args[pointer];        
      }

      argList.append( result );
      
    }while( pointer < len );
    
  return argList;
}

QString KrServices::registerdProtocol(QString mimetype){
	if( slaveMap == 0 ){
		slaveMap = new QMap<QString,QString>();
		
		KConfigGroup group( krConfig, "Protocols" );
		QStringList protList = group.readEntry( "Handled Protocols", QStringList() );
		for( QStringList::Iterator it = protList.begin(); it != protList.end(); it++ ){
			QStringList mimes = group.readEntry( QString( "Mimes For %1" ).arg( *it ), QStringList() );
			for( QStringList::Iterator it2 = mimes.begin(); it2 != mimes.end(); it2++ )
				(*slaveMap)[*it2] = *it;
  		}
		
		
	}
	return (*slaveMap)[mimetype];
}

void KrServices::clearProtocolCache()
{
  if( slaveMap )
    delete slaveMap;
  slaveMap = 0;
}

bool KrServices::fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines) {
	if (!stream) return false;
	QString line;
	while ( !stream->atEnd() ) {
		line = stream->readLine().trimmed();
		if (keepEmptyLines || !line.isEmpty()) target.append(line);
	}
	return true;
}

bool KrServices::fileToStringList(QFile *file, QStringList& target, bool keepEmptyLines) {
  QTextStream stream(file);
  return fileToStringList(&stream, target, keepEmptyLines);
}

QString KrServices::quote( QString name ) {
  if( !name.contains( '\'' ) )
    return "'" + name + "'";
  if( !name.contains( '"' ) && !name.contains( '$' ) )
    return "\"" + name + "\"";
  return escape( name );
}

QStringList KrServices::quote( const QStringList& names ) {
	QStringList result;
	for (int i=0; i<names.size(); ++i)
		result.append(quote(names[i]));
	return result;
}

QString KrServices::escape( QString name ) {
  const QString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n";		// stuff that should get escaped
     
    for ( int i = 0; i < evilstuff.length(); ++i )
        name.replace( evilstuff[ i ], ('\\' + evilstuff[ i ]) );

  return name;
}

