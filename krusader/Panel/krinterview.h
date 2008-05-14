#ifndef __krinterview__
#define __krinterview__

#include "krview.h"

#include <QListView>

class KrInterView : public KrView, public QListView {

public:
	KrInterView( QWidget *parent, bool &left, KConfig *cfg = krConfig );
	virtual ~KrInterView();

public:
	virtual void addItems(vfs* v, bool addUpDir = true);
	virtual KrViewItem* findItemByName(const QString &name);
	virtual QString getCurrentItem() const;
	virtual KrViewItem* getCurrentKrViewItem();
	virtual KrViewItem* getFirst();
	virtual KrViewItem* getKrViewItemAt(const QPoint &vp);
	virtual KrViewItem* getLast();
	virtual KrViewItem* getNext(KrViewItem *current);
	virtual KrViewItem* getPrev(KrViewItem *current);
	virtual void makeItemVisible(const KrViewItem *item);
	virtual KrViewItem* preAddItem(vfile *vf);
	virtual bool preDelItem(KrViewItem *item);
	virtual void redraw();
	virtual void refreshColors();
	virtual void restoreSettings();
	virtual void saveSettings();
	virtual void setCurrentItem(const QString& name);
	virtual void sort();
	virtual void updateView();
	virtual void updateItem(KrViewItem* item);
	
	static KrView* create( QWidget *parent, bool &left, KConfig *cfg ) { return new KrInterView( parent, left, cfg ); }
	
protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
};
#endif // __krinterview__
