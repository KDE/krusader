/***************************************************************************
                          syncbrowsebutton.h  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas Bähr
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************
This is the button which toggles the synchron-browse-mode (a directory-change
is done in both panels)
I could imagine an optional extension which also performs mkdir etc. in the other panel
or in ALL tabs on the other side (this could also include copy-actions to this panels)
This is very handy if you have several identical clients which you want to update
simoultanious.

The current version only manages sync-browse and got no mode-switch options.
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "syncbrowsebutton.h"

#include <klocale.h>
#include <kiconloader.h>

//#include <kdebug.h>

SyncBrowseButton::SyncBrowseButton(QWidget *parent, const char *name) : QToolButton(parent,name)
{
  KIconLoader *iconLoader = new KIconLoader();
  _icon_on = iconLoader->loadIcon( "kr_syncbrowse_on", KIcon::Toolbar, 16 );
  _icon_off = iconLoader->loadIcon( "kr_syncbrowse_off", KIcon::Toolbar, 16 );

  setFixedSize( _icon_off.width() + 4, _icon_off.height() + 4 );
  setPixmap( _icon_off );
  setToggleButton( true );
  
  setTextLabel( i18n( "This button toggles the sync-browse mode.\n"
			"If active, each directory change is performed in the\n"
			"active and inactive panel - if possible." ), true );	//set this as toop-tip (somehow whatsthis::add(this, ...) don't work)

  connect( this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)) );
}

SyncBrowseButton::~SyncBrowseButton() {
}

void SyncBrowseButton::slotToggled( bool on ) {
  if ( on )
    setPixmap( _icon_on );
  else
    setPixmap( _icon_off );
}

int SyncBrowseButton::state() {
  if ( isOn() )
    _state = SYNCBROWSE_CD;
  else
    _state = SYNCBROWSE_OFF;
  
  return _state;
}


#include "syncbrowsebutton.moc"
