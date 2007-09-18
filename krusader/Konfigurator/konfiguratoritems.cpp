/***************************************************************************
                     konfiguratoritems.cpp  -  description
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

#include "konfiguratoritems.h"
#include "../krusader.h"
#include <klocale.h>
#include <qpainter.h>
#include <qpen.h>
#include <qcolordialog.h>
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>
#include <kiconloader.h>

KonfiguratorExtension::KonfiguratorExtension( QObject *obj, QString cfgClass, QString cfgName, bool rst, int pg) :
      QObject(), objectPtr( obj ), applyConnected( false ), setDefaultsConnected( false ),
      changed( false ), restartNeeded( rst ), subpage(pg), configClass( cfgClass ), configName( cfgName )
{
}

void KonfiguratorExtension::connectNotify( const char *signal )
{
  QString signalString    = QString( signal ).replace( " ", "" );
  QString applyString     = QString( SIGNAL( applyManually(QObject *,QString, QString) ) ).replace( " ", "" );
  QString defaultsString  = QString( SIGNAL( setDefaultsManually(QObject *) ) ).replace( " ", "" );

  if( signalString == applyString )
    applyConnected = true;
  else if ( signalString == defaultsString )
    setDefaultsConnected = true;

  QObject::connectNotify( signal );
}

bool KonfiguratorExtension::apply()
{
  if( !changed )
    return false;

  if( applyConnected )
    emit applyManually( objectPtr, configClass, configName );
  else
    emit applyAuto( objectPtr, configClass, configName );

  setChanged( false );
  return restartNeeded;
}

void KonfiguratorExtension::setDefaults()
{
  if( setDefaultsConnected )
    emit setDefaultsManually( objectPtr );
  else
    emit setDefaultsAuto( objectPtr );
}

void KonfiguratorExtension::loadInitialValue()
{
  emit setInitialValue( objectPtr );
}

bool KonfiguratorExtension::isChanged()
{
  return changed;
}

// KonfiguratorCheckBox class
///////////////////////////////

KonfiguratorCheckBox::KonfiguratorCheckBox( QString cls, QString name, bool dflt, QString text,
    QWidget *parent, bool rst, int pg ) : QCheckBox( text, parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( stateChanged( int ) ), ext, SLOT( setChanged() ) );
  loadInitialValue();
}

KonfiguratorCheckBox::~KonfiguratorCheckBox()
{
  delete ext;
}

void KonfiguratorCheckBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setChecked( krConfig->readBoolEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorCheckBox::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, isChecked() );
}

void KonfiguratorCheckBox::slotSetDefaults(QObject *)
{
  if( isChecked() != defaultValue )
    setChecked( defaultValue );
}

// KonfiguratorSpinBox class
///////////////////////////////

KonfiguratorSpinBox::KonfiguratorSpinBox( QString cls, QString name, int dflt, int min, int max,
    QWidget *parent, bool rst, int pg ) : QSpinBox( parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( valueChanged(int) ), ext, SLOT( setChanged() ) );

  setMinValue( min );
  setMaxValue( max );

  loadInitialValue();
}

KonfiguratorSpinBox::~KonfiguratorSpinBox()
{
  delete ext;
}

void KonfiguratorSpinBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setValue( krConfig->readNumEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorSpinBox::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value() );
}

void KonfiguratorSpinBox::slotSetDefaults(QObject *)
{
  if( value() != defaultValue )
    setValue( defaultValue );
}

// KonfiguratorCheckBoxGroup class
///////////////////////////////

void KonfiguratorCheckBoxGroup::add( KonfiguratorCheckBox *checkBox )
{
  checkBoxList.append( checkBox );
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find( int index )
{
  return checkBoxList.at( index );
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find( QString name )
{
  KonfiguratorCheckBox *checkBox = checkBoxList.first();

  while( checkBox )
  {
    if( checkBox->extension()->getCfgName() == name )
      return checkBox;
    checkBox = checkBoxList.next();
  }

  return 0;
}


// KonfiguratorRadioButtons class
///////////////////////////////

KonfiguratorRadioButtons::KonfiguratorRadioButtons( QString cls, QString name,
    QString dflt, QWidget *parent, bool rst, int pg ) :
    Q3ButtonGroup( parent ), defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );
}

KonfiguratorRadioButtons::~KonfiguratorRadioButtons()
{
  delete ext;
}

void KonfiguratorRadioButtons::addRadioButton( QRadioButton *radioWidget, QString name, QString value )
{
  radioButtons.append( radioWidget );
  radioNames.push_back( name );
  radioValues.push_back( value );

  connect( radioWidget, SIGNAL( toggled( bool ) ), ext, SLOT( setChanged() ) );
}

QRadioButton * KonfiguratorRadioButtons::find( int index )
{
  return radioButtons.at( index );
}

QRadioButton * KonfiguratorRadioButtons::find( QString name )
{
  int index = radioNames.findIndex( name );
  if( index == -1 )
    return 0;

  return radioButtons.at( index );
}

void KonfiguratorRadioButtons::selectButton( QString value )
{
  int cnt = 0;
  QRadioButton *btn  = radioButtons.first();

  while( btn )
  {
    if( value == radioValues[ cnt ] )
    {
      btn->setChecked( true );
      return;
    }

    btn = radioButtons.next();
    cnt++;
  }

  if( radioButtons.first() )
    radioButtons.first()->setChecked( true );
}

void KonfiguratorRadioButtons::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  QString initValue = krConfig->readEntry( ext->getCfgName(), defaultValue );

  selectButton( initValue );
  ext->setChanged( false );
}

void KonfiguratorRadioButtons::slotApply(QObject *,QString cls, QString name)
{
  QRadioButton *btn  = radioButtons.first();
  int cnt = 0;

  while( btn )
  {
    if( btn->isChecked() )
    {
      krConfig->setGroup( cls );
      krConfig->writeEntry( name, radioValues[ cnt ] );
      break;
    }

    btn = radioButtons.next();
    cnt++;
  }
}

void KonfiguratorRadioButtons::slotSetDefaults(QObject *)
{
  selectButton( defaultValue );
}

// KonfiguratorEditBox class
///////////////////////////////

KonfiguratorEditBox::KonfiguratorEditBox( QString cls, QString name, QString dflt,
    QWidget *parent, bool rst, int pg ) : QLineEdit( parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( textChanged(const QString &) ), ext, SLOT( setChanged() ) );

  loadInitialValue();
}

KonfiguratorEditBox::~KonfiguratorEditBox()
{
  delete ext;
}

void KonfiguratorEditBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setText( krConfig->readEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorEditBox::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, text() );
}

void KonfiguratorEditBox::slotSetDefaults(QObject *)
{
  if( text() != defaultValue )
    setText( defaultValue );
}


// KonfiguratorURLRequester class
///////////////////////////////

KonfiguratorURLRequester::KonfiguratorURLRequester( QString cls, QString name, QString dflt,
    QWidget *parent, bool rst, int pg ) : KUrlRequester( parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( textChanged(const QString &) ), ext, SLOT( setChanged() ) );

  button()->setIconSet( SmallIcon( "fileopen" ) );
  loadInitialValue();
}

KonfiguratorURLRequester::~KonfiguratorURLRequester()
{
  delete ext;
}

void KonfiguratorURLRequester::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setUrl( krConfig->readEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorURLRequester::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, url() );
}

void KonfiguratorURLRequester::slotSetDefaults(QObject *)
{
  if( url() != defaultValue )
    setUrl( defaultValue );
}

// KonfiguratorFontChooser class
///////////////////////////////

KonfiguratorFontChooser::KonfiguratorFontChooser( QString cls, QString name, QFont *dflt,
  QWidget *parent, bool rst, int pg ) : Q3HBox ( parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  pLabel = new QLabel( this );
  pLabel->setMinimumWidth( 150 );
  pToolButton = new QToolButton( this );

  connect( pToolButton, SIGNAL( clicked() ), this, SLOT( slotBrowseFont() ) );

  pToolButton->setIconSet( SmallIcon( "fileopen" ) );

  loadInitialValue();
}

KonfiguratorFontChooser::~KonfiguratorFontChooser()
{
  delete ext;
}

void KonfiguratorFontChooser::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  font = krConfig->readEntry( ext->getCfgName(), *defaultValue );
  ext->setChanged( false );
  setFont();
}

void KonfiguratorFontChooser::setFont()
{
  pLabel->setFont( font );
  pLabel->setText( font.family()+QString(", %1").arg(font.pointSize()) );
}

void KonfiguratorFontChooser::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, font );
}

void KonfiguratorFontChooser::slotSetDefaults(QObject *)
{
  font = *defaultValue;
  ext->setChanged();
  setFont();
}

void KonfiguratorFontChooser::slotBrowseFont()
{
  int ok=KFontDialog::getFont( font );
  if (ok!=1) return;  // cancelled by the user
  ext->setChanged();
  setFont();
}

// KonfiguratorComboBox class
///////////////////////////////

KonfiguratorComboBox::KonfiguratorComboBox( QString cls, QString name, QString dflt,
    KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen, QWidget *parent,
    bool rst, bool editable, int pg ) : QComboBox ( parent ),
    defaultValue( dflt ), listLen( listInLen )
{
  list = new KONFIGURATOR_NAME_VALUE_PAIR[ listInLen ];

  for( int i=0; i != listLen; i++ )
  {
    list[i] = listIn[i];
    insertItem( list[i].text );
  }

  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

//  connect( this, SIGNAL( highlighted(int) ), ext, SLOT( setChanged() ) ); /* Removed because of startup combo failure */
  connect( this, SIGNAL( activated(int) ), ext, SLOT( setChanged() ) );
  connect( this, SIGNAL( textChanged ( const QString & ) ), ext, SLOT( setChanged() ) );

  setEditable( editable );
  loadInitialValue();
}

KonfiguratorComboBox::~KonfiguratorComboBox()
{
  delete []list;
  delete ext;
}

void KonfiguratorComboBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  QString select = krConfig->readEntry( ext->getCfgName(), defaultValue );
  selectEntry( select );
  ext->setChanged( false );
}

void KonfiguratorComboBox::slotApply(QObject *,QString cls, QString name)
{
  QString text = editable() ? lineEdit()->text() : currentText();
  QString value = text;
  
  for( int i=0; i != listLen; i++ )
    if( list[i].text == text ) {
      value = list[i].value;
      break;
    }
  
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value );
}

void KonfiguratorComboBox::selectEntry( QString entry )
{
  for( int i=0; i != listLen; i++ )
    if( list[i].value == entry )
    {
      setCurrentItem( i );
      return;
    }

  if( editable() )
    lineEdit()->setText( entry );
  else
    setCurrentItem( 0 );
}

void KonfiguratorComboBox::slotSetDefaults(QObject *)
{
  selectEntry( defaultValue );
}


// KonfiguratorColorChooser class
///////////////////////////////

KonfiguratorColorChooser::KonfiguratorColorChooser( QString cls, QString name, QColor dflt,
    QWidget *parent, bool rst, ADDITIONAL_COLOR *addColPtr,
    int addColNum, int pg ) : QComboBox ( parent ),
    defaultValue( dflt ), disableColorChooser( true )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );

  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  addColor( i18n("Custom color" ),  QColor( 255, 255, 255 ) );
  addColor( i18n("Default" ),       defaultValue );

  for( int i=0; i != addColNum; i++ )
  {
    additionalColors.push_back( addColPtr[i] );
    addColor( addColPtr[i].name, addColPtr[i].color );
  }

  addColor( i18n("Red" ),           Qt::red );
  addColor( i18n("Green" ),         Qt::green );
  addColor( i18n("Blue" ),          Qt::blue );
  addColor( i18n("Cyan" ),          Qt::cyan );
  addColor( i18n("Magenta" ),       Qt::magenta );
  addColor( i18n("Yellow" ),        Qt::yellow );
  addColor( i18n("Dark Red" ),      Qt::darkRed );
  addColor( i18n("Dark Green" ),    Qt::darkGreen );
  addColor( i18n("Dark Blue" ),     Qt::darkBlue );
  addColor( i18n("Dark Cyan" ),     Qt::darkCyan );
  addColor( i18n("Dark Magenta" ),  Qt::darkMagenta );
  addColor( i18n("Dark Yellow" ),   Qt::darkYellow );
  addColor( i18n("White" ),         Qt::white );
  addColor( i18n("Light Gray" ),    Qt::lightGray );
  addColor( i18n("Gray" ),          Qt::gray );
  addColor( i18n("Dark Gray" ),     Qt::darkGray );
  addColor( i18n("Black" ),         Qt::black );

  connect( this, SIGNAL( activated(int) ),   this, SLOT( slotCurrentChanged( int ) ) );

  loadInitialValue();
}

KonfiguratorColorChooser::~KonfiguratorColorChooser()
{
  delete ext;
}

QPixmap KonfiguratorColorChooser::createPixmap( QColor color )
{
  QPainter painter;
  QPen pen;
  int size = QFontMetrics(font()).height()*3/4;
  QRect rect( 0, 0, size, size );
  QPixmap pixmap( rect.width(), rect.height() );

  pen.setColor( Qt::black );

  painter.begin( &pixmap );
  QBrush brush( color );
  painter.fillRect( rect, brush );
  painter.setPen( pen );
  painter.drawRect( rect );
  painter.end();

  pixmap.detach();
  return pixmap;
}

void KonfiguratorColorChooser::addColor( QString text, QColor color )
{
  insertItem( createPixmap(color), text );
  palette.push_back( color );
}

void KonfiguratorColorChooser::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  QString selected = krConfig->readEntry( ext->getCfgName(), QString( "" ) );
  setValue( selected );
  ext->setChanged( false );
}

void KonfiguratorColorChooser::setDefaultColor( QColor dflt )
{
  defaultValue = dflt;
  palette[1] = defaultValue;
  changeItem( createPixmap( defaultValue ), text( 1 ), 1 );

  if( currentItem() == 1 )
    emit colorChanged();
}

void KonfiguratorColorChooser::changeAdditionalColor( unsigned int num, QColor color )
{
  if( num < additionalColors.size() )
  {
    palette[2+num] = color;
    additionalColors[num].color = color;
    changeItem( createPixmap( color ), text( 2+num ), 2+num );

    if( (unsigned int)currentItem() == 2+num )
      emit colorChanged();
  }
}

void KonfiguratorColorChooser::setDefaultText( QString text )
{
  changeItem( createPixmap( defaultValue ), text, 1 );
}

void KonfiguratorColorChooser::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, getValue() );
}

void KonfiguratorColorChooser::setValue( QString value )
{
  disableColorChooser = true;

  if( value.isEmpty() )
  {
    setCurrentItem( 1 );
    customValue = defaultValue;
  }
  else
  {
    bool found = false;

    for( unsigned j=0; j != additionalColors.size(); j++ )
      if( additionalColors[j].value == value )
      {
        setCurrentItem( 2 + j );
        found = true;
        break;
      }

    if( ! found )
    {
      krConfig->setGroup( ext->getCfgClass() );
      krConfig->writeEntry( "TmpColor", value );
      QColor color = krConfig->readEntry( "TmpColor", defaultValue );
      customValue = color;
      krConfig->deleteEntry( "TmpColor" );

      setCurrentItem( 0 );
      for( unsigned i= 2+additionalColors.size(); i != palette.size(); i++ )
        if( palette[i] == color )
        {
          setCurrentItem( i );
          break;
        }
    }
  }

  palette[0] = customValue;
  changeItem( createPixmap( customValue ), text( 0 ), 0 );

  ext->setChanged();
  emit colorChanged();
  disableColorChooser = false;
}

QString KonfiguratorColorChooser::getValue()
{
  QColor color = palette[ currentItem() ];
  if( currentItem() == 1 )    /* it's the default value? */
    return "";
  else if( currentItem() >= 2 && (unsigned)currentItem() < 2 + additionalColors.size() )
    return additionalColors[ currentItem() - 2 ].value;
  else
    return QString( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

bool KonfiguratorColorChooser::isValueRGB()
{
  return !( currentItem() >= 1 && (unsigned)currentItem() < 2 + additionalColors.size() );
}

void KonfiguratorColorChooser::slotSetDefaults(QObject *)
{
  ext->setChanged();
  setCurrentItem( 1 );
  emit colorChanged();
}

void KonfiguratorColorChooser::slotCurrentChanged( int number )
{
  ext->setChanged();
  if( number == 0 && !disableColorChooser )
  {
    QColor color = QColorDialog::getColor ( customValue, this, "ColorDialog" );
    if( color.isValid() )
    {
      disableColorChooser = true;
      customValue = color;
      palette[0] = customValue;
      changeItem( createPixmap( customValue ), text( 0 ), 0 );
      disableColorChooser = false;
    }
  }

  emit colorChanged();
}

QColor KonfiguratorColorChooser::getColor()
{
  return palette[ currentItem() ];
}

// KonfiguratorListBox class
///////////////////////////////

KonfiguratorListBox::KonfiguratorListBox( QString cls, QString name, QStringList dflt,
    QWidget *parent, bool rst, int pg ) : Q3ListBox( parent ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst, pg );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  loadInitialValue();
}

KonfiguratorListBox::~KonfiguratorListBox()
{
  delete ext;
}

void KonfiguratorListBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setList( krConfig->readListEntry( ext->getCfgName().ascii(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorListBox::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, list() );
}

void KonfiguratorListBox::slotSetDefaults(QObject *)
{
  if( list() != defaultValue )
  {
    ext->setChanged();
    setList( defaultValue );
  }
}

void KonfiguratorListBox::setList( QStringList list )
{
  clear();
  insertStringList( list );
}

QStringList KonfiguratorListBox::list()
{
  QStringList lst;
  
  for( unsigned i=0; i != count(); i++ )
    lst += text( i );

  return lst;
}

void KonfiguratorListBox::addItem( const QString & item )
{
  if( !list().contains( item ) )
  {
    insertItem( item );
    ext->setChanged();
  }
}

void KonfiguratorListBox::removeItem( const QString & item )
{
  Q3ListBoxItem * listItem = findItem( item );
  if( listItem != 0 )
  {
    takeItem( listItem );
    ext->setChanged();
  }
}

#include "konfiguratoritems.moc"
