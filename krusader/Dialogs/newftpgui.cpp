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
#include <qvariant.h>
#include <qimage.h>
#include <qpixmap.h>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <khistorycombobox.h>
#include <kconfiggroup.h>
#include "../krusader.h"


/* 
 *  Constructs a newFTPGUI which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
 
 #define SIZE_MINIMUM	QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed )
 
newFTPGUI::newFTPGUI( QWidget* parent )
    : QDialog( parent ){
    
    setModal( true );
    QVBoxLayout * layout = new QVBoxLayout( this );
    layout->setContentsMargins( 11, 11, 11, 11 );
    layout->setSpacing( 6 );
    
    resize( 342, 261 );
    setWindowTitle( i18n( "New Network Connection"  ) );
//     setSizeGripEnabled( TRUE );
    QSizePolicy policy( QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth( sizePolicy().hasHeightForWidth() );
    setSizePolicy( policy );
    //setMinimumSize( QSize( 342, 261 ) );

    QWidget * hbox_image_widget = new QWidget( this );
    QHBoxLayout* hbox_image = new QHBoxLayout( hbox_image_widget );
    hbox_image->setSpacing( 6 );
    
    PixmapLabel1 = new QLabel( hbox_image_widget );
    PixmapLabel1->setWindowIcon( krLoader->loadIcon("network", KIconLoader::Desktop, 32) );
    PixmapLabel1->setSizePolicy( SIZE_MINIMUM );
    hbox_image->addWidget( PixmapLabel1 );

    TextLabel3 = new QLabel( i18n( "About to connect to..."  ), hbox_image_widget );
    QFont TextLabel3_font(  TextLabel3->font() );
    TextLabel3_font.setBold( TRUE );
    TextLabel3->setFont( TextLabel3_font );
    hbox_image->addWidget( TextLabel3 );
    layout->addWidget( hbox_image_widget );

    
    QWidget * grid_host = new QWidget( this );
    QGridLayout * grid_layout = new QGridLayout( grid_host );
    
    grid_layout->addWidget( TextLabel1 = new QLabel( i18n( "Protocol:"  ), grid_host ), 0, 0 );
    grid_layout->addWidget( TextLabel1_22 = new QLabel( i18n( "Host:"), grid_host ), 0, 1 );
    grid_layout->addWidget( TextLabel1_3 = new QLabel( i18n( "Port:"  ), grid_host ), 0, 2 );

    QStringList protocols = KProtocolInfo::protocols();

    prefix = new KComboBox( FALSE, grid_host );
    grid_layout->addWidget( prefix, 1, 0 );
    prefix->setObjectName( "protocol" );
    if( protocols.contains("ftp") )
      prefix->addItem( i18n( "ftp://" ) );
    if( protocols.contains("smb") )
      prefix->addItem( i18n( "smb://" ) );
    if( protocols.contains("fish") )
      prefix->addItem( i18n( "fish://" ));
    if( protocols.contains("sftp") )
      prefix->addItem( i18n( "sftp://" ));
    prefix->setAcceptDrops( FALSE );
    prefix->setEnabled( TRUE );
    prefix->setSizePolicy( SIZE_MINIMUM );
    connect( prefix,SIGNAL(activated(const QString& )),
               this,SLOT(slotTextChanged(const QString& )));

    url = new KHistoryComboBox( grid_host );
    grid_layout->addWidget( url );
    //url->setMaximumHeight( 20 );
    url->setMaxCount( 25 );
    url->setDuplicatesEnabled( false );
    connect( url, SIGNAL( activated( const QString& )),
             url, SLOT( addToHistory( const QString& )));
    // load the history and completion list after creating the history combo
    KConfigGroup group( krConfig, "Private");
    QStringList list = group.readEntry( "newFTP Completion list", QStringList() );
    url->completionObject()->setItems( list );
    list = group.readEntry( "newFTP History list", QStringList() );
    url->setHistoryItems( list );

    port = new QSpinBox( grid_host );
    port->setMaximum( 65535 );
    grid_layout->addWidget( port, 1, 2 );
#if QT_VERSION < 300
    port->setFrameShadow( QSpinBox::Sunken );
#endif
    port->setValue( 21 );
    port->setSizePolicy( SIZE_MINIMUM );

    layout->addWidget( grid_host );

    TextLabel1_2 = new QLabel( i18n( "Username:"  ), this );
    layout->addWidget( TextLabel1_2 );
    username = new QLineEdit( this );
    layout->addWidget( username );
    TextLabel1_2_2 = new QLabel( i18n( "Password:"  ), this );
    layout->addWidget( TextLabel1_2_2 );
    password = new QLineEdit( this );
    password->setEchoMode( QLineEdit::Password );
    layout->addWidget( password );

    
    QWidget* Layout6 = new QWidget( this );
    hbox = new QHBoxLayout( Layout6 );
    hbox->setSpacing( 6 );
    hbox->setContentsMargins( 0, 0, 0, 0 );

	 hbox->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding));
	 
    connectBtn = new QPushButton( i18n( "&Connect"  ), Layout6 );
    connectBtn->setAutoDefault( TRUE );
    connectBtn->setDefault( TRUE );
    hbox->addWidget( connectBtn );

    //saveBtn = new QPushButton( i18n( "&Save"  ), Layout6 );
    //saveBtn->setAutoDefault( TRUE );
    //hbox->addWidget( saveBtn );

    cancelBtn = new QPushButton( i18n( "&Cancel"  ), Layout6 );
    cancelBtn->setAutoDefault( TRUE );
    hbox->addWidget( cancelBtn );

    layout->addWidget( Layout6 );

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
