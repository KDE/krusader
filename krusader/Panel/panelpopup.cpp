#include "panelpopup.h"
#include "listpanel.h"

#include <kdebug.h>

#include <kfiletreeview.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qheader.h>
#include <krview.h>
#include <krviewitem.h>
#include <qwidgetstack.h>
#include <kmultitabbar.h>
#include "../kicons.h"


PanelPopup::PanelPopup( QWidget *parent ) : QWidget( parent ), stack( 0 ), viewer( 0 ), pjob( 0 ) {
   QVBoxLayout * layout = new QVBoxLayout( this );

   // create the button bar
   tabbar = new KMultiTabBar( KMultiTabBar::Horizontal, this );
   tabbar->appendTab( krLoader->loadIcon( "view_tree", KIcon::Panel ), Tree, "Tree" );
   connect( tabbar->tab( Tree ), SIGNAL( clicked( int ) ), this, SLOT( tabSelected( int ) ) );
   tabbar->appendTab( krLoader->loadIcon( "folder_image", KIcon::Panel ), Preview, "Preview" );
   connect( tabbar->tab( Preview ), SIGNAL( clicked( int ) ), this, SLOT( tabSelected( int ) ) );

   // create a widget stack on which to put the parts
   stack = new QWidgetStack( this );

   // create the tree part ----------
   tree = new KFileTreeView( stack );
   stack->addWidget( tree, Tree );
   tree->addColumn( "" );
   tree->addBranch( "/", i18n( "Root Folder" ) );
   tree->setDirOnlyMode( tree->branch( i18n( "Root Folder" ) ), true );
	tree->branch( i18n( "Root Folder" ) ) ->setChildRecurse(false);
	
   tree->branch( i18n( "Root Folder" ) ) ->setOpen( true );
   tree->header() ->setHidden( true );
	connect(tree, SIGNAL(executed(QListViewItem*)), this, SLOT(treeSelection(QListViewItem*)));
   // start listing the tree
   tree->branch( i18n( "Root Folder" ) ) ->root();

   // create the quickview part ------
   viewer = new QLabel( i18n( "No preview available" ), stack );
   stack->addWidget( viewer, Preview );

   layout->addWidget( tabbar );
   layout->addWidget( stack );

   // raise the tree part
   tabbar->setTab( Tree, true );
   stack->raiseWidget( Tree );
}

PanelPopup::~PanelPopup() {}

void PanelPopup::tabSelected( int id ) {
	// unraise all tabs except selected one and raise the widget
   for ( int i = Tree; i < Last; ++i )
      tabbar->setTab( i, ( i == id ) );
   stack->raiseWidget( id );
}

// decide which part to update, if at all
void PanelPopup::update( KURL url ) {
	if ( isHidden() ) return ; // failsafe
	
	KFileItem *kfi;
	KFileItemList lst;
	
   switch ( stack->id( stack->visibleWidget() ) ) {
         case Preview:
         if ( url.isEmpty() ) {
            dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setText( i18n( "No preview available" ) );
            return ; // in case of a bad url
         }

         kfi = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url );
         lst.append( kfi );
         if ( pjob ) // stop running jobs
            delete pjob;
         pjob = new KIO::PreviewJob( lst, stack->width(), stack->height(), stack->width(), 1, true, true, 0 );
         connect( pjob, SIGNAL( gotPreview( const KFileItem*, const QPixmap& ) ),
                  this, SLOT( view( const KFileItem*, const QPixmap& ) ) );
         connect( pjob, SIGNAL( failed( const KFileItem* ) ),
                  this, SLOT( failedToView( const KFileItem* ) ) );
         break;
			
         case Tree: // nothing to do
			break;
   }
}

// called when the preview job got something for us
void PanelPopup::view( const KFileItem* kfi, const QPixmap& pix ) {
   dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setPixmap( pix );
}

// preview job failed here...
void PanelPopup::failedToView( const KFileItem* kfi ) {
   dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setText( i18n( "No preview available" ) );
}

void PanelPopup::treeSelection(QListViewItem*) {
	emit selection(tree->currentURL());
	emit hideMe();
}
