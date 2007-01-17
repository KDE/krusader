#ifndef KRBRIEFVIEW_H
#define KRBRIEFVIEW_H

#include "krview.h"
#include "krviewitem.h"
#include <kiconview.h>
#include <qtimer.h>

class KrBriefViewItem;
class QDragMoveEvent;

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
	virtual void addItems(vfs* v, bool addUpDir = true);
	virtual void delItem(const QString &);
	virtual void updateItem(vfile */* vf */) {}
	virtual QString getCurrentItem() const;
	virtual void setCurrentItem(const QString& name );
	virtual void makeItemVisible(const KrViewItem * item );
	virtual void clear();
	virtual void updateView() {}
	virtual void updateItem(KrViewItem* /* item */) {}
	virtual void sort()                        { KIconView::sort( true ); }
	virtual void sort( bool ascending )        { KIconView::sort( ascending ); }
	virtual void saveSettings() {}
	virtual void restoreSettings() {}
	virtual void prepareForActive();
	virtual void prepareForPassive();
	virtual QString nameInKConfig() {return QString::null;}
	virtual void resizeEvent ( QResizeEvent * );

protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
	virtual KrViewItem *preAddItem(vfile * vf);
	virtual bool preDelItem(KrViewItem * item );

	virtual void keyPressEvent( QKeyEvent *e );
	virtual void imStartEvent( QIMEvent* e );
	virtual void imEndEvent( QIMEvent *e );
	virtual void imComposeEvent( QIMEvent *e );
	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void contentsMouseReleaseEvent (QMouseEvent *e);
	virtual void contentsMouseMoveEvent ( QMouseEvent * e );
	virtual bool acceptDrag( QDropEvent* e ) const;
	virtual void contentsDropEvent( QDropEvent *e );
	virtual void contentsDragMoveEvent( QDragMoveEvent *e );
	virtual void startDrag() { op()->startDrag(); }
	virtual bool event( QEvent *e );

signals:
	void middleButtonClicked( KrViewItem *item );
	void currentChanged( KrViewItem *item );


protected slots:
	void rename( QIconViewItem *item );
	void slotClicked( QIconViewItem *item );
	void slotDoubleClicked( QIconViewItem *item );
	void slotItemDescription( QIconViewItem * );
	void slotCurrentChanged( QIconViewItem *item );
	void handleContextMenu( QIconViewItem*, const QPoint& );
	virtual void renameCurrentItem();
	virtual void showContextMenu( );
	void inplaceRenameFinished( QIconViewItem *it );
	void setNameToMakeCurrent( QIconViewItem *it );
	void slotRightButtonPressed(QIconViewItem*, const QPoint& point);
	void transformCurrentChanged( QIconViewItem * item ) { emit currentChanged( dynamic_cast<KrViewItem *>(item ) ); }

	/**
	  * used internally to produce the signal middleButtonClicked()
	 */
	void slotMouseClicked( int button, QIconViewItem * item, const QPoint & pos );
	inline void slotExecuted( QIconViewItem* i ) {
		QString tmp = dynamic_cast<KrViewItem*>( i ) ->name();
		op()->emitExecuted( tmp );
	}

public slots:
	void refreshColors();
	void quickSearch( const QString &, int = 0 );
	void stopQuickSearch( QKeyEvent* );
	void handleQuickSearchEvent( QKeyEvent* );

  
signals:
	void letsDrag(QStringList items, QPixmap icon);
	void gotDrop(QDropEvent *);

private:
	bool swushSelects;
	QPoint dragStartPos;
	QIconViewItem *lastSwushPosition;
	KrViewItem *_currDragItem;
	bool singleClicked;
	bool modifierPressed;
	QTime clickTime;
	QIconViewItem *clickedItem;
	QTimer renameTimer;
	QTimer contextMenuTimer;
	QPoint contextMenuPoint;
	KrBriefViewItem *currentlyRenamedItem;
	QIconViewItem *pressedItem;
};

#endif
