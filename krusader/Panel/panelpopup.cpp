#include "../krusader.h"
#include "panelpopup.h"
#include "../kicons.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../defaults.h"
#include "../krslots.h"
#include "panelfunc.h"
#include <qbuttongroup.h>
#include <qtoolbutton.h>
#include <kfiletreeview.h>
#include <klocale.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qheader.h>
#include <krview.h>
#include <krviewitem.h>
#include <klineedit.h>
#include <kio/jobclasses.h>

#include <kdebug.h>

PanelPopup::PanelPopup( QWidget *parent, bool left ) : QWidget( parent ), 
	stack( 0 ), viewer( 0 ), pjob( 0 ) {
   QGridLayout * layout = new QGridLayout(this, 1, 1);
	
	// create the label+buttons setup
	dataLine = new KrSqueezedTextLabel(this);
	dataLine->setText("blah blah");
	krConfig->setGroup( "Look&Feel" );
   dataLine->setFont( krConfig->readFontEntry( "Filelist Font", _FilelistFont ) );
   // --- hack: setup colors to be the same as an inactive panel
	dataLine->setBackgroundMode( PaletteBackground );
	QPalette q( dataLine->palette() );
   q.setColor( QColorGroup::Foreground, KGlobalSettings::textColor() );
   q.setColor( QColorGroup::Background, KGlobalSettings::baseColor() );
   dataLine->setPalette( q );
   dataLine->setFrameStyle( QFrame::Box | QFrame::Raised );
   dataLine->setLineWidth( 1 );		// a nice 3D touch :-)
   int sheight = QFontMetrics( dataLine->font() ).height() + 4;
   dataLine->setMaximumHeight( sheight );
	
	btns = new QButtonGroup(this);
	btns->setExclusive(true);
	btns->hide();	// it should be invisible
	connect(btns, SIGNAL(clicked(int)), this, SLOT(tabSelected(int)));
	
	treeBtn = new QToolButton(this);
	treeBtn->setPixmap(krLoader->loadIcon( "view_tree", KIcon::Toolbar, 16 ));
	treeBtn->setFixedSize(20, 20);
	treeBtn->setToggleButton(true);
	btns->insert(treeBtn, Tree);
	
	
	previewBtn = new QToolButton(this);
	previewBtn->setPixmap(krLoader->loadIcon( "folder_image", KIcon::Toolbar, 16 ));
	previewBtn->setFixedSize(20, 20);
	previewBtn->setToggleButton(true);
	btns->insert(previewBtn, Preview);
	
	quickBtn = new QToolButton(this);
	quickBtn->setPixmap(krLoader->loadIcon( "kr_select", KIcon::Toolbar, 16 ));
	quickBtn->setFixedSize(20, 20);
	quickBtn->setToggleButton(true);
	btns->insert(quickBtn, QuickPanel);
	
	layout->addWidget(dataLine,0,0);
	layout->addWidget(treeBtn,0,1);
	layout->addWidget(previewBtn,0,2);
	layout->addWidget(quickBtn,0,3);
	
	// create a widget stack on which to put the parts
   stack = new QWidgetStack( this );

   // create the tree part ----------
	tree = new KFileTreeView( stack );
	tree->setAcceptDrops(true);
	connect(tree, SIGNAL(dropped (QWidget *, QDropEvent *, KURL::List &, KURL &)), 
		this, SLOT(slotDroppedOnTree(QWidget *, QDropEvent *, KURL::List&, KURL& )));
   stack->addWidget( tree, Tree );
   tree->addColumn( "" );
	// add ~
	tree->addBranch( QDir::home().absPath(), i18n("Home"));
	tree->setDirOnlyMode( tree->branch(i18n("Home")), true);
	tree->branch(i18n("Home"))->setChildRecurse(false);
	// add /
	tree->addBranch( "/", i18n( "Root" ) );
   tree->setDirOnlyMode( tree->branch( i18n( "Root" ) ), true );
	tree->setShowFolderOpenPixmap(true);
	tree->branch( i18n( "Root" ) ) ->setChildRecurse(false);
	tree->branch( i18n( "Root" ) ) ->setOpen( true );
   tree->header() ->setHidden( true );
	connect(tree, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(treeSelection(QListViewItem*)));
   // start listing the tree
   tree->branch( i18n( "Root" ) ) ->root();
	tree->branch( i18n( "Home" ) ) ->root();

   // create the quickview part ------
   viewer = new QLabel( i18n( "No preview available" ), stack );
   stack->addWidget( viewer, Preview );

	// create the quick-panel part ----
	
	QWidget *quickPanel = new QWidget(stack);
	QGridLayout *qlayout = new QGridLayout(quickPanel);	
	// --- quick select
	QLabel *selectLabel = new QLabel(i18n("Quick Select"), quickPanel);
	quickSelectCombo = new KComboBox( quickPanel );
	quickSelectCombo->setEditable( true );
	krConfig->setGroup( "Private" );
	QStrList lst;
	int i = krConfig->readListEntry( "Predefined Selections", lst );
	if ( i > 0 )
		quickSelectCombo->insertStrList( lst );
	quickSelectCombo->setCurrentText( "*" );
	quickSelectCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred ) );

	connect(quickSelectCombo, SIGNAL(returnPressed(const QString& )),
		this, SLOT(quickSelect(const QString& )));
	
	QToolButton *qselectBtn = new QToolButton(quickPanel);
	qselectBtn->setPixmap(krLoader->loadIcon( "kr_selectall", KIcon::Toolbar, 16 ));
	qselectBtn->setFixedSize(20, 20);
	QToolTip::add( qselectBtn, i18n("apply the selection") );
	connect(qselectBtn, SIGNAL(clicked()), this, SLOT(quickSelect()));

	QToolButton *qstoreBtn = new QToolButton(quickPanel);
	qstoreBtn->setPixmap(krLoader->loadIcon( "filesave", KIcon::Toolbar, 16 ));
	qstoreBtn->setFixedSize(20, 20);
	QToolTip::add( qstoreBtn, i18n("store the current selection") );
	connect(qstoreBtn, SIGNAL(clicked()), this, SLOT(quickSelectStore()));
	
	QToolButton *qsettingsBtn = new QToolButton(quickPanel);
	qsettingsBtn->setPixmap(krLoader->loadIcon( "configure", KIcon::Toolbar, 16 ));
	qsettingsBtn->setFixedSize(20, 20);
	QToolTip::add( qsettingsBtn, i18n("select group dialog") );
	connect(qsettingsBtn, SIGNAL(clicked()), krSelect, SLOT(activate()));

	qlayout->addWidget(selectLabel,0,0);
	qlayout->addWidget(quickSelectCombo,0,1);
	qlayout->addWidget(qselectBtn,0,2);
	qlayout->addWidget(qstoreBtn,0,3);
	qlayout->addWidget(qsettingsBtn,0,4);
	stack->addWidget(quickPanel, QuickPanel);
	
	// -------- finish the layout (General one)
	layout->addMultiCellWidget(stack,1,1,0,3);
	
   // set the wanted widget
	// ugly: are we left or right?
	int id;
	krConfig->setGroup("Startup");
	if (left) {
		id = krConfig->readNumEntry("Left Panel Popup", _LeftPanelPopup);
	} else {
		id = krConfig->readNumEntry("Right Panel Popup", _RightPanelPopup);	
	}
	btns->setButton(id);
	tabSelected(id);
}

PanelPopup::~PanelPopup() {}

void PanelPopup::tabSelected( int id ) {
   stack->raiseWidget( id );
	// if tab is tree, set something logical in the data line
	switch (id) {
		case Tree:
			dataLine->setText("Tree:");
			break;
		case Preview:
			dataLine->setText("Preview:");
			break;
		case QuickPanel:
			dataLine->setText("Quick Select:");
			break;
	}
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
			dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setText( i18n( "Please wait..." ) );
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

// ------------------ preview

// called when the preview job got something for us
void PanelPopup::view( const KFileItem *kfi, const QPixmap& pix ) {
   dataLine->setText(i18n("Preview: ") + kfi->name());
	dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setPixmap( pix );
}

// preview job failed here...
void PanelPopup::failedToView( const KFileItem* ) {
	dataLine->setText("Preview:");
   dynamic_cast<QLabel*>( stack->widget( Preview ) ) ->setText( i18n( "No preview available" ) );
}

// ------------------- tree

void PanelPopup::treeSelection(QListViewItem*) {
	emit selection(tree->currentURL());
	//emit hideMe();
}

// ------------------- quick panel

void PanelPopup::quickSelect() {
	SLOTS->markGroup(quickSelectCombo->currentText(), true);
}

void PanelPopup::quickSelect(const QString &mask) {
	SLOTS->markGroup(mask, true);
}

void PanelPopup::quickSelectStore() {
        krConfig->setGroup( "Private" );
        QStringList lst = krConfig->readListEntry( "Predefined Selections" );
        if ( lst.find(quickSelectCombo->currentText()) == lst.end() )
           lst.append( quickSelectCombo->currentText() );
        krConfig->writeEntry( "Predefined Selections", lst );
}

void PanelPopup::slotDroppedOnTree(QWidget *widget, QDropEvent *e, KURL::List &lst, KURL &) {
	// KFileTreeView is buggy: when dropped, it might not give us the correct
	// destination, but actually, it's parent. workaround: don't use
	// the destination in the signal, but take the current item
	KURL dest = tree->currentURL();
	
	// ask the user what to do: copy, move or link?
   QPopupMenu popup( this );
   popup.insertItem( i18n( "Copy Here" ), 1 );
   popup.insertItem( i18n( "Move Here" ), 2 );
   popup.insertItem( i18n( "Link Here" ), 3 );
   popup.insertItem( i18n( "Cancel" ), 4 );
	QPoint tmp = widget->mapToGlobal( e->pos() );
   int result = popup.exec( QCursor::pos() );
	
	KIO::CopyJob *job;
   switch ( result ) {
         case 1 :
			job = KIO::copy(lst, dest, true);
         break;
         case 2 :
			job = KIO::move(lst, dest, true);
         break;
         case 3 :
			job = KIO::link(lst, dest, true);
         break;
         case - 1 :         // user pressed outside the menu
         case 4:
         return ; // cancel was pressed;
   }
}
