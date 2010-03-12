/***************************************************************************
                          syncbrowsebutton.h  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas B�hr
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
#include <kicon.h>

//#include <kdebug.h>

SyncBrowseButton::SyncBrowseButton(QWidget *parent) : QToolButton(parent)
{
    setFixedSize(16 + 4, 16 + 4);
    setIcon(KIcon("kr_syncbrowse_off"));
    setCheckable(true);

    setText(i18n("This button toggles the sync-browse mode.\n"
                 "When active, each directory change is performed in the\n"
                 "active and inactive panel - if possible."));   //set this as toop-tip (somehow whatsthis::add(this, ...) don't work)
    setToolTip(i18n("This button toggles the sync-browse mode.\n"
                    "When active, each directory change is performed in the\n"
                    "active and inactive panel - if possible."));   //set this as toop-tip (somehow whatsthis::add(this, ...) don't work)

    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
}

SyncBrowseButton::~SyncBrowseButton()
{
}

void SyncBrowseButton::slotToggled(bool on)
{
    if (on)
        setIcon(KIcon("kr_syncbrowse_on"));
    else
        setIcon(KIcon("kr_syncbrowse_off"));
}

int SyncBrowseButton::state()
{
    if (isChecked())
        _state = SYNCBROWSE_CD;
    else
        _state = SYNCBROWSE_OFF;

    return _state;
}


#include "syncbrowsebutton.moc"
