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
#include "krusader.h"

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
