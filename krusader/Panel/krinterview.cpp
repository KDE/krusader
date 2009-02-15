#include "krviewfactory.h"
#include "krinterview.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "krmousehandler.h"
#include <klocale.h>
#include <kdirlister.h>
#include <QDir>
#include <QDirModel>
#include <QHashIterator>
#include <QHeaderView>
#include "../GUI/krstyleproxy.h"
#include <QItemDelegate>
#include <QPainter>

// dummy. remove this class when no longer needed
class KrInterViewItem: public KrViewItem
{
public:
	KrInterViewItem(KrInterView *parent, vfile *vf): KrViewItem(vf, parent->properties()) {
		_view = parent;
		_vfile = vf;
		if( parent->_model->dummyVfile() == vf )
			dummyVfile = true;
	}
	
	bool isSelected() const {
		const QModelIndex & ndx = _view->_model->vfileIndex( _vfile );
		return _view->selectionModel()->isSelected( ndx );
	}
	void setSelected( bool s ) {
		const QModelIndex & ndx = _view->_model->vfileIndex( _vfile );
		_view->selectionModel()->select( ndx, ( s ? QItemSelectionModel::Select : QItemSelectionModel::Deselect )
			| QItemSelectionModel::Rows );
	}
	QRect itemRect() const {
		const QModelIndex & ndx = _view->_model->vfileIndex( _vfile );
		return _view->visualRect( ndx );
	}
	static void itemHeightChanged()
	{
	} // force the items to resize when icon/font size change
	void redraw() {}

private:
	vfile *_vfile;
	KrInterView * _view;
};

class KrInterViewItemDelegate : public QItemDelegate
{
public:
	KrInterViewItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) {}
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItemV4 opt = option;
		opt.state &= ~QStyle::State_Selected;
		QItemDelegate::paint( painter, opt, index );
	}
};


// code used to register the view
#define INTERVIEW_ID 2
KrViewInstance interView( INTERVIEW_ID, i18n( "&Experimental View" ), 0 /*Qt::ALT + Qt::SHIFT + Qt::Key_D*/,
                          KrInterView::create, KrInterViewItem::itemHeightChanged );
// end of register code

KrInterView::KrInterView( QWidget *parent, bool &left, KConfig *cfg ):
		KrView(cfg),
		QTreeView(parent)
{
	setWidget( this );
	_nameInKConfig=QString( "KrInterView" ) + QString( ( left ? "Left" : "Right" ) ) ;
	KConfigGroup group( krConfig, "Private" );

	_model = new KrVfsModel( this );
	this->setModel(_model);
	this->setRootIsDecorated(false);
	this->setSortingEnabled(true);
	this->sortByColumn( KrVfsModel::Name, Qt::AscendingOrder );
	_model->sort( KrVfsModel::Name, Qt::AscendingOrder );
	connect( _model, SIGNAL( layoutChanged() ), this, SLOT( slotMakeCurrentVisible() ) );
	
	_mouseHandler = new KrMouseHandler( this );
	setSelectionMode( QAbstractItemView::NoSelection );
	setAllColumnsShowFocus( true );
	
	setStyle( new KrStyleProxy() );
	setItemDelegate( new KrInterViewItemDelegate() );
}

KrInterView::~KrInterView()
{
	delete _properties;
	_properties = 0;
	delete _operator;
	_operator = 0;
	delete _model;
	delete _mouseHandler;
	QHashIterator< vfile *, KrInterViewItem *> it( _itemHash );
	while( it.hasNext() )
		delete it.next().value();
	_itemHash.clear();
}

KrViewItem* KrInterView::findItemByName(const QString &name)
{
	if (!_model->ready()) 
		return 0;
		
	QModelIndex ndx = _model->nameIndex( name );
	if( !ndx.isValid() )
		return 0;
	return getKrInterViewItem( ndx );
}

QString KrInterView::getCurrentItem() const
{
	if (!_model->ready()) 
		return QString();
	
	vfile * vf = _model->vfileAt( currentIndex() );
	if( vf == 0 )
		return QString();
	return vf->vfile_getName();
}

KrViewItem* KrInterView::getCurrentKrViewItem()
{
	if (!_model->ready()) 
		return 0;

	return getKrInterViewItem( currentIndex() );
}

KrViewItem* KrInterView::getFirst()
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem( _model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterView::getKrViewItemAt(const QPoint &vp)
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem( indexAt( vp ) );
}

KrViewItem* KrInterView::getLast()
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem(_model->index(_model->rowCount()-1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getNext(KrViewItem *current)
{
	vfile* vf = (vfile *)current->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.row() >= _model->rowCount()-1 )
		return 0;
	return getKrInterViewItem( _model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getPrev(KrViewItem *current)
{
	vfile* vf = (vfile *)current->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.row() <= 0 )
		return 0;
	return getKrInterViewItem( _model->index(ndx.row() - 1, 0, QModelIndex()));
}

void KrInterView::slotMakeCurrentVisible()
{
	scrollTo( currentIndex() );
}

void KrInterView::makeItemVisible(const KrViewItem *item)
{
	vfile* vf = (vfile *)item->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.isValid() )
		scrollTo( ndx );
}

void KrInterView::setCurrentKrViewItem(KrViewItem *item)
{
	vfile* vf = (vfile *)item->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.isValid() )
		setCurrentIndex( ndx );
}

KrViewItem* KrInterView::preAddItem(vfile *vf)
{
	return 0;
}

bool KrInterView::preDelItem(KrViewItem *item)
{
	return false;
}

void KrInterView::redraw()
{
}

void KrInterView::refreshColors()
{
}

void KrInterView::restoreSettings()
{
}

void KrInterView::saveSettings()
{
}

void KrInterView::setCurrentItem(const QString& name)
{
	QModelIndex ndx = _model->nameIndex( name );
	if( ndx.isValid() )
		setCurrentIndex( ndx );
}

void KrInterView::prepareForActive() {
	KrView::prepareForActive();
	setFocus();
	//slotItemDescription( currentItem() );
}

void KrInterView::prepareForPassive() {
	KrView::prepareForPassive();
	//CANCEL_TWO_CLICK_RENAME;
	//if ( renameLineEdit() ->isVisible() )
		//renameLineEdit() ->clearFocus();
}

int KrInterView::itemsPerPage() {
	QRect rect = visualRect( currentIndex() );
	if( !rect.isValid() )
	{
		for( int i=0; i != _model->rowCount(); i++ )
		{
			rect = visualRect( _model->index( i, 0 ) );
			if( rect.isValid() )
				break;
		}
	}
	if( !rect.isValid() )
		return 0;
	int size = (height() - header()->height() ) / rect.height();
	if( size < 0 )
		size = 0;
	return size;
}

void KrInterView::sort()
{
	_model->sort();
}

void KrInterView::updateView()
{
}

void KrInterView::updateItem(KrViewItem* item)
{
}

void KrInterView::clear()
{
	clearSelection();
	_model->clear();
	QHashIterator< vfile *, KrInterViewItem *> it( _itemHash );
	while( it.hasNext() )
		delete it.next().value();
	_itemHash.clear();
	KrView::clear();
}

void KrInterView::addItems(vfs* v, bool addUpDir)
{
	_model->setVfs(v, addUpDir);
	
	this->setCurrentIndex(_model->index(0, 0));
	if( !nameToMakeCurrent().isEmpty() )
		setCurrentItem( nameToMakeCurrent() );
}

void KrInterView::setup()
{

}

void KrInterView::initOperator()
{
	_operator = new KrViewOperator(this, this);
	// klistview emits selection changed, so chain them to operator
	connect(selectionModel(), SIGNAL(selectionChanged( const QItemSelection &, const QItemSelection &)), _operator, SLOT(emitSelectionChanged()));
}

void KrInterView::keyPressEvent( QKeyEvent *e )
{
	if ( !e || !_model->ready() )
		return ; // subclass bug
	if( handleKeyEvent( e ) ) // did the view class handled the event?
		return;
	QTreeView::keyPressEvent( e );
}

void KrInterView::mousePressEvent ( QMouseEvent * ev )
{
	if( _mouseHandler->mousePressEvent( ev ) )
		QTreeView::mousePressEvent( ev );
}

KrInterViewItem * KrInterView::getKrInterViewItem( const QModelIndex & ndx )
{
	vfile * vf = _model->vfileAt( ndx );
	if( !_itemHash.contains( vf ) )
		_itemHash[ vf ] = new KrInterViewItem( this, vf );
	return _itemHash[ vf ];
}
