/***************************************************************************
                   krremoteencodingmenu.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
    based on             : KRemoteEncodingPlugin from Dawit Alemayehu
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

#include "krremoteencodingmenu.h"

#include <kactioncollection.h>
#include <kmenu.h>
#include <kcharsets.h>
#include <kio/slaveconfig.h>
#include <kio/scheduler.h>

#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../kicons.h"

#define DATA_KEY    QString::fromLatin1("Charset")

KrRemoteEncodingMenu::KrRemoteEncodingMenu(const QString &text, const QString &icon, KActionCollection *parent) :
  KActionMenu( KIcon( icon, krLoader ), text, parent ), settingsLoaded( false )
{
  connect(menu(), SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));

  parent->addAction("changeremoteencoding", this);
}

void KrRemoteEncodingMenu::slotAboutToShow()
{
  if (!settingsLoaded)
    loadSettings();

  // uncheck everything
  for (unsigned i =  0; i < menu()->count(); i++)
    menu()->setItemChecked(menu()->idAt(i), false);

  KUrl currentURL = ACTIVE_PANEL->virtualPath();

  QString charset = KIO::SlaveConfig::self()->configData(currentURL.protocol(), currentURL.host(), DATA_KEY);
  if (!charset.isEmpty())
  {
    int id = 1;
    QStringList::Iterator it;
    for (it = encodingNames.begin(); it != encodingNames.end(); ++it, ++id)
      if ((*it).find(charset) != -1)
        break;

//     kDebug() << k_funcinfo << "URL=" << currentURL << " charset=" << charset << endl;

    if (it == encodingNames.end())
      kWarning() << k_funcinfo << "could not find entry for charset=" << charset << endl;
    else
      menu()->setItemChecked(id, true);
  }
  else
    menu()->setItemChecked(defaultID, true);
}

void KrRemoteEncodingMenu::loadSettings()
{
  settingsLoaded = true;
  encodingNames = KGlobal::charsets()->descriptiveEncodingNames();

  KMenu *kmenu = menu();
  kmenu->clear();

  QStringList::ConstIterator it;
  int count = 0;
  for (it = encodingNames.begin(); it != encodingNames.end(); ++it)
    kmenu->insertItem(*it, this, SLOT(slotItemSelected(int)), 0, ++count);
  kmenu->insertSeparator();

  kmenu->insertItem(i18n("Reload"), this, SLOT(slotReload()), 0, ++count);
  kmenu->insertItem(i18n("Default"), this, SLOT(slotDefault()), 0, ++count);
  defaultID = count;
}

void KrRemoteEncodingMenu::slotItemSelected(int id)
{
  KUrl currentURL = ACTIVE_PANEL->virtualPath();

  KConfig config(("kio_" + currentURL.protocol() + "rc").toLatin1());
  QString host = currentURL.host();

  QString charset = KGlobal::charsets()->encodingForName( encodingNames[id - 1] );

  KConfigGroup group( &config, host);
  group.writeEntry(DATA_KEY, charset);
  config.sync();

  // Update the io-slaves...
  updateKIOSlaves();
}

void KrRemoteEncodingMenu::slotReload()
{
  loadSettings();
}

void KrRemoteEncodingMenu::slotDefault()
{
  KUrl currentURL = ACTIVE_PANEL->virtualPath();

  // We have no choice but delete all higher domain level
  // settings here since it affects what will be matched.
  KConfig config ( ( "kio_" + currentURL.protocol() + "rc" ).toLatin1() );

  QStringList partList = currentURL.host().split ( '.', QString::SkipEmptyParts );
  if ( !partList.isEmpty() )
  {
    partList.erase ( partList.begin() );

    QStringList domains;
    // Remove the exact name match...
    domains << currentURL.host();

    while ( partList.count() )
    {
      if ( partList.count() == 2 )
        if ( partList[0].length() <= 2 && partList[1].length() == 2 )
          break;

      if ( partList.count() == 1 )
        break;

      domains << partList.join ( "." );
      partList.erase ( partList.begin() );
    }

    for ( QStringList::Iterator it = domains.begin(); it != domains.end(); ++it )
    {
      kDebug() << "Domain to remove: " << *it;
      if ( config.hasGroup ( *it ) )
        config.deleteGroup ( *it );
      else if ( config.group ( "" ).hasKey ( *it ) )
        config.group ( "" ).deleteEntry ( *it ); //don't know what group name is supposed to be XXX
    }
  }
  config.sync();

  updateKIOSlaves();
}


void KrRemoteEncodingMenu::updateKIOSlaves()
{
  KIO::Scheduler::emitReparseSlaveConfiguration();

  // Reload the page with the new charset
  QTimer::singleShot( 500, ACTIVE_FUNC, SLOT( refresh() ) );
}

#include "krremoteencodingmenu.moc"
