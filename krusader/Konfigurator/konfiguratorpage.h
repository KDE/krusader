/***************************************************************************
                       konfiguratorpage.h  -  description
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

#ifndef __KONFIGURATOR_PAGE_H__
#define __KONFIGURATOR_PAGE_H__
 
#include "konfiguratoritems.h"
#include <qframe.h>
#include <qptrlist.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

struct KONFIGURATOR_CHECKBOX_PARAM;
struct KONFIGURATOR_NAME_VALUE_PAIR;
   
class KonfiguratorPage : public QFrame
{
  Q_OBJECT
   
public:
  KonfiguratorPage( bool firstTime, QWidget* parent,  const char* name );

  virtual bool apply();
  virtual void setDefaults();
  virtual void loadInitialValues();
  virtual bool isChanged();

  inline  bool isFirst()   {return firstCall;}

  KonfiguratorCheckBox    *createCheckBox( QString cls, QString name, bool dflt,
                                           QString text, QWidget *parent=0, bool rst=false,
                                           QString toolTip = QString::null );
  KonfiguratorSpinBox     *createSpinBox(  QString cls, QString name, int dflt, int min,
                                           int max, QWidget *parent = 0, bool rst = false );
  KonfiguratorEditBox     *createEditBox(  QString cls, QString name, QString dflt,
                                           QWidget *parent=0, bool rst=false );
  KonfiguratorURLRequester *createURLRequester(  QString cls, QString name,
                                           QString dflt, QWidget *parent, bool rst );
  KonfiguratorFontChooser *createFontChooser(  QString cls, QString name, QFont *dflt,
                                           QWidget *parent=0, bool rst=false );
  KonfiguratorComboBox    *createComboBox(  QString cls, QString name, QString dflt,
                                           KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum,
                                           QWidget *parent=0, bool rst=false );

  QGroupBox               *createFrame( QString text = QString::null, QWidget *parent=0,
                                           const char *widgetName=0 );

  QGridLayout             *createGridLayout( QLayout *parent );
  QLabel                  *addLabel( QGridLayout *layout, int x, int y, QString label,
                                           QWidget *parent=0, const char *widgetName=0 );
  QWidget                 *createSpacer( QWidget *parent=0, const char *widgetName=0 );
  QFrame                  *createLine( QWidget *parent, const char *widgetName );
  QWidget                 *createCheckBoxGroup( int sizex, int sizey,
                                           KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum,
                                           QPtrList<KonfiguratorCheckBox> &cbList,
                                           QWidget *parent=0, const char *widgetName=0 );
  KonfiguratorRadioButtons *createRadioButtonGroup( QString cls, QString name, 
                                           QString dflt, int sizex, int sizey,
                                           KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum,
                                           QWidget *parent=0, const char *widgetName=0, bool rst=false );
  void                    registerObject( KonfiguratorExtension *item );
  void                    removeObject( KonfiguratorExtension *item );

signals:
  void  sigChanged( bool );
  
protected:
  QPtrList<KonfiguratorExtension> itemList;

private:
  bool  firstCall;
};

struct KONFIGURATOR_CHECKBOX_PARAM
{
  QString configClass;
  QString configName;
  bool    defaultValue;
  QString text;
  bool    restart;
  QString toolTip;
};

#endif /* __KONFIGURATOR_PAGE_H__ */
