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
#include <QLineEdit>
#include <QDialog>

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
private:
	mutable int _currentlyEdited;
	mutable bool _dontDraw;
	
public:
	KrInterViewItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ), _currentlyEdited( -1 ) {}
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItemV4 opt = option;
		opt.state &= ~QStyle::State_Selected;
		_dontDraw = ( _currentlyEdited == index.row() ) && (index.column() == KrVfsModel::Extension );
		QItemDelegate::paint( painter, opt, index );
	}
	
	void drawDisplay ( QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text ) const
	{
		if( !_dontDraw )
			QItemDelegate::drawDisplay( painter, option, rect, text );
	}

	QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const
	{
		_currentlyEdited = index.row();
		return QItemDelegate::createEditor( parent, sovi, index );
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		QItemDelegate::setEditorData( editor, index );
		if( editor->inherits( "QLineEdit" ) )
		{
			QLineEdit *lineEdit = qobject_cast<QLineEdit *> ( editor );
			if( lineEdit ) {
				QString nameWithoutExt = index.data( Qt::UserRole ).toString();
				lineEdit->deselect();
				lineEdit->setSelection( 0, nameWithoutExt.length() );
			}
		}
	}
	
	bool eventFilter(QObject *object, QEvent *event)
	{
		QWidget *editor = qobject_cast<QWidget*>(object);
		if (!editor)
			return false;
		if (event->type() == QEvent::KeyPress)
		{
			switch (static_cast<QKeyEvent *>(event)->key()) {
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				_currentlyEdited = -1;
				emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
				return true;
			case Qt::Key_Enter:
			case Qt::Key_Return:
				if (QLineEdit *e = qobject_cast<QLineEdit*>(editor))
				{
					if (!e->hasAcceptableInput())
						return true;
					event->accept();
					emit commitData(editor);
					emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
					_currentlyEdited = -1;
					return true;
				}
				return false;
			case Qt::Key_Escape:
				event->accept();
				// don't commit data
				_currentlyEdited = -1;
				emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
				break;
			default:
				return false;
			}
			
			if (editor->parentWidget())
				editor->parentWidget()->setFocus();
			return true;
		} else if (event->type() == QEvent::FocusOut) {
			if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
				QWidget *w = QApplication::focusWidget();
				while (w) { // don't worry about focus changes internally in the editor
					if (w == editor)
						return false;
					w = w->parentWidget();
				}
				// Opening a modal dialog will start a new eventloop
				// that will process the deleteLater event.
				if (QApplication::activeModalWidget()
					&& !QApplication::activeModalWidget()->isAncestorOf(editor)
					&& qobject_cast<QDialog*>(QApplication::activeModalWidget()))
						return false;
				_currentlyEdited = -1;
				emit closeEditor(editor, RevertModelCache);
			}
		} else if (event->type() == QEvent::ShortcutOverride) {
			if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
				event->accept();
				return true;
			}
		}
		return false;
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
	// fix the context menu problem
	int j = QFontMetrics( font() ).height() * 2;
	_mouseHandler = new KrMouseHandler( this, j );
	connect( _mouseHandler, SIGNAL( renameCurrentItem() ), this, SLOT( renameCurrentItem() ) );
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
	
	setSelectionMode( QAbstractItemView::NoSelection );
	setAllColumnsShowFocus( true );
	
	setStyle( new KrStyleProxy() );
	setItemDelegate( new KrInterViewItemDelegate() );
	setMouseTracking( true );
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
	if( ndx.isValid() && ndx.row() != currentIndex().row() ) {
		_mouseHandler->cancelTwoClickRename();
		setCurrentIndex( ndx );
	}
}

KrViewItem* KrInterView::preAddItem(vfile *vf)
{
	return getKrInterViewItem( _model->addItem( vf ) );
}

bool KrInterView::preDelItem(KrViewItem *item)
{
	if( item == 0 )
		return true;
	QModelIndex ndx = _model->removeItem( (vfile *)item->getVfile() );
	if( ndx.isValid() )
		setCurrentIndex( ndx );
	_itemHash.remove( (vfile *)item->getVfile() );
	return true;
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
	KrViewItem * current = getCurrentKrViewItem();
	if( current != 0 ) {
		QString desc = current->description();
		op()->emitItemDescription( desc );
	}
}

void KrInterView::prepareForPassive() {
	KrView::prepareForPassive();
	_mouseHandler->cancelTwoClickRename();
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
	if( item == 0 )
		return;
	_model->updateItem( (vfile *)item->getVfile() );
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
	if( !_mouseHandler->mousePressEvent( ev ) )
		QTreeView::mousePressEvent( ev );
}

void KrInterView::mouseReleaseEvent ( QMouseEvent * ev )
{
	if( !_mouseHandler->mouseReleaseEvent( ev ) )
		QTreeView::mouseReleaseEvent( ev );
}

void KrInterView::mouseDoubleClickEvent ( QMouseEvent *ev )
{
	if( !_mouseHandler->mouseDoubleClickEvent( ev ) )
		QTreeView::mouseDoubleClickEvent( ev );
}

void KrInterView::mouseMoveEvent ( QMouseEvent * ev )
{
	if( !_mouseHandler->mouseMoveEvent( ev ) )
		QTreeView::mouseMoveEvent( ev );
}

void KrInterView::wheelEvent ( QWheelEvent *ev )
{
	if( !_mouseHandler->wheelEvent( ev ) )
		QTreeView::wheelEvent( ev );
}

bool KrInterView::event( QEvent * e )
{
	_mouseHandler->otherEvent( e );
	return QTreeView::event( e );
}

KrInterViewItem * KrInterView::getKrInterViewItem( const QModelIndex & ndx )
{
	if( !ndx.isValid() )
		return 0;
	vfile * vf = _model->vfileAt( ndx );
	if( vf == 0 )
		return 0;
	if( !_itemHash.contains( vf ) ) {
		KrInterViewItem * newItem =  new KrInterViewItem( this, vf );
		_itemHash[ vf ] = newItem;
		_dict.insert( vf->vfile_getName(), newItem );
	}
	return _itemHash[ vf ];
}

void KrInterView::selectRegion( KrViewItem *i1, KrViewItem *i2, bool select)
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

void KrInterView::renameCurrentItem() {
	QModelIndex cIndex = currentIndex();
	QModelIndex nameIndex = _model->index( cIndex.row(), KrVfsModel::Name );
	edit( nameIndex );
	updateEditorData();
	update( nameIndex );
}
