/***************************************************************************
                         kglookfeel.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#ifndef __KGLOOKFEEL_H__
#define __KGLOOKFEEL_H__

#include "konfiguratorpage.h"
#include <kedittoolbar.h>
#include <kkeydialog.h>

class KonfiguratorEditToolbarWidget;
class KonfiguratorKeyChooser;

class KgLookFeel : public KonfiguratorPage
{
  Q_OBJECT

public:
  KgLookFeel( bool first, QWidget* parent=0,  const char* name=0 );

protected:
  KonfiguratorCheckBoxGroup *cbs;
  KonfiguratorCheckBoxGroup *pnlcbs;
  KonfiguratorCheckBoxGroup *panelToolbarActive;
  
  QWidget     *tab_panel;
  
  QGridLayout *toolBarLayout;
  QWidget     *tab_2;
  QGridLayout *keyBindingsLayout;
  QWidget     *tab_3;

protected slots:
  void slotReload( KonfiguratorEditToolbarWidget * oldEditToolbar );
  void slotReload( KonfiguratorKeyChooser * oldChooser );
  void slotDisable();
  void slotEnablePanelToolbar();
  void slotImportShortcuts();
  void slotExportShortcuts();
  
private:
  KonfiguratorKeyChooser *keyBindings;
};

class KonfiguratorEditToolbarWidget : public KonfiguratorExtension
{
  Q_OBJECT

public:
  KonfiguratorEditToolbarWidget( KXMLGUIFactory *factory, QWidget *parent, bool restart=false ) :
    KonfiguratorExtension( this, QString(), QString(), restart )
  {
    editToolbar = new KEditToolbarWidget( factory, parent );
    connect( editToolbar, SIGNAL( enableOk( bool ) ), this, SLOT( setChanged() ) );
  }

  ~KonfiguratorEditToolbarWidget()
  {
    delete editToolbar;
  }
  
  KEditToolbarWidget *editToolbarWidget() { return editToolbar; }

  virtual bool apply()
  {
    if( !changed )
      return false;
    editToolbar->save();
    setChanged( false );
    return restartNeeded;
  }
  
  virtual void setDefaults()      { if( changed ) emit reload( this ); }
  virtual void loadInitialValue() { if( changed ) emit reload( this ); }

signals:
  void reload( KonfiguratorEditToolbarWidget * );
  
private:  
  KEditToolbarWidget *editToolbar;
};

class KonfiguratorKeyChooser : public KonfiguratorExtension
{
  Q_OBJECT
  
public:
  KonfiguratorKeyChooser( KActionCollection *actColl, QWidget *parent, bool restart=false ) :
    KonfiguratorExtension( this, QString(), QString(), restart )
  {
    keyChooser = new KKeyChooser( actColl, parent );
    connect( keyChooser, SIGNAL( keyChange() ), this, SLOT( setChanged() ) );
  }

  ~KonfiguratorKeyChooser()
  {
    delete keyChooser;
  }
  
  KKeyChooser *keyChooserWidget() { return keyChooser; }

  virtual bool apply()
  {
    if( !changed )
      return false;
    keyChooser->save();
    setChanged( false );
    return restartNeeded;
  }

  virtual void setDefaults()      { if( changed ) emit reload( this ); }
  virtual void loadInitialValue() { if( changed ) emit reload( this ); }

signals:
  void reload( KonfiguratorKeyChooser * );

private:
  KKeyChooser *keyChooser;
};

#endif /* __KGLOOKFEEL_H__ */
