/***************************************************************************
                         kguseractions.h  -  description
                             -------------------
    copyright            : (C) 2004 by Jonas Bähr
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

#ifndef __KGUSERACTIONS_H__
#define __KGUSERACTIONS_H__

#include "konfiguratorpage.h"

class KListBox;

//class QString;
class UserActionXML;
class ActionProperty;
class KPushButton;

class KgUserActions : public KonfiguratorPage
{
  Q_OBJECT

public:
  KgUserActions( bool first, QWidget* parent=0,  const char* name=0 );
  ~KgUserActions();

public slots:
  void slotChangeAction();	//loads a new action into the detail-view
  void slotUpdateAction();	//updates the action to the xml-file
  void slotNewAction();
  void slotRemoveAction();
  void slotReset();	//restets the action to the last state where the user had presst "Ok"
  void slotImport();
  void slotExport();

protected:
 KListBox *actionList;
 ActionProperty *actionProperties;
 KPushButton *okButton, *resetButton, *newButton, *removeButton, *importButton, *exportButton;
 UserActionXML *_importXML;	//holds all the actions w/ nameconflicts after an import
};

#endif /* __KGUSERACTIONS_H__ */
