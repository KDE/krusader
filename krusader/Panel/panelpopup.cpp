#include "../krusader.h"
#include "panelpopup.h"
#include "../kicons.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../defaults.h"
#include "../krslots.h"
#include "panelfunc.h"
#include <qtooltip.h>
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
#include "../KViewer/kimagefilepreview.h"
#include "../KViewer/panelviewer.h"
#include "../KViewer/diskusageviewer.h"

PanelPopup::PanelPopup( QSplitter *parent, bool left ) : QWidget( parent ), 
	_left( left ), _hidden(true), stack( 0 ), viewer( 0 ), pjob( 0 ), splitterSizes() {
	splitter = parent;
	QGridLayout * layout = new QGridLayout(this, 1, 1);
	
	// loading the splitter sizes
	krConfig->setGroup( "Private" );
	if( left )
		splitterSizes = krConfig->readIntListEntry( "Left PanelPopup Splitter Sizes" );
	else
		splitterSizes = krConfig->readIntListEntry( "Right PanelPopup Splitter Sizes" );
	
	if( splitterSizes.count() < 2 ) {
		splitterSizes.clear();
		splitterSizes.push_back( 100 );
		splitterSizes.push_back( 100 );
	}
	
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
	QToolTip::add(treeBtn, i18n("Tree Panel: a tree view of the local file system"));
	treeBtn->setPixmap(krLoader->loadIcon( "view_tree", KIcon::Toolbar, 16 ));
	treeBtn->setFixedSize(20, 20);
	treeBtn->setToggleButton(true);
	btns->insert(treeBtn, Tree);
	
	
	previewBtn = new QToolButton(this);
	QToolTip::add(previewBtn, i18n("Preview Panel: display a preview of the current file"));
	previewBtn->setPixmap(krLoader->loadIcon( "folder_image", KIcon::Toolbar, 16 ));
	previewBtn->setFixedSize(20, 20);
	previewBtn->setToggleButton(true);
	btns->insert(previewBtn, Preview);
	
	quickBtn = new QToolButton(this);
	QToolTip::add(quickBtn, i18n("Quick Panel: quick way to perform actions"));
	quickBtn->setPixmap(krLoader->loadIcon( "kr_select", KIcon::Toolbar, 16 ));
	quickBtn->setFixedSize(20, 20);
	quickBtn->setToggleButton(true);
	btns->insert(quickBtn, QuickPanel);

	viewerBtn = new QToolButton(this);
	QToolTip::add(viewerBtn, i18n("View Panel: view the current file"));
	viewerBtn->setPixmap(krLoader->loadIcon( "viewmag", KIcon::Toolbar, 16 ));
	viewerBtn->setFixedSize(20, 20);
	viewerBtn->setToggleButton(true);
	btns->insert(viewerBtn, View);	
		
	duBtn = new QToolButton(this);
	QToolTip::add(duBtn, i18n("Disk Usage Panel: view the usage of a directory"));
	duBtn->setPixmap(krLoader->loadIcon( "kchart", KIcon::Toolbar, 16 ));
	duBtn->setFixedSize(20, 20);
	duBtn->setToggleButton(true);
	btns->insert(duBtn, DskUsage);	
		
	layout->addWidget(dataLine,0,0);
	layout->addWidget(treeBtn,0,1);
	layout->addWidget(previewBtn,0,2);
	layout->addWidget(quickBtn,0,3);
	layout->addWidget(viewerBtn,0,4);
	layout->addWidget(duBtn,0,5);
	
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
	viewer = new KrusaderImageFilePreview(stack);
   stack->addWidget( viewer, Preview );

	// create the panelview
	
	panelviewer = new PanelViewer(stack);
	stack->addWidget(panelviewer, View);
	connect(panelviewer, SIGNAL(openURLRequest(const KURL &)), this, SLOT(handleOpenURLRequest(const KURL &)));
	
	// create the disk usage view
	
	diskusage = new DiskUsageViewer( stack );
	diskusage->setStatusLabel( dataLine, i18n("Disk Usage: ") );
	stack->addWidget( diskusage, DskUsage );
	connect(diskusage, SIGNAL(openURLRequest(const KURL &)), this, SLOT(handleOpenURLRequest(const KURL &)));
	
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
	layout->addMultiCellWidget(stack,1,1,0,5);
	
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

	hide(); // for not to open the 3rd hand tool at start (selecting the last used tab)
	tabSelected(id);
}

PanelPopup::~PanelPopup() {}

void PanelPopup::show() {
  QWidget::show();
  if( _hidden )
    splitter->setSizes( splitterSizes );
  _hidden = false;
  tabSelected( stack->id(stack->visibleWidget()) );
}


void PanelPopup::hide() {
  if( !_hidden )
    splitterSizes = splitter->sizes();
  QWidget::hide();
  _hidden = true;
  if (stack->id(stack->visibleWidget()) == View) panelviewer->closeURL();
  if (stack->id(stack->visibleWidget()) == DskUsage) diskusage->closeURL();
}

void PanelPopup::saveSizes() {
  krConfig->setGroup( "Private" );

  if( !isHidden() )
    splitterSizes = splitter->sizes();

  if( _left )
    krConfig->writeEntry( "Left PanelPopup Splitter Sizes", splitterSizes );
  else
    krConfig->writeEntry( "Right PanelPopup Splitter Sizes", splitterSizes );
}

void PanelPopup::handleOpenURLRequest(const KURL &url) {
  if (KMimeType::findByURL(url.url())->name() == "inode/directory") ACTIVE_PANEL->func->openUrl(url);
}


void PanelPopup::tabSelected( int id ) {
	KURL url = "";
	if ( ACTIVE_PANEL && ACTIVE_PANEL->func && ACTIVE_PANEL->func->files() && ACTIVE_PANEL->view ) {
		url = ACTIVE_PANEL->func->files()->vfs_getFile( ACTIVE_PANEL->view->getCurrentItem());
	}
	
	stack->raiseWidget( id );
	
	// if tab is tree, set something logical in the data line
	switch (id) {
		case Tree:
			dataLine->setText( i18n("Tree:") );
			if( !isHidden() )
				tree->setFocus();
			break;
		case Preview:
			dataLine->setText( i18n("Preview:") );
			update(url);
			break;
		case QuickPanel:
			dataLine->setText( i18n("Quick Select:") );
			if( !isHidden() )
				quickSelectCombo->setFocus();
			break;
		case View:
			dataLine->setText( i18n("View:") );
			update(url);
			if( !isHidden() && panelviewer->part() && panelviewer->part()->widget() )
				panelviewer->part()->widget()->setFocus();
			break;
		case DskUsage:
			dataLine->setText( i18n("Disk Usage:") );
			update(url);
			break;
	}
	if (id != View) panelviewer->closeURL();  
}

// decide which part to update, if at all
void PanelPopup::update( KURL url ) {
   if ( isHidden() || url.url()=="" ) return ; // failsafe

   KFileItemList lst;

   switch ( stack->id( stack->visibleWidget() ) ) {
      case Preview:
			viewer->showPreview(url);
			dataLine->setText( i18n("Preview: ")+url.fileName() );
			break;
      case View:
			panelviewer->openURL(url);
			dataLine->setText( i18n("View: ")+url.fileName() );
			break;
      case DskUsage:
			if( url.fileName() == ".." )
				url.setFileName( "" );
			if (KMimeType::findByURL(url.url())->name() != "inode/directory")
				url = url.upURL();
			dataLine->setText( i18n("Disk Usage: ")+url.fileName() );
			diskusage->openURL(url);
         break;
      case Tree:  // nothing to do      
         break;
   }
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

#include "panelpopup.moc"
