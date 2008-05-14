#include "krviewfactory.h"
#include "krinterview.h"
#include "krviewitem.h"
#include <klocale.h>

#include <QDirModel>

// dummy. remove this class when no longer needed
class KrInterViewItem: public KrViewItem
{
public:
	static void itemHeightChanged() {} // force the items to resize when icon/font size change

};

// code used to register the view
#define INTERVIEW_ID 2
KrViewInstance interView( INTERVIEW_ID, i18n( "&Experimental View" ), 0 /*Qt::ALT + Qt::SHIFT + Qt::Key_D*/,
                          KrInterView::create, KrInterViewItem::itemHeightChanged );
// end of register code

KrInterView::KrInterView( QWidget *parent, bool &left, KConfig *cfg ):
		KrView(cfg),
		QListView(parent)
{
	setWidget( this );
	_nameInKConfig=QString( "KrInterView" ) + QString( ( left ? "Left" : "Right" ) ) ;
	KConfigGroup group( krConfig, "Private" );
	
	
	// dummy code
	QDirModel *model = new QDirModel;
	//QTreeView *tree = new QTreeView(this);
	this->setModel(model);
	this->setRootIndex(model->index(QDir::currentPath()));
	//setWidget(tree);
	//tree->show();
}

KrInterView::~KrInterView()
{
	delete _properties; _properties = 0;
	delete _operator; _operator = 0;
}

KrViewItem* KrInterView::findItemByName(const QString &name)
{
	return 0;
}

QString KrInterView::getCurrentItem() const
{
	return QString();
}

KrViewItem* KrInterView::getCurrentKrViewItem()
{
	return 0;
}

KrViewItem* KrInterView::getFirst()
{
	return 0;
}

KrViewItem* KrInterView::getKrViewItemAt(const QPoint &vp)
{
	return 0;
}

KrViewItem* KrInterView::getLast()
{
	return 0;
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
}

void KrInterView::setup()
{

}

void KrInterView::initProperties()
{
	_properties = new KrViewProperties();

}

void KrInterView::initOperator()
{
	_operator = new KrViewOperator(this, this);
	// klistview emits selection changed, so chain them to operator
	connect(this, SIGNAL(selectionChanged()), _operator, SIGNAL(selectionChanged()));
}
