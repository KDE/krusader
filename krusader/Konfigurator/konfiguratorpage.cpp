/* **************************************************************************
                      konfiguratorpage.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konfiguratorpage.h"
#include <qlayout.h>
#include "../krusader.h"
#include <qtooltip.h>

KonfiguratorPage::KonfiguratorPage( bool firstTime, QWidget* parent,  const char* name ) :
  QFrame( parent, name ), firstCall( firstTime )
{
}

bool KonfiguratorPage::apply()
{
  bool restartNeeded = false;

  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    restartNeeded = item->apply() || restartNeeded;
    item = itemList.next();
  }

  krConfig->sync();  
  return restartNeeded;
}

void KonfiguratorPage::setDefaults()
{
  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    item->setDefaults();
    item = itemList.next();
  }
}

void KonfiguratorPage::loadInitialValues()
{
  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    item->loadInitialValue();
    item = itemList.next();
  }
}

bool KonfiguratorPage::isChanged()
{
  KonfiguratorExtension *currentItem = itemList.current();  /* save the current pointer */
  bool isChanged = false;

  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    isChanged = isChanged || item->isChanged();
    item = itemList.next();
  }

  itemList.find( currentItem );  /* restore the current pointer */
  return isChanged;
}

KonfiguratorCheckBox* KonfiguratorPage::createCheckBox( QString cls, QString name,
    bool dflt, QString text, QWidget *parent, bool rst, QString toolTip )
{
  KonfiguratorCheckBox *checkBox = new KonfiguratorCheckBox( cls, name, dflt, text,
                                 parent, QString(cls + "/" + name).ascii(), rst );
  if( !toolTip.isEmpty() )
    QToolTip::add( checkBox, toolTip );
  
  registerObject( checkBox->extension() );
  return checkBox;
}

KonfiguratorSpinBox* KonfiguratorPage::createSpinBox(  QString cls, QString name,
    int dflt, int min, int max, QWidget *parent, bool rst )
{
  KonfiguratorSpinBox *spinBox = new KonfiguratorSpinBox( cls, name, dflt, min, max,
                                 parent, QString(cls + "/" + name).ascii(), rst );

  registerObject( spinBox->extension() );
  return spinBox;
}

KonfiguratorEditBox* KonfiguratorPage::createEditBox(  QString cls, QString name,
    QString dflt, QWidget *parent, bool rst )
{
  KonfiguratorEditBox *editBox = new KonfiguratorEditBox( cls, name, dflt, parent,
                                        QString(cls + "/" + name).ascii(), rst );

  registerObject( editBox->extension() );
  return editBox;
}

KonfiguratorURLRequester* KonfiguratorPage::createURLRequester(  QString cls, QString name,
    QString dflt, QWidget *parent, bool rst )
{
  KonfiguratorURLRequester *urlRequester = new KonfiguratorURLRequester( cls, name, dflt,
                                        parent, QString(cls + "/" + name).ascii(), rst );

  registerObject( urlRequester->extension() );
  return urlRequester;
}

QGroupBox* KonfiguratorPage::createFrame( QString text, QWidget *parent,
                                          const char *widgetName )
{
  QGroupBox *groupBox = new QGroupBox( parent, widgetName );
  groupBox->setFrameShape( QGroupBox::Box );
  groupBox->setFrameShadow( QGroupBox::Sunken );
  if( !text.isNull() )
    groupBox->setTitle( text );
  groupBox->setColumnLayout(0, Qt::Vertical );
  groupBox->layout()->setSpacing( 0 );
  groupBox->layout()->setMargin( 0 );
  return groupBox;
}                                          

QGridLayout* KonfiguratorPage::createGridLayout( QLayout *parent )
{
  QGridLayout *gridLayout = new QGridLayout( parent );
  gridLayout->setAlignment( Qt::AlignTop );
  gridLayout->setSpacing( 6 );
  gridLayout->setMargin( 11 );
  return gridLayout;
}

QLabel* KonfiguratorPage::addLabel( QGridLayout *layout, int x, int y, QString label,
                                    QWidget *parent, const char *widgetName )
{
  QLabel *lbl = new QLabel( label, parent, widgetName );
  layout->addWidget( lbl, x, y );
  return lbl;
}

QWidget* KonfiguratorPage::createSpacer( QWidget *parent, const char *widgetName )
{
  QWidget *widget = new QWidget( parent, widgetName );
  QHBoxLayout *hboxlayout = new QHBoxLayout( widget );
  QSpacerItem* spacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hboxlayout->addItem( spacer );
  return widget;
}

KonfiguratorCheckBoxGroup* KonfiguratorPage::createCheckBoxGroup( int sizex, int sizey,
    KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum, QWidget *parent,
    const char *widgetName )
{
  KonfiguratorCheckBoxGroup *groupWidget = new KonfiguratorCheckBoxGroup( parent, widgetName );
  QGridLayout *layout = new QGridLayout( groupWidget );
  layout->setSpacing( 11 );
  layout->setMargin( 0 );
  
  int x = 0, y = 0;
  
  for( int i=0; i != paramNum; i++ )
  {
    KonfiguratorCheckBox *checkBox = createCheckBox( params[i].configClass,
      params[i].configName, params[i].defaultValue, params[i].text, groupWidget,
      params[i].restart, params[i].toolTip );

    groupWidget->add( checkBox );
    layout->addWidget( checkBox, y, x );

    if( sizex )
    {
      if( ++x == sizex )
        x = 0, y++;
    }
    else
    {
      if( ++y == sizey )
        y = 0, x++;
    }
  }
  
  return groupWidget;
}

KonfiguratorRadioButtons* KonfiguratorPage::createRadioButtonGroup( QString cls,
    QString name, QString dflt, int sizex, int sizey, KONFIGURATOR_NAME_VALUE_TIP *params,
    int paramNum, QWidget *parent, const char *widgetName, bool rst )
{
  KonfiguratorRadioButtons *radioWidget = new KonfiguratorRadioButtons( cls, name, dflt, parent, widgetName, rst );
  radioWidget->setFrameShape( QButtonGroup::NoFrame );
  radioWidget->setFrameShadow( QButtonGroup::Sunken );
  radioWidget->setTitle( "" );
  radioWidget->setExclusive( true );
  radioWidget->setRadioButtonExclusive( true );
  radioWidget->setColumnLayout(0, Qt::Vertical );

  QGridLayout *layout = new QGridLayout( radioWidget->layout() );
  layout->setAlignment( Qt::AlignTop );
  layout->setSpacing( 11 );
  layout->setMargin( 0 );

  int x = 0, y = 0;

  for( int i=0; i != paramNum; i++ )
  {
    QRadioButton *radBtn = new QRadioButton( params[i].text, radioWidget,
                        QString( cls + "/" + name + "/" + params[i].value ).ascii() );
    
    if( !params[i].tooltip.isEmpty() )
      QToolTip::add( radBtn, params[i].tooltip );

    layout->addWidget( radBtn, y, x );

    radioWidget->addRadioButton( radBtn, params[i].value );

    if( sizex )
    {
      if( ++x == sizex )
        x = 0, y++;
    }
    else
    {
      if( ++y == sizey )
        y = 0, x++;
    }
  }

  radioWidget->loadInitialValue();
  registerObject( radioWidget->extension() );  
  return radioWidget;
}

KonfiguratorFontChooser *KonfiguratorPage::createFontChooser( QString cls, QString name,
  QFont *dflt, QWidget *parent, bool rst )
{
  KonfiguratorFontChooser *fontChooser = new KonfiguratorFontChooser( cls, name, dflt, parent,
                                        QString(cls + "/" + name).ascii(), rst );

  registerObject( fontChooser->extension() );
  return fontChooser;
}

KonfiguratorComboBox *KonfiguratorPage::createComboBox(  QString cls, QString name, QString dflt,
    KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum, QWidget *parent, bool rst, bool editable )
{
  KonfiguratorComboBox *comboBox = new KonfiguratorComboBox( cls, name, dflt, params,
                                        paramNum, parent, QString(cls + "/" + name).ascii(),
                                        rst, editable );

  registerObject( comboBox->extension() );
  return comboBox;
}

QFrame* KonfiguratorPage::createLine( QWidget *parent, const char *widgetName )
{
  QFrame *line = new QFrame( parent, widgetName );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  return line;
}

void KonfiguratorPage::registerObject( KonfiguratorExtension *item )
{
  KonfiguratorExtension *currentItem = itemList.current();
  
  itemList.append( item );
  connect( item, SIGNAL( sigChanged( bool ) ), this, SIGNAL ( sigChanged( ) ) );

  itemList.find( currentItem );
}

void KonfiguratorPage::removeObject( KonfiguratorExtension *item )
{
  if( item == itemList.current() )
  {
    itemList.remove();
    if( itemList.current() != itemList.getFirst() )
      itemList.prev();
  }
  else
    itemList.removeRef( item );
}

KonfiguratorColorChooser *KonfiguratorPage::createColorChooser( QString cls, QString name, QColor dflt,
                                                                QWidget *parent, bool rst,
                                                                ADDITIONAL_COLOR *addColPtr, int addColNum )
{
  KonfiguratorColorChooser *colorChooser = new KonfiguratorColorChooser( cls, name, dflt,  parent,
                                        QString(cls + "/" + name).ascii(), rst, addColPtr, addColNum );

  registerObject( colorChooser->extension() );
  return colorChooser;
}

#include "konfiguratorpage.moc"
