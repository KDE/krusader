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
 
KonfiguratorExtension::KonfiguratorExtension( QObject *obj, QString cfgClass, QString cfgName, bool rst) :
      QObject(), objectPtr( obj ), applyConnected( false ), setDefaultsConnected( false ),
      changed( false ), restartNeeded( rst ), configClass( cfgClass ), configName( cfgName )
{
}

void KonfiguratorExtension::connectNotify( const char *signal )
{
  if( strcmp( signal, SIGNAL( applyManually(QObject *,QString, QString) ) ) == 0 )
    applyConnected = true;
  else if ( strcmp( signal, SIGNAL( setDefaultsManually(QObject *) ) ) == 0 )
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
 
  connect( this, SIGNAL( toggled(bool) ), ext, SLOT( setChanged() ) );
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

// KonfiguratorRadioButtons class
///////////////////////////////

KonfiguratorRadioButtons::KonfiguratorRadioButtons( QString cls, QString name, QString dflt,
    QButtonGroup *grp, bool rst ) : KonfiguratorExtension( this, cls, name, rst ),
    buttonGroup( grp )
{
  defaultValue = dflt;
}

void KonfiguratorRadioButtons::addRadioButton( QRadioButton *radioWidget, QString value )
{
  radioButtons.append( radioWidget );
  radioValues.push_back( value );

  connect( radioWidget, SIGNAL( toggled(bool) ), this, SLOT( setChanged() ) );
}

void KonfiguratorRadioButtons::selectButton( QString value )
{
  int cnt = 0;
  QRadioButton *btn  = radioButtons.first();
  if( btn )
    btn->setChecked( true );

  while( btn )
  {
    if( value == radioValues[ cnt ] )
    {
      btn->setChecked( true );
      break;
    }

    btn = radioButtons.next();
    cnt++;
  }
}

void KonfiguratorRadioButtons::loadInitialValue()
{
  krConfig->setGroup( configClass );
  QString initValue = krConfig->readEntry( configName, defaultValue );

  selectButton( initValue );
  setChanged( false );
}

bool KonfiguratorRadioButtons::apply()
{
  if( !changed )
    return false;

  if( applyConnected )
  {
    emit applyManually( objectPtr, configClass, configName );
    return restartNeeded;
  }
  
  QRadioButton *btn  = radioButtons.first();
  int cnt = 0;

  while( btn )
  {
    if( btn->isChecked() )
    {
      krConfig->setGroup( configClass );
      krConfig->writeEntry( configName, radioValues[ cnt ] );
      break;
    }

    btn = radioButtons.next();
    cnt++;
  }
  setChanged( false );
  
  return restartNeeded;
}

void KonfiguratorRadioButtons::setDefaults()
{
  if( setDefaultsConnected )
  {
    emit setDefaultsManually( objectPtr );
    return;
  }
  
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
    const char *widgetName, bool rst ) : QComboBox ( parent, widgetName ),
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

  connect( this, SIGNAL( activated(int) ), ext, SLOT( setChanged() ) );
    
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
  selectEntry( currentText() );
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, list[selected].value );
}

void KonfiguratorComboBox::selectEntry( QString entry )
{
  selected = 0;

  for( int i=0; i != listLen; i++ )
    if( list[i].value == entry )
      selected = i;

  setCurrentItem( selected );
}

void KonfiguratorComboBox::slotSetDefaults(QObject *)
{
  selectEntry( defaultValue );
}

#include "konfiguratoritems.moc"
