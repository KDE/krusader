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
  if( QFile( krConfig->readEntry( cmdName, QString() )).exists() )
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

  KConfigGroup config = krConfig->group( "Dependencies" );
  if( QFile( supposedName = config.readEntry( confName, QString() )).exists() )
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

bool KrServices::fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines) {
	if (!stream) return false;
	QString line;
	while ( !stream->atEnd() ) {
		line = stream->readLine().trimmed();
		if (keepEmptyLines || !line.isEmpty()) target.append(line);
	}
	return true;
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
	for (unsigned i=0; i<names.size(); ++i)
		result.append(quote(names[i]));
	return result;
}

QString KrServices::escape( QString name ) {
  const QString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n";		// stuff that should get escaped
     
    for ( unsigned int i = 0; i < evilstuff.length(); ++i )
        name.replace( evilstuff[ i ], ('\\' + evilstuff[ i ]) );

  return name;
}


// ------- KEasyProcess
KEasyProcess::KEasyProcess(QObject *parent, const char *name): K3Process(parent) {
	init();
}

KEasyProcess::KEasyProcess(): K3Process() {
	init();
}

void KEasyProcess::init() {
	connect(this, SIGNAL(receivedStdout(K3Process *, char *, int)),
		this, SLOT(receivedStdout(K3Process *, char *, int)));
	connect(this, SIGNAL(receivedStderr(K3Process *, char *, int)),
		this, SLOT(receivedStderr(K3Process *, char *, int)));
}

void KEasyProcess::receivedStdout (K3Process * /* proc */, char *buffer, int buflen) {
	_stdout+=QString::fromLocal8Bit(buffer, buflen);
}

void KEasyProcess::receivedStderr (K3Process * /* proc */, char *buffer, int buflen) {
	_stderr+=QString::fromLocal8Bit(buffer, buflen);
}
