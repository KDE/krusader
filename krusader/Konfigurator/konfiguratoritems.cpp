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
  
KonfiguratorExtension::KonfiguratorExtension( QObject *obj, QString cfgClass, QString cfgName, bool rst) :
      QObject(), objectPtr( obj ), applyConnected( false ), setDefaultsConnected( false ),
      changed( false ), restartNeeded( rst ), configClass( cfgClass ), configName( cfgName )
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
    QWidget *parent, const char *widgetName, bool rst ) : QCheckBox( text, parent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
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
    QWidget *parent, const char *widgetName, bool rst ) : QSpinBox( parent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
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
    QString dflt, QWidget *parent, const char *widgetName, bool rst ) :
    QButtonGroup( parent, widgetName ), defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );
}

KonfiguratorRadioButtons::~KonfiguratorRadioButtons()
{
  delete ext;
}

void KonfiguratorRadioButtons::addRadioButton( QRadioButton *radioWidget, QString value )
{
  radioButtons.append( radioWidget );
  radioValues.push_back( value );

  connect( radioWidget, SIGNAL( stateChanged(int) ), ext, SLOT( setChanged() ) );
}                                                       

QRadioButton * KonfiguratorRadioButtons::find( int index )
{
  return radioButtons.at( index );
}

QRadioButton * KonfiguratorRadioButtons::find( QString name )
{
  QRadioButton *radioButton = radioButtons.first();

  while( radioButton )
  {
    if( radioButton->text() == name )
      return radioButton;
    radioButton = radioButtons.next();
  }

  return 0;
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
    QWidget *parent, const char *widgetName, bool rst ) : QLineEdit( parent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
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

static const char* const image0_data[] = {
"16 16 53 1",
". c None",
"# c #000000",
"R c #080808",
"Q c #101010",
"Y c #181c18",
"K c #202020",
"w c #313031",
"X c #313431",
"W c #414041",
"V c #414441",
"g c #4a3018",
"E c #4a484a",
"U c #4a4c4a",
"l c #524c4a",
"h c #52504a",
"j c #525052",
"i c #5a5952",
"J c #6a6962",
"P c #6a6d6a",
"T c #8b8d8b",
"D c #949594",
"S c #9c999c",
"O c #9c9d9c",
"C c #a4a19c",
"M c #a4a1a4",
"N c #a4a5a4",
"I c #acaaac",
"B c #acaeac",
"F c #b47d41",
"v c #b4b2b4",
"u c #b4b6b4",
"A c #bdbabd",
"t c #bdbebd",
"k c #c5854a",
"H c #c5c2c5",
"s c #c5c6c5",
"r c #cdcacd",
"L c #cdcecd",
"z c #d5d2d5",
"q c #d5d6d5",
"x c #dedade",
"p c #dedede",
"a c #e6a562",
"G c #e6e2e6",
"o c #e6e6e6",
"y c #eeeae6",
"n c #eeeeee",
"m c #f6f6f6",
"e c #ffae62",
"d c #ffc283",
"c c #ffc683",
"f c #ffca8b",
"b c #ffd29c",
"......####......",
".....##.####.#..",
"....#.....####..",
"...........###..",
"..........####..",
".####...........",
"#abca#######....",
"#bcdeeeeeeee#...",
"#fdghhhihhjjh###",
"#dklmmnopqrstuvw",
"#dgxpyopzsABCDE.",
"#FhxxGpqrHuICJK.",
"#gLrqqzLHABMDE..",
"#jLtssHtuBNOPQ..",
"RtNIBBINMSDTE...",
"KEEUUUEEVWWXY..."};

KonfiguratorURLRequester::KonfiguratorURLRequester( QString cls, QString name, QString dflt,
    QWidget *parent, const char *widgetName, bool rst ) : KURLRequester( parent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( textChanged(const QString &) ), ext, SLOT( setChanged() ) );

  QPixmap pixMap( (const char **)image0_data );
  button()->setIconSet( QIconSet() );
  button()->setPixmap( pixMap );
  loadInitialValue();
}

KonfiguratorURLRequester::~KonfiguratorURLRequester()
{
  delete ext;
}

void KonfiguratorURLRequester::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setURL( krConfig->readEntry( ext->getCfgName(), defaultValue ) );
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
    setURL( defaultValue );
}

// KonfiguratorFontChooser class
///////////////////////////////

KonfiguratorFontChooser::KonfiguratorFontChooser( QString cls, QString name, QFont *dflt,
  QWidget *parent, const char *widgetName, bool rst ) : QHBox ( parent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  pLabel = new QLabel( this );
  pLabel->setMinimumWidth( 150 );
  pToolButton = new QToolButton( this );
  
  connect( pToolButton, SIGNAL( clicked() ), this, SLOT( slotBrowseFont() ) );

  QPixmap pixMap( (const char **)image0_data );
  pToolButton->setText( "" );
  pToolButton->setPixmap( pixMap );
  
  loadInitialValue();
}

KonfiguratorFontChooser::~KonfiguratorFontChooser()
{
  delete ext;
}

void KonfiguratorFontChooser::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  font = krConfig->readFontEntry( ext->getCfgName(), defaultValue );
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
    const char *widgetName, bool rst, bool editable ) : QComboBox ( parent, widgetName ),
    defaultValue( dflt ), listLen( listInLen )
{
  list = new KONFIGURATOR_NAME_VALUE_PAIR[ listInLen ];
  
  for( int i=0; i != listLen; i++ )
  {
    list[i] = listIn[i];
    insertItem( list[i].text );
  }
    
  ext = new KonfiguratorExtension( this, cls, name, rst );
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  connect( this, SIGNAL( highlighted(int) ), ext, SLOT( setChanged() ) );
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
  QString value = editable() ? lineEdit()->text() : currentText();
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
    QWidget *parent, const char *widgetName, bool rst ) : QComboBox ( parent, widgetName ),
    defaultValue( dflt ), disableColorChooser( true )
{
  ext = new KonfiguratorExtension( this, cls, name, rst );
  
  connect( ext, SIGNAL( applyAuto(QObject *,QString, QString) ), this, SLOT( slotApply(QObject *,QString, QString) ) );
  connect( ext, SIGNAL( setDefaultsAuto(QObject *) ), this, SLOT( slotSetDefaults(QObject *) ) );
  connect( ext, SIGNAL( setInitialValue(QObject *) ), this, SLOT( loadInitialValue() ) );

  addColor( i18n("Custom color" ),  QColor( 255, 255, 255 ) );
  addColor( i18n("KDE default" ),   defaultValue );
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

  connect( this, SIGNAL( highlighted(int) ), ext,  SLOT( setChanged() ) );
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
  disableColorChooser = true;
  
  krConfig->setGroup( ext->getCfgClass() );
  QString selected = krConfig->readEntry( ext->getCfgName(), "" );
  if( selected.isEmpty() )
  {
    setCurrentItem( 1 );
    customValue = defaultValue;
  }
  else
  {
    QColor color = krConfig->readColorEntry( ext->getCfgName(), &defaultValue );
    customValue = color;

    setCurrentItem( 0 );
    for( int i=2; i != palette.size(); i++ )
      if( palette[i] == color )
      {
        setCurrentItem( i );
        break;
      }
  }

  palette[0] = customValue;
  changeItem( createPixmap( customValue ), text( 0 ), 0 );
      
  ext->setChanged( false );
  disableColorChooser = false;
}

void KonfiguratorColorChooser::setDefaultColor( QColor dflt )
{
  defaultValue = dflt;
  palette[1] = defaultValue;
  changeItem( createPixmap( defaultValue ), text( 1 ), 1 );
}

void KonfiguratorColorChooser::slotApply(QObject *,QString cls, QString name)
{
  krConfig->setGroup( cls );
  
  QColor color = palette[ currentItem() ];
  if( currentItem() == 1 )    /* it's the default value? */
    krConfig->writeEntry( name, "" );   /* set nothing */
  else
    krConfig->writeEntry( name, color );
}

void KonfiguratorColorChooser::slotSetDefaults(QObject *)
{
  setCurrentItem( 1 );
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
}

#include "konfiguratoritems.moc"
