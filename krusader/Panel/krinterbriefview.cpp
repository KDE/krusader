/***************************************************************************
                      krinterbriefview.cpp  -  description
                             -------------------
    begin                : Sat Feb 14 2009
    copyright            : (C) 2009+ by Csaba Karai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krinterbriefview.h"
#include "krviewfactory.h"
#include "krinterviewitemdelegate.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "krmousehandler.h"
#include "krcolorcache.h"
#include <klocale.h>
#include <kdirlister.h>
#include <QDir>
#include <QDirModel>
#include <QHashIterator>
#include <QHeaderView>
#include "../GUI/krstyleproxy.h"
#include <KMenu>
#include <QPainter>
#include <QScrollBar>

// dummy. remove this class when no longer needed
class KrInterBriefViewItem: public KrViewItem
{
public:
	KrInterBriefViewItem(KrInterBriefView *parent, vfile *vf): KrViewItem(vf, parent->properties()) {
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
	KrInterBriefView * _view;
};


// code used to register the view
#define INTERBRIEFVIEW_ID 3
KrViewInstance interBriefView( INTERBRIEFVIEW_ID, i18n( "&Experimental View" ), 0 /*Qt::ALT + Qt::SHIFT + Qt::Key_D*/,
                                  KrInterBriefView::create, KrInterBriefViewItem::itemHeightChanged );
// end of register code

KrInterBriefView::KrInterBriefView( QWidget *parent, bool &left, KConfig *cfg ):
		KrView(cfg),
		QAbstractItemView(parent)
{
	// fix the context menu problem
	int j = QFontMetrics( font() ).height() * 2;
	_mouseHandler = new KrMouseHandler( this, j );
	connect( _mouseHandler, SIGNAL( renameCurrentItem() ), this, SLOT( renameCurrentItem() ) );
	setWidget( this );
	_nameInKConfig=QString( "KrInterBriefView" ) + QString( ( left ? "Left" : "Right" ) ) ;
	KConfigGroup group( krConfig, "Private" );

	KConfigGroup grpSvr( _config, "Look&Feel" );
	_viewFont = grpSvr.readEntry( "Filelist Font", *_FilelistFont );
	_fileIconSize = (grpSvr.readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
	
	_model = new KrVfsModel( this );
	this->setModel(_model);
	_model->sort( KrVfsModel::Name, Qt::AscendingOrder );
	_model->setExtensionEnabled( false );
	connect( _model, SIGNAL( layoutChanged() ), this, SLOT( slotMakeCurrentVisible() ) );
	//header()->installEventFilter( this );
	
	setSelectionMode( QAbstractItemView::NoSelection );
	
	setStyle( new KrStyleProxy() );
	setItemDelegate( new KrInterViewItemDelegate() );
	setMouseTracking( true );
	setAcceptDrops( true );
	setDropIndicatorShown( true );
	
	restoreSettings();
	connect( &KrColorCache::getColorCache(), SIGNAL( colorsRefreshed() ), this, SLOT( refreshColors() ) );
}

KrInterBriefView::~KrInterBriefView()
{
	delete _properties;
	_properties = 0;
	delete _operator;
	_operator = 0;
	delete _model;
	delete _mouseHandler;
	QHashIterator< vfile *, KrInterBriefViewItem *> it( _itemHash );
	while( it.hasNext() )
		delete it.next().value();
	_itemHash.clear();
}

KrViewItem* KrInterBriefView::findItemByName(const QString &name)
{
	if (!_model->ready()) 
		return 0;
		
	QModelIndex ndx = _model->nameIndex( name );
	if( !ndx.isValid() )
		return 0;
	return getKrInterViewItem( ndx );
}

QString KrInterBriefView::getCurrentItem() const
{
	if (!_model->ready()) 
		return QString();
	
	vfile * vf = _model->vfileAt( currentIndex() );
	if( vf == 0 )
		return QString();
	return vf->vfile_getName();
}

KrViewItem* KrInterBriefView::getCurrentKrViewItem()
{
	if (!_model->ready()) 
		return 0;

	return getKrInterViewItem( currentIndex() );
}

KrViewItem* KrInterBriefView::getFirst()
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem( _model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getKrViewItemAt(const QPoint &vp)
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem( indexAt( vp ) );
}

KrViewItem* KrInterBriefView::getLast()
{
	if (!_model->ready()) 
		return 0;
	
	return getKrInterViewItem(_model->index(_model->rowCount()-1, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getNext(KrViewItem *current)
{
	vfile* vf = (vfile *)current->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.row() >= _model->rowCount()-1 )
		return 0;
	return getKrInterViewItem( _model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getPrev(KrViewItem *current)
{
	vfile* vf = (vfile *)current->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.row() <= 0 )
		return 0;
	return getKrInterViewItem( _model->index(ndx.row() - 1, 0, QModelIndex()));
}

void KrInterBriefView::slotMakeCurrentVisible()
{
	scrollTo( currentIndex() );
}

void KrInterBriefView::makeItemVisible(const KrViewItem *item)
{
	vfile* vf = (vfile *)item->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.isValid() )
		scrollTo( ndx );
}

void KrInterBriefView::setCurrentKrViewItem(KrViewItem *item)
{
	vfile* vf = (vfile *)item->getVfile();
	QModelIndex ndx = _model->vfileIndex( vf );
	if( ndx.isValid() && ndx.row() != currentIndex().row() ) {
		_mouseHandler->cancelTwoClickRename();
		setCurrentIndex( ndx );
	}
}

KrViewItem* KrInterBriefView::preAddItem(vfile *vf)
{
	return getKrInterViewItem( _model->addItem( vf ) );
}

bool KrInterBriefView::preDelItem(KrViewItem *item)
{
	if( item == 0 )
		return true;
	QModelIndex ndx = _model->removeItem( (vfile *)item->getVfile() );
	if( ndx.isValid() )
		setCurrentIndex( ndx );
	_itemHash.remove( (vfile *)item->getVfile() );
	return true;
}

void KrInterBriefView::redraw()
{
}

void KrInterBriefView::refreshColors()
{
	if( _model->rowCount() != 0 )
		_model->emitChanged();
}

void KrInterBriefView::restoreSettings()
{
	/* TODO */
	_numOfColumns = 3;
}

void KrInterBriefView::saveSettings()
{
	/* TODO */
}

void KrInterBriefView::setCurrentItem(const QString& name)
{
	QModelIndex ndx = _model->nameIndex( name );
	if( ndx.isValid() )
		setCurrentIndex( ndx );
}

void KrInterBriefView::prepareForActive() {
	KrView::prepareForActive();
	setFocus();
	KrViewItem * current = getCurrentKrViewItem();
	if( current != 0 ) {
		QString desc = current->description();
		op()->emitItemDescription( desc );
	}
}

void KrInterBriefView::prepareForPassive() {
	KrView::prepareForPassive();
	_mouseHandler->cancelTwoClickRename();
	//if ( renameLineEdit() ->isVisible() )
		//renameLineEdit() ->clearFocus();
}

int KrInterBriefView::itemsPerPage() {
	/* TODO */
	return 0;
}

void KrInterBriefView::sort()
{
	_model->sort();
}

void KrInterBriefView::updateView()
{
}

void KrInterBriefView::updateItem(KrViewItem* item)
{
	if( item == 0 )
		return;
	_model->updateItem( (vfile *)item->getVfile() );
}

void KrInterBriefView::clear()
{
	clearSelection();
	_model->clear();
	QHashIterator< vfile *, KrInterBriefViewItem *> it( _itemHash );
	while( it.hasNext() )
		delete it.next().value();
	_itemHash.clear();
	KrView::clear();
}

void KrInterBriefView::addItems(vfs* v, bool addUpDir)
{
	_model->setVfs(v, addUpDir);
	_count = _model->rowCount();
	if( addUpDir )
		_count--;
	
	this->setCurrentIndex(_model->index(0, 0));
	if( !nameToMakeCurrent().isEmpty() )
		setCurrentItem( nameToMakeCurrent() );
}

void KrInterBriefView::setup()
{

}

void KrInterBriefView::initOperator()
{
	_operator = new KrViewOperator(this, this);
	// klistview emits selection changed, so chain them to operator
	connect(selectionModel(), SIGNAL(selectionChanged( const QItemSelection &, const QItemSelection &)), _operator, SLOT(emitSelectionChanged()));
}

void KrInterBriefView::keyPressEvent( QKeyEvent *e )
{
	if ( !e || !_model->ready() )
		return ; // subclass bug
	if( handleKeyEvent( e ) ) // did the view class handled the event?
		return;
	QAbstractItemView::keyPressEvent( e );
}

void KrInterBriefView::mousePressEvent ( QMouseEvent * ev )
{
	if( !_mouseHandler->mousePressEvent( ev ) )
		QAbstractItemView::mousePressEvent( ev );
}

void KrInterBriefView::mouseReleaseEvent ( QMouseEvent * ev )
{
	if( !_mouseHandler->mouseReleaseEvent( ev ) )
		QAbstractItemView::mouseReleaseEvent( ev );
}

void KrInterBriefView::mouseDoubleClickEvent ( QMouseEvent *ev )
{
	if( !_mouseHandler->mouseDoubleClickEvent( ev ) )
		QAbstractItemView::mouseDoubleClickEvent( ev );
}

void KrInterBriefView::mouseMoveEvent ( QMouseEvent * ev )
{
	if( !_mouseHandler->mouseMoveEvent( ev ) )
		QAbstractItemView::mouseMoveEvent( ev );
}

void KrInterBriefView::wheelEvent ( QWheelEvent *ev )
{
	if( !_mouseHandler->wheelEvent( ev ) )
		QAbstractItemView::wheelEvent( ev );
}

void KrInterBriefView::dragEnterEvent ( QDragEnterEvent *ev )
{
	if( !_mouseHandler->dragEnterEvent( ev ) )
		QAbstractItemView::dragEnterEvent( ev );
}

void KrInterBriefView::dragMoveEvent ( QDragMoveEvent *ev )
{
	QAbstractItemView::dragMoveEvent( ev );
	_mouseHandler->dragMoveEvent( ev );
}

void KrInterBriefView::dragLeaveEvent ( QDragLeaveEvent *ev )
{
	if( !_mouseHandler->dragLeaveEvent( ev ) )
		QAbstractItemView::dragLeaveEvent( ev );
}

void KrInterBriefView::dropEvent ( QDropEvent *ev )
{
	if( !_mouseHandler->dropEvent( ev ) )
		QAbstractItemView::dropEvent( ev );
}

bool KrInterBriefView::event( QEvent * e )
{
	_mouseHandler->otherEvent( e );
	return QAbstractItemView::event( e );
}

KrInterBriefViewItem * KrInterBriefView::getKrInterViewItem( const QModelIndex & ndx )
{
	if( !ndx.isValid() )
		return 0;
	vfile * vf = _model->vfileAt( ndx );
	if( vf == 0 )
		return 0;
	QHash<vfile *,KrInterBriefViewItem*>::iterator it = _itemHash.find( vf );
	if( it == _itemHash.end() ) {
		KrInterBriefViewItem * newItem =  new KrInterBriefViewItem( this, vf );
		_itemHash[ vf ] = newItem;
		_dict.insert( vf->vfile_getName(), newItem );
		return newItem;
	}
	return *it;
}

void KrInterBriefView::selectRegion( KrViewItem *i1, KrViewItem *i2, bool select)
{
	vfile* vf1 = (vfile *)i1->getVfile();
	QModelIndex mi1 = _model->vfileIndex( vf1 );
	vfile* vf2 = (vfile *)i2->getVfile();
	QModelIndex mi2 = _model->vfileIndex( vf2 );
	
	if( mi1.isValid() && mi2.isValid() )
	{
		int r1 = mi1.row();
		int r2 = mi2.row();
		
		if( r1 > r2 ) {
			int t = r1;
			r1 = r2;
			r2 = t;
		}
		
		for( int row = r1; row <= r2; row++ )
		{
			const QModelIndex & ndx = _model->index( row, 0 );
			selectionModel()->select( ndx, ( select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect )
			| QItemSelectionModel::Rows );
		}
	}
	else if( mi1.isValid() && !mi2.isValid() )
		i1->setSelected( select );
	else if( mi2.isValid() && !mi1.isValid() )
		i2->setSelected( select );
}

void KrInterBriefView::renameCurrentItem() {
	QModelIndex cIndex = currentIndex();
	QModelIndex nameIndex = _model->index( cIndex.row(), KrVfsModel::Name );
	edit( nameIndex );
	updateEditorData();
	update( nameIndex );
}

bool KrInterBriefView::eventFilter(QObject *object, QEvent *event)
{
	/* TODO */
	return false;
}

void KrInterBriefView::showContextMenu( const QPoint & p )
{
	/* TODO */
}

bool KrInterBriefView::viewportEvent ( QEvent * event )
{
	/* TODO */
	return QAbstractItemView::viewportEvent( event );
}


QRect KrInterBriefView::visualRect(const QModelIndex&ndx) const
{
	/* TODO */
	int width = (viewport()->width())/_numOfColumns;
	if( (viewport()->width())%_numOfColumns )
		width++;
	int height = getItemHeight();
	int numRows = viewport()->height() / height;
	if( numRows == 0 )
		numRows++;
	int x = width * (ndx.row()/numRows);
	int y = height * (ndx.row() % numRows );
	return mapToViewport(QRect( x, y, width, height ));
}

void KrInterBriefView::scrollTo(const QModelIndex&, QAbstractItemView::ScrollHint)
{
	/* TODO */
}

QModelIndex KrInterBriefView::indexAt(const QPoint& p) const
{
	int x = p.x() + horizontalOffset();
	int y = p.y() + verticalOffset();
	
	int width = (viewport()->width())/_numOfColumns;
	if( (viewport()->width())%_numOfColumns )
		width++;
	int height = getItemHeight();
	int numRows = viewport()->height() / height;
	if( numRows == 0 )
		numRows++;
	int ys = y / height;
	int xs = (x / width) * numRows;
	
	return _model->index( xs + ys, 0 );
}

QModelIndex KrInterBriefView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
	/* TODO */
	return QModelIndex();
}

int KrInterBriefView::horizontalOffset() const
{
	return horizontalScrollBar()->value();
}

int KrInterBriefView::verticalOffset() const
{
	return 0;
}

bool KrInterBriefView::isIndexHidden(const QModelIndex&ndx) const
{
	return ndx.column() != 0;
}

void KrInterBriefView::setSelection(const QRect&, QFlags<QItemSelectionModel::SelectionFlag>)
{
	/* TODO */
}

QRegion KrInterBriefView::visualRegionForSelection(const QItemSelection&) const
{
	/* TODO */
	return QRegion();
}

void KrInterBriefView::paintEvent(QPaintEvent *e)
{
	/* TODO */
	QStyleOptionViewItemV4 option;
	option.widget = this;
	QPainter painter( viewport() );
	
	for( int i=0; i != _model->rowCount(); i++ )
	{
		option.rect = visualRect( _model->index( i, 0 ) );
		painter.save();
		itemDelegate()->paint(&painter, option, _model->index( i, 0 ) );
		painter.restore();
	}
}

int KrInterBriefView::getItemHeight() const {
	int textHeight = QFontMetrics( _viewFont ).height();
	int height = textHeight;
	int iconSize = 0;
	if( properties()->displayIcons )
		iconSize = _fileIconSize;
	if( iconSize > textHeight )
		height = iconSize;
	return height;
}

void KrInterBriefView::updateGeometries()
{
	if (_model->rowCount() <= 0 )
		horizontalScrollBar()->setRange(0, 0);
	else
	{
		int itemsPerColumn = viewport()->height() / getItemHeight();
		if( itemsPerColumn <= 0 )
			itemsPerColumn = 1;
		int columnWidth = (viewport()->width())/_numOfColumns;
		if( (viewport()->width())%_numOfColumns )
			columnWidth++;
		int maxWidth = _model->rowCount() / itemsPerColumn;
		if( _model->rowCount() % itemsPerColumn )
			maxWidth++;
		maxWidth *= columnWidth;
		if( maxWidth > viewport()->width() )
		{
			horizontalScrollBar()->setSingleStep(columnWidth);
			horizontalScrollBar()->setPageStep(columnWidth * _numOfColumns);
			horizontalScrollBar()->setRange(0, maxWidth - viewport()->width());
		} else {
			horizontalScrollBar()->setRange(0, 0);
		}
	}

	QAbstractItemView::updateGeometries();
}

QRect KrInterBriefView::mapToViewport(const QRect &rect) const
{
    if (!rect.isValid())
        return rect;

    QRect result = rect;

    int dx = -horizontalOffset();
    int dy = -verticalOffset();
    result.adjust(dx, dy, dx, dy);
    return result;
}
