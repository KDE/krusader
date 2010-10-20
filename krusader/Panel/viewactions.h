/***************************************************************************
                                viewactions.h
                           -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2010 by Jan Lepper
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __VIEWACTIONS_H__
#define __VIEWACTIONS_H__

#include "../actionsbase.h"

#include <kaction.h>

class KrView;

class ViewActions : public ActionsBase
{
    Q_OBJECT
public:
    ViewActions(QObject *parent, KrMainWindow *mainWindow);

public slots:
    //zoom 
    void zoomIn();
    void zoomOut();
    void defaultZoom();

    //filter
    void allFilter();
    //void execFilter();
    void customFilter();

    // selection
    void markAll();
    void unmarkAll();
    void markGroup();
    void markGroup(const QString &, bool select);
    void unmarkGroup();
    void invertSelection();

    void showOptionsMenu();
    void saveDefaultSettings();
    void applySettingsToOthers();
    void focusPanel();

public:
    KAction *actZoomIn, *actZoomOut, *actDefaultZoom;
    KAction *actSelect, *actUnselect, *actSelectAll, *actUnselectAll, *actInvert;

protected:
    KrView *view();
};

#endif // __VIEWACTIONS_H__
