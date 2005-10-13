#include "krbriefview.h"
#include "../defaults.h"
#include "krcolorcache.h"
#include "krselectionmode.h"
#include "../krusader.h"
#include "../kicons.h"
#include "../krslots.h"

KrBriefView::KrBriefView( QWidget *parent, bool &left, KConfig *cfg, const char *name ):
	KIconView(parent, name), KrView( cfg ) {
	setWidget( this );
	_nameInKConfig = QString( "KrBriefView" ) + QString( ( left ? "Left" : "Right" ) );
}

KrBriefView::~KrBriefView() {
	delete _properties; _properties = 0;
	delete _operator; _operator = 0;
}

void KrBriefView::setup() {
   { // use the {} so that KConfigGroupSaver will work correctly!
      KConfigGroupSaver grpSvr( _config, "Look&Feel" );
      setFont( _config->readFontEntry( "Filelist Font", _FilelistFont ) );
      // decide on single click/double click selection
      if ( _config->readBoolEntry( "Single Click Selects", _SingleClickSelects ) ) {
         connect( this, SIGNAL( executed( QIconViewItem* ) ), this, SLOT( slotExecuted( QIconViewItem* ) ) );
      } else {
         connect( this, SIGNAL( clicked( QIconViewItem* ) ), this, SLOT( slotClicked( QIconViewItem* ) ) );
         connect( this, SIGNAL( doubleClicked( QIconViewItem* ) ), this, SLOT( slotDoubleClicked( QIconViewItem* ) ) );
      }

      // a change in the selection needs to update totals
      connect( this, SIGNAL( onItem( QIconViewItem* ) ), this, SLOT( slotItemDescription( QIconViewItem* ) ) );
      connect( this, SIGNAL( contextMenuRequested( QIconViewItem*, const QPoint&, int ) ),
               this, SLOT( handleContextMenu( QIconViewItem*, const QPoint&, int ) ) );
		connect( this, SIGNAL( rightButtonPressed(QIconViewItem*, const QPoint&, int)),
			this, SLOT(slotRightButtonPressed(QIconViewItem*, const QPoint&, int)));
      connect( this, SIGNAL( currentChanged( QIconViewItem* ) ), this, SLOT( setNameToMakeCurrent( QIconViewItem* ) ) );
      connect( this, SIGNAL( mouseButtonClicked ( int, QIconViewItem *, const QPoint &, int ) ),
               this, SLOT( slotMouseClicked ( int, QIconViewItem *, const QPoint &, int ) ) );
      connect( &KrColorCache::getColorCache(), SIGNAL( colorsRefreshed() ), this, SLOT( refreshColors() ) );
   }
   
   // determine basic settings for the view
   setAcceptDrops( true );
	setItemsMovable( false );
   // TODO: setDragEnabled( true );
   // TODO: setDropVisualizer(false);
   // TODO: setDropHighlighter(true);
   // TODO: setSelectionModeExt( KListView::FileManager );

   //---- don't enable these lines, as it causes an ugly bug with inplace renaming
   //-->  setItemsRenameable( true );
   //-->  setRenameable( column( Name ), true );
   //-------------------------------------------------------------------------------

   // TODO: renameLineEdit()->installEventFilter( this );
   
   // allow in-place renaming
// TODO
/*   connect( renameLineEdit(), SIGNAL( done( QListViewItem *, int ) ),
            this, SLOT( inplaceRenameFinished( QListViewItem*, int ) ) );
   connect( &renameTimer, SIGNAL( timeout() ), this, SLOT( renameCurrentItem() ) );
   connect( &contextMenuTimer, SIGNAL (timeout()), this, SLOT (showContextMenu()));*/

	// TODO: connect( header(), SIGNAL(clicked(int)), this, SLOT(slotSortOrderChanged(int )));

   setFocusPolicy( StrongFocus );
   restoreSettings();
   // TODO: refreshColors();

   //CANCEL_TWO_CLICK_RENAME;	
}

void KrBriefView::initProperties() {
	// TODO: move this to a general location, maybe KrViewProperties constructor ?
	_properties = new KrViewProperties;
	KConfigGroupSaver grpSvr( _config, "Look&Feel" );
	_properties->displayIcons = _config->readBoolEntry( "With Icons", _WithIcons );
	_properties->sortMode = static_cast<KrViewProperties::SortSpec>( KrViewProperties::Name |
		KrViewProperties::Descending | KrViewProperties::DirsFirst );
	if ( !_config->readBoolEntry( "Case Sensative Sort", _CaseSensativeSort ) )
      _properties->sortMode = static_cast<KrViewProperties::SortSpec>( _properties->sortMode |
				 KrViewProperties::IgnoreCase );
	_properties->localeAwareCompareIsCaseSensitive = QString( "a" ).localeAwareCompare( "B" ) > 0; // see KDE bug #40131
}

void KrBriefView::initOperator() {
	_operator = new KrViewOperator(this, this);
	// QIconView emits selection changed, so chain them to operator
	connect(this, SIGNAL(selectionChanged()), _operator, SIGNAL(selectionChanged()));
}

KrViewItem *KrBriefView::getKrViewItemAt( const QPoint & vp ) {
   return dynamic_cast<KrViewItem*>( KIconView::findItem( vp ) );
}

QString KrBriefView::getCurrentItem() const {
   QIconViewItem * it = currentItem();
   if ( !it )
      return QString::null;
   else
      return dynamic_cast<KrViewItem*>( it ) ->name();
}
