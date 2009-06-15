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

#ifndef SYNCBROWSEBUTTON_H
#define SYNCBROWSEBUTTON_H

#include <QtGui/QToolButton>
#include <QPixmap>

// No synchrone browsing
#define SYNCBROWSE_OFF  0
// Change only the directory
#define SYNCBROWSE_CD   1
/*
// Make new dirs in both panels
#define SYNCBROWSE_MKDIR  2
// Delete in both panels
#define SYNCBROWSE_DELETE  4

// Do everything in all tabs on the other side (not only the oposite panel)
#define SYNCBROWSE_ALLTABS  1024
// Copy files not only to the other panel but to all tabs on the other side
#define SYNCBROWSE_COPY  2048
*/

class SyncBrowseButton : public QToolButton
{
    Q_OBJECT
public:
    SyncBrowseButton(QWidget *parent = 0);
    ~SyncBrowseButton();

    int state();

protected:
    int _state;
    QPixmap _icon_on;
    QPixmap _icon_off;

private slots:
    void slotToggled(bool on);
};

#endif
