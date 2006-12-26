#ifndef KRBRIEFVIEW_H
#define KRBRIEFVIEW_H

#include "krview.h"
#include "krviewitem.h"
#include <kiconview.h>

class KrBriefView: public KIconView, public KrView {
	friend class KrBriefViewItem;
	Q_OBJECT
public:
	KrBriefView( QWidget *parent, bool &left, KConfig *cfg = krConfig, const char *name = 0 );
	virtual ~KrBriefView();
	virtual inline KrViewItem *getFirst() { return dynamic_cast<KrViewItem*>( firstItem() ); }
	virtual inline KrViewItem *getLast() { return dynamic_cast<KrViewItem*>( lastItem() ); }
	virtual inline KrViewItem *getNext( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<QIconViewItem*>( current ) ->nextItem() ); }
   virtual inline KrViewItem *getPrev( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<QIconViewItem*>( current ) ->prevItem() ); }
	virtual inline KrViewItem *getCurrentKrViewItem() { return dynamic_cast<KrViewItem*>( currentItem() ); }
	virtual KrViewItem *getKrViewItemAt(const QPoint &vp);
	virtual inline KrViewItem *findItemByName(const QString &name) { return dynamic_cast<KrViewItem*>( findItem( name, Qt::ExactMatch ) ); }
	virtual void addItems(vfs* /* v */, bool /*addUpDir*/ = true) {}
	virtual KrViewItem *addItem(vfile * /* vf */) {return 0;}
	virtual void delItem(const QString &/* name */) {}
	virtual void updateItem(vfile */* vf */) {}
	virtual QString getCurrentItem() const;
	virtual void setCurrentItem(const QString& /* name */) {}
	virtual void makeItemVisible(const KrViewItem * /* item */) {}
	virtual void clear() {}
	virtual void updateView() {}
	virtual void updateItem(KrViewItem* /* item */) {}
	virtual void sort() {}
	virtual void saveSettings() {}
	virtual void restoreSettings() {}
	virtual void prepareForActive() {}
	virtual void prepareForPassive() {}
	virtual QString nameInKConfig() {return QString::null;}
	virtual void renameCurrentItem() {}

protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
	virtual KrViewItem *preAddItem(vfile * /* vf */) {return 0;}
	virtual bool preDelItem(KrViewItem * /* item */) {return false;}

  
signals:
	void letsDrag(QStringList items, QPixmap icon);
	void gotDrop(QDropEvent *);
	void selectionChanged();
};

#endif
