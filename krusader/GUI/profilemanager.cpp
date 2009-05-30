/***************************************************************************
                      profilemanager.cpp  -  description
                             -------------------
    copyright            : (C) 2004 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../krusader.h"
#include "profilemanager.h"

#include <klocale.h>
#include <kmenu.h>
#include <QtGui/QCursor>
#include <kinputdialog.h>
#include <kiconloader.h>

ProfileManager::ProfileManager( QString profileType, QWidget * parent ) 
  : QPushButton( parent )
{
  setText( "" );
  KIconLoader *iconLoader = new KIconLoader();
  QPixmap pm = iconLoader->loadIcon( "kr_profile", KIconLoader::Toolbar, 16 );
  setIcon( QIcon( pm ) );
  setFixedWidth( pm.width() + 15 );
  setToolTip( i18n( "Profiles" ) );
  
  this->profileType = profileType;
  
  connect( this, SIGNAL( clicked() ), this, SLOT( profilePopup() ) );

  KConfigGroup group( krConfig, "Private");
  profileList = group.readEntry( profileType, QStringList() );
}

void ProfileManager::profilePopup()
{
  // profile menu identifiers
  #define ADD_NEW_ENTRY_ID    1000
  #define LOAD_ENTRY_ID       2000
  #define REMOVE_ENTRY_ID     3000
  #define OVERWRITE_ENTRY_ID  4000
  
  // create the menu
  KMenu popup, removePopup, overwritePopup;
  popup.setTitle(i18n("Profiles"));
  
  for( int i=0; i != profileList.count() ; i++ )
  {
    KConfigGroup group( krConfig, profileType + " - " + profileList[i] ); 
    QString name = group.readEntry( "Name" );
    popup.addAction( name )->setData( QVariant( (int)( LOAD_ENTRY_ID + i ) ) );
    removePopup.addAction( name )->setData( QVariant( (int)( REMOVE_ENTRY_ID + i ) ) );
    overwritePopup.addAction( name )->setData( QVariant( (int)( OVERWRITE_ENTRY_ID + i ) ) );
  }

  popup.addSeparator();
  
  if( profileList.count() )
  {
    popup.addMenu( &removePopup )->setText( i18n("Remove entry") );
    popup.addMenu( &overwritePopup )->setText( i18n("Overwrite entry") );
  }
  
  popup.addAction(i18n("Add new entry") )->setData( QVariant( (int)( ADD_NEW_ENTRY_ID) ) );


  int result = 0;
  QAction * res = popup.exec(QCursor::pos());
  if( res && res->data().canConvert<int>() )
    result = res->data().toInt();

  // check out the user's selection
  if( result == ADD_NEW_ENTRY_ID )
    newProfile();
  else if( result >= LOAD_ENTRY_ID && result < LOAD_ENTRY_ID + profileList.count() )
  {
    emit loadFromProfile( profileType + " - " + profileList[ result - LOAD_ENTRY_ID ] );
  }else if( result >= REMOVE_ENTRY_ID && result < REMOVE_ENTRY_ID + profileList.count() )
  { 
    krConfig->deleteGroup( profileType + " - " + profileList[ result - REMOVE_ENTRY_ID ] );    
    profileList.removeAll( profileList[ result - REMOVE_ENTRY_ID ] );
  
    KConfigGroup group( krConfig, "Private");
    group.writeEntry( profileType, profileList );
    krConfig->sync();
  }else if( result >= OVERWRITE_ENTRY_ID && result < OVERWRITE_ENTRY_ID + profileList.count() )
  {
    emit saveToProfile( profileType + " - " + profileList[ result - OVERWRITE_ENTRY_ID ] );
  }  
}

void ProfileManager::newProfile( QString defaultName )
{
  QString profile = KInputDialog::getText( i18n( "Krusader::ProfileManager" ), i18n( "Enter the profile name:" ), 
                                           defaultName, 0, this );  
  if( !profile.isEmpty() )
  {
    int profileNum = 1;
    while( profileList.contains( QString( "%1" ).arg( profileNum ) ) )
      profileNum++;

    QString profileString = QString( "%1" ).arg( profileNum );
    QString profileName = profileType + " - " + profileString;
    profileList.append( QString( "%1" ).arg( profileString ) );
  
    KConfigGroup group( krConfig, "Private");
    group.writeEntry( profileType, profileList );
      
    KConfigGroup pg( krConfig, profileName );
    pg.writeEntry( "Name", profile );
    emit saveToProfile( profileName );
    krConfig->sync();
  }
}
  
void ProfileManager::deleteProfile( QString name )
{
  for( int i=0; i != profileList.count() ; i++ )
  {
    KConfigGroup group( krConfig, profileType + " - " + profileList[ i ] ); 
    QString currentName = group.readEntry( "Name" );
    
    if( name == currentName )
    {
      krConfig->deleteGroup( profileType + " - " + profileList[ i ] );    
      profileList.removeAll( profileList[ i ] );
  
      KConfigGroup pg( krConfig, "Private");
      pg.writeEntry( profileType, profileList );
      krConfig->sync();
      return;
    }
  }
}
  
void ProfileManager::overwriteProfile( QString name )
{
  for( int i=0; i != profileList.count() ; i++ )
  {
    KConfigGroup group( krConfig, profileType + " - " + profileList[ i ] ); 
    QString currentName = group.readEntry( "Name" );
    
    if( name == currentName )
    {
      emit saveToProfile( profileType + " - " + profileList[ i ] );
      return;
    }
  }
}

bool ProfileManager::loadProfile( QString name )
{
  for( int i=0; i != profileList.count() ; i++ )
  {
    KConfigGroup group( krConfig, profileType + " - " + profileList[i] ); 
    QString currentName = group.readEntry( "Name" );
    
    if( name == currentName )
    {
      emit loadFromProfile( profileType + " - " + profileList[ i ] );
      return true;
    }
  }
  return false;    
}

QStringList ProfileManager::availableProfiles( QString profileType ) {
  KConfigGroup group( krConfig, "Private");
  QStringList profiles = group.readEntry( profileType, QStringList() );
  QStringList profileNames;

  for( int i=0; i != profiles.count() ; i++ ) {
    KConfigGroup pg( krConfig, profileType + " - " + profiles[ i ] ); 
    profileNames.append( pg.readEntry("Name") );
  }

  return profileNames;
}

#include "profilemanager.moc"
