#ifndef KRBRIEFVIEW_H
#define KRBRIEFVIEW_H

#include "krview.h"
#include <kiconview.h>

class KrBriefView: public KIconView, KrView {
	Q_OBJECT
public:
	virtual KrViewItem *getFirst() {}	
	virtual KrViewItem *getNext(KrViewItem *current) {}
	virtual KrViewItem *getPrev(KrViewItem *current) {}
	virtual KrViewItem *getCurrentKrViewItem() {}
	virtual KrViewItem *getKrViewItemAt(const QPoint &vp) {}
	virtual KrViewItem *findItemByName(const QString &name) {}
	virtual void addItems(vfs* v, bool addUpDir = true) {}
	virtual void addItem(vfile *vf) {}
	virtual void delItem(const QString &name) {}
	virtual void updateItem(vfile *vf) {}
	virtual QString getCurrentItem() const {}
	virtual void setCurrentItem(const QString& name) {}
	virtual void makeItemVisible(const KrViewItem *item) {}
	virtual void clear() {}
	virtual void updateView() {}
	virtual void sort() {}
	virtual void saveSettings() {}
	virtual void restoreSettings() {}
	virtual void prepareForActive() {}
	virtual void prepareForPassive() {}
	virtual QString nameInKConfig() {}
	virtual void renameCurrentItem() {}
protected:
  virtual void initProperties(){}
  
signals:
	void letsDrag(QStringList items, QPixmap icon);
	void gotDrop(QDropEvent *);
	void selectionChanged();
};

#endif
