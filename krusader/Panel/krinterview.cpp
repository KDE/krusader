#include "krviewfactory.h"
#include "krinterview.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include <klocale.h>
#include <kdirlister.h>
#include <QDir>
#include <QDirModel>

// dummy. remove this class when no longer needed
class KrInterViewItem: public KrViewItem
{
public:
	KrInterViewItem(KrInterView *parent, QModelIndex index): KrViewItem(NULL, parent->properties()), _pIndex(index) {
		// create a vfile from our persistent index
		_vfile = new vfile(
			_pIndex.data().toString(),
			0,
			"rwxrwxrwx",
			0,
			false,
			0,
			0,
			"",
			"",
			0,
			-1);
		
		setVfile(_vfile);
	}

	const QString& name(bool withExtension=true) const {
		return _vfile->vfile_getName();
	}
	//virtual inline bool hasExtension() const { return _hasExtension; }
	//virtual inline const QString& extension() const { return _extension; }
	//virtual QString dateTime() const;
	//virtual QString description() const;
	bool isSelected() const {
		return false;
	}
	void setSelected( bool s ) {}
	//virtual QPixmap icon();
	QRect itemRect() const {
		return QRect();
	}
	static void itemHeightChanged() {} // force the items to resize when icon/font size change
	void redraw() {}

private:
	QPersistentModelIndex _pIndex;
	vfile *_vfile;
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
}

KrInterView::~KrInterView()
{
	delete _properties;
	_properties = 0;
	delete _operator;
	_operator = 0;
	delete _model;
}

KrViewItem* KrInterView::findItemByName(const QString &name)
{
	if (!_model->ready()) 
		return 0;
		
	return 0; // TODO
}

QString KrInterView::getCurrentItem() const
{
	if (!_model->ready()) 
		return QString();
		
	return currentIndex().data().toString();
}

KrViewItem* KrInterView::getCurrentKrViewItem()
{
	if (!_model->ready()) 
		return 0;

	return new KrInterViewItem(this, currentIndex());
}

KrViewItem* KrInterView::getFirst()
{
	if (!_model->ready()) 
		return 0;
	
	return new KrInterViewItem(this, _model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterView::getKrViewItemAt(const QPoint &vp)
{
	return 0;
}

KrViewItem* KrInterView::getLast()
{
	if (!_model->ready()) 
		return 0;
	
	return new KrInterViewItem(this, _model->index(_model->rowCount()-1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getNext(KrViewItem *current)
{
	return 0;
}

KrViewItem* KrInterView::getPrev(KrViewItem *current)
{
	return 0;
}

void KrInterView::makeItemVisible(const KrViewItem *item)
{
}

void KrInterView::setCurrentKrViewItem(KrViewItem *item)
{
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
}

void KrInterView::sort()
{
}

void KrInterView::updateView()
{
}

void KrInterView::updateItem(KrViewItem* item)
{
}

void KrInterView::addItems(vfs* v, bool addUpDir)
{
	_model->setVfs(v);
	this->setCurrentIndex(_model->index(0, 0));
	
#if 0
	Q3ListViewItem * item = firstChild();
	Q3ListViewItem *currentItem = item;
	QString size, name;

	// add the up-dir arrow if needed
	if ( addUpDir ) {
		new KrDetailedViewItem( this, ( Q3ListViewItem* ) 0L, ( vfile* ) 0L );
	}

	// text for updating the status bar
	QString statusText = QString("%1/  ").arg( v->vfs_getOrigin().fileName() ) + i18n("Directory");

	int cnt = 0;
	int cl = columnSorted();
	bool as = ascendingSort();
	setSorting( -1 ); // disable sorting

	for ( vfile * vf = v->vfs_getFirstFile(); vf != 0 ; vf = v->vfs_getNextFile() ) {
		size = KRpermHandler::parseSize( vf->vfile_getSize() );
		name = vf->vfile_getName();
		bool isDir = vf->vfile_isDir();
		if ( !isDir || ( isDir && ( _properties->filter & KrViewProperties::ApplyToDirs ) ) ) {
			switch ( _properties->filter ) {
			case KrViewProperties::All :
				break;
			case KrViewProperties::Custom :
				if ( !_properties->filterMask.match( vf ) )
					continue;
				break;
			case KrViewProperties::Dirs:
				if ( !vf->vfile_isDir() )
					continue;
				break;
			case KrViewProperties::Files:
				if ( vf->vfile_isDir() )
					continue;
				break;

			case KrViewProperties::ApplyToDirs :
				break; // no-op, stop compiler complaints
			}
		}

		KrDetailedViewItem *dvitem = new KrDetailedViewItem( this, item, vf );
		_dict.insert( vf->vfile_getName(), dvitem );
		if ( isDir )
			++_numDirs;
		else
			_countSize += dvitem->VF->vfile_getSize();
		++_count;
		// if the item should be current - make it so
		if ( dvitem->name() == nameToMakeCurrent() ) {
			currentItem = static_cast<Q3ListViewItem*>(dvitem);
			statusText = dvitem->description();
		}

		cnt++;
	}


	// re-enable sorting
	setSorting( cl, as );
	sort();

	if ( !currentItem )
		currentItem = firstChild();
	K3ListView::setCurrentItem( currentItem );
	ensureItemVisible( currentItem );

	op()->emitItemDescription( statusText );
#endif
}

void KrInterView::setup()
{

}

void KrInterView::initOperator()
{
	_operator = new KrViewOperator(this, this);
	// klistview emits selection changed, so chain them to operator
	connect(this, SIGNAL(selectionChanged()), _operator, SLOT(emitSelectionChanged()));
}
