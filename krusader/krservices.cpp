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
// KDE includes
#include <kdebug.h>
// Krusader includes
#include "krservices.h"
#include "krusader.h"

QMap<QString,QString>* KrServices::slaveMap=0;

bool KrServices::cmdExist(QString cmdName)
{
  QString lastGroup = krConfig->group();

  krConfig->setGroup( "Dependencies" );
  if( QFile( krConfig->readEntry( cmdName, QString::null )).exists() )
  {
    krConfig->setGroup( lastGroup );
    return true;
  }

  krConfig->setGroup( lastGroup );
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
  QString lastGroup = krConfig->group();
  QString supposedName;

  if( confName.isNull() )
    confName = name;

  krConfig->setGroup( "Dependencies" );
  if( QFile( supposedName = krConfig->readEntry( confName, "" )).exists() )
  {
    krConfig->setGroup( lastGroup );
    return supposedName;
  }

  if( ( supposedName = detectFullPathName( name ) ).isEmpty() )
  {
    krConfig->setGroup( lastGroup );
    return "";
  }

  krConfig->writeEntry( confName, supposedName );
  krConfig->setGroup( lastGroup );
  return supposedName;
}

QStringList KrServices::separateArgs( QString args )
{
  QStringList argList;
  int   pointer = 0, tokenStart, len = args.length();
  bool  quoted = false, slashed = false;
  QChar quoteCh;

  do{
      while( pointer < len && args[ pointer ].isSpace() )
        pointer++;

      if( pointer >= len )
        break;

      tokenStart = pointer;

      QString result="";
      
      while( pointer < len && ( quoted || slashed || !args[ pointer ].isSpace()) )
      {
        slashed = false;
        
        if( !quoted && ( args[pointer] == '"' || args[pointer] == '\'' ) )
          quoted = true, quoteCh = args[pointer];
        else if( quoted && args[pointer] == quoteCh )
          quoted = false;
        else if( !quoted && args[pointer] == '\\' )
        {
          pointer++;
          slashed = true;
          continue;
        }

        result += args[pointer];        
        pointer++;
      }

      result = result.stripWhiteSpace();
      if( result.startsWith( "'" ) && result.endsWith( "'" ) )
        result = result.mid( 1, result.length()-2 );
      else if( result.startsWith( "\"" ) && result.endsWith( "\"" ) )
        result = result.mid( 1, result.length()-2 );

      if( !result.isEmpty() )
        argList.append( result );
      
    }while( pointer < len );
    
  return argList;
}

QString KrServices::registerdProtocol(QString mimetype){
	if( slaveMap == 0 ){
		slaveMap = new QMap<QString,QString>();
		
		krConfig->setGroup( "Protocols" );
		QStringList protList = krConfig->readListEntry( "Handled Protocols" );
		for( QStringList::Iterator it = protList.begin(); it != protList.end(); it++ ){
			QStringList mimes = krConfig->readListEntry( QString( "Mimes For %1" ).arg( *it ) );
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
