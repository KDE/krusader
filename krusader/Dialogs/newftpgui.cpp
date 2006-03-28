/****************************************************************************
** Form implementation generated from reading ui file 'newftpgui.ui'
**
** Created: Fri Oct 27 23:47:10 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "newftpgui.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qgrid.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include "../krusader.h"


/* 
 *  Constructs a newFTPGUI which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
 
 #define SIZE_MINIMUM	QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0 )
 
newFTPGUI::newFTPGUI( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ){
    
    QVBoxLayout * layout = new QVBoxLayout( this, 11, 6, "newFTPGUI_layout" );
    layout->setAutoAdd(true);
    
    if ( !name )
    setName( "newFTPGUI" );
    resize( 342, 261 );
    setCaption( i18n( "New Network Connection"  ) );
//     setSizeGripEnabled( TRUE );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 342, 261 ) );

    
    QHBox* hbox_image = new QHBox( this, "hbox_image" );
    hbox_image->setSpacing( 6 );
    
    PixmapLabel1 = new QLabel( hbox_image, "PixmapLabel1" );
    PixmapLabel1->setPixmap( krLoader->loadIcon("network", KIcon::Desktop, 32) );
    PixmapLabel1->setSizePolicy( SIZE_MINIMUM );

    TextLabel3 = new QLabel( i18n( "About to connect to..."  ), hbox_image, "TextLabel3" );
    QFont TextLabel3_font(  TextLabel3->font() );
    TextLabel3_font.setBold( TRUE );
    TextLabel3->setFont( TextLabel3_font );

    
    QGrid* grid_host = new QGrid( 3, this, "grid_host" );
    
    TextLabel1 = new QLabel( i18n( "Protocol:"  ), grid_host, "TextLabel1" );
    TextLabel1_22 = new QLabel( i18n( "Host:"), grid_host, "TextLabel_2" );
    TextLabel1_3 = new QLabel( i18n( "Port:"  ), grid_host, "TextLabel1_3" );

    QStringList protocols = KProtocolInfo::protocols();

    prefix = new KComboBox( FALSE, grid_host, "protocol" );
    if( protocols.contains("ftp") )
      prefix->insertItem( i18n( "ftp://" ) );
    if( protocols.contains("smb") )
      prefix->insertItem( i18n( "smb://" ) );
    if( protocols.contains("fish") )
      prefix->insertItem( i18n( "fish://" ));
    if( protocols.contains("sftp") )
      prefix->insertItem( i18n( "sftp://" ));
    prefix->setAcceptDrops( FALSE );
    prefix->setEnabled( TRUE );
    prefix->setSizePolicy( SIZE_MINIMUM );
    connect( prefix,SIGNAL(activated(const QString& )),
               this,SLOT(slotTextChanged(const QString& )));

    url = new KHistoryCombo( grid_host, "url" );
    url->setMaximumHeight( 20 );
    url->setMaxCount( 25 );
    url->setDuplicatesEnabled( false );
    connect( url, SIGNAL( activated( const QString& )),
             url, SLOT( addToHistory( const QString& )));
    // load the history and completion list after creating the history combo
    krConfig->setGroup("Private");
    QStringList list = krConfig->readListEntry( "newFTP Completion list" );
    url->completionObject()->setItems( list );
    list = krConfig->readListEntry( "newFTP History list" );
    url->setHistoryItems( list );

    port = new QSpinBox( grid_host, "port" );
    port->setMaxValue( 65535 );
#if QT_VERSION < 300
    port->setFrameShadow( QSpinBox::Sunken );
#endif
    port->setValue( 21 );
    port->setSizePolicy( SIZE_MINIMUM );


    TextLabel1_2 = new QLabel( i18n( "Username:"  ), this, "TextLabel1_2" );
    username = new QLineEdit( this, "username" );
    TextLabel1_2_2 = new QLabel( i18n( "Password:"  ), this, "TextLabel1_2_2" );
    password = new QLineEdit( this, "password" );
    password->setEchoMode( QLineEdit::Password );

    
    QWidget* Layout6 = new QWidget( this, "Layout6" );
    hbox = new QHBoxLayout( Layout6 );
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

	 hbox->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding));
	 
    connectBtn = new QPushButton( i18n( "&Connect"  ), Layout6, "connectBtn" );
    connectBtn->setAutoDefault( TRUE );
    connectBtn->setDefault( TRUE );
    hbox->addWidget( connectBtn );

    //saveBtn = new QPushButton( i18n( "&Save"  ), Layout6, "saveBtn" );
    //saveBtn->setAutoDefault( TRUE );
    //hbox->addWidget( saveBtn );

    cancelBtn = new QPushButton( i18n( "&Cancel"  ), Layout6, "cancelBtn" );
    cancelBtn->setAutoDefault( TRUE );
    hbox->addWidget( cancelBtn );

    // signals and slots connections
    connect( connectBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // tab order
    setTabOrder( url, username );
    setTabOrder( username, password );
    setTabOrder( password, connectBtn );
    setTabOrder( connectBtn, cancelBtn );
    setTabOrder( cancelBtn, prefix );
    setTabOrder( prefix, url );
}

/*
 *  Destroys the object and frees any allocated resources
 */
newFTPGUI::~newFTPGUI(){
    // no need to delete child widgets, Qt does it all for us
}

void newFTPGUI::slotTextChanged(const QString& string){
   if( string.startsWith("ftp") || string.startsWith("sftp") || string.startsWith("fish") )
   {
     if( port->value() == 21 || port->value() == 22 )
       port->setValue( string.startsWith("ftp") ? 21 : 22 );
     port->setEnabled(true);
   }
   else
     port->setEnabled(false);
}

/*
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool newFTPGUI::event( QEvent* ev ) {
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel3_font(  TextLabel3->font() );
	TextLabel3_font.setBold( TRUE );
	TextLabel3->setFont( TextLabel3_font ); 
    }
    return ret;
}

#include "newftpgui.moc"
