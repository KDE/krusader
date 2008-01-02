/***************************************************************************
                              krbriefview.h
                           -------------------
  copyright            : (C) 2000-2007 by Shie Erlich & Rafi Yanai & Csaba Karai
  e-mail               : krusader@users.sourceforge.net
  web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

   db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
   88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
   88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
   88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
   88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
   YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                   H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef KRBRIEFVIEW_H
#define KRBRIEFVIEW_H

#include "krview.h"
#include "krviewitem.h"
#include <k3iconview.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

// extends KrViewProperties to add detailedview-only properties
class KrBriefViewProperties: public KrViewProperties {
public:
	int numberOfColumns; // the number of columns in the view
};

class KrBriefViewItem;
class QDragMoveEvent;
class QToolTip;
class Q3Header;

/**
 * KrBriefView implements everthing and anything regarding a brief view in a filemananger.
 * IT MUST USE KrViewItem as the children to it's *K3IconView. KrBriefView and KrViewItem are
 * tightly coupled and the view will not work with other kinds of items.
 * Apart from this, the view is self-reliant and you can use the vast interface to get whatever
 * information is necessery from it.
 */
class KrBriefView: public K3IconView, public KrView {
	friend class KrBriefViewItem;
	Q_OBJECT
public:
	KrBriefView( Q3Header *header, QWidget *parent, bool &left, KConfig *cfg = krConfig );
	virtual ~KrBriefView();
	virtual inline KrViewItem *getFirst() { return dynamic_cast<KrViewItem*>( firstItem() ); }
	virtual inline KrViewItem *getLast() { return dynamic_cast<KrViewItem*>( lastItem() ); }
	virtual inline KrViewItem *getNext( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<Q3IconViewItem*>( current ) ->nextItem() ); }
	virtual inline KrViewItem *getPrev( KrViewItem *current ) { return dynamic_cast<KrViewItem*>( dynamic_cast<Q3IconViewItem*>( current ) ->prevItem() ); }
	virtual inline KrViewItem *getCurrentKrViewItem() { return dynamic_cast<KrViewItem*>( currentItem() ); }
	virtual KrViewItem *getKrViewItemAt(const QPoint &vp);
	virtual inline KrViewItem *findItemByName(const QString &name) { return dynamic_cast<KrViewItem*>( findItem( name, K3IconView::ExactMatch ) ); }
	virtual void addItems(vfs* v, bool addUpDir = true);
	virtual void delItem(const QString &);
	virtual QString getCurrentItem() const;
	virtual void makeItemVisible(const KrViewItem * item );
	virtual void setCurrentItem(const QString& name );
	virtual void updateView();
	virtual void updateItem(KrViewItem* item );
	virtual void clear();
	virtual void sort()                        { if( sortDirection() ) sortOrderChanged();K3IconView::sort( true ); }
	virtual void sort( bool ascending )        { if( sortDirection() != ascending ) sortOrderChanged();K3IconView::sort( ascending ); }
	virtual void prepareForActive();
	virtual void prepareForPassive();
	virtual void saveSettings() {}
	virtual void restoreSettings() {}
	virtual QString nameInKConfig() {return _nameInKConfig;}
	virtual void resizeEvent ( QResizeEvent * );

signals:
	void middleButtonClicked( KrViewItem *item );
	void currentChanged( KrViewItem *item );

protected:
	virtual void setup();
	virtual void initProperties();
	virtual void initOperator();
	virtual KrViewItem *preAddItem(vfile * vf);
	virtual bool preDelItem(KrViewItem * item );

	void setColumnNr();
	void redrawColumns();

	virtual void keyPressEvent( QKeyEvent *e );
	virtual void inputMethodEvent(QInputMethodEvent *e);
	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void contentsMouseReleaseEvent (QMouseEvent *e);
	virtual void contentsMouseMoveEvent ( QMouseEvent * e );
	virtual void contentsMouseDoubleClickEvent ( QMouseEvent * e );
	virtual bool acceptDrag( QDropEvent* e ) const;
	virtual void contentsDropEvent( QDropEvent *e );
	virtual void contentsDragMoveEvent( QDragMoveEvent *e );
	virtual void contentsDragLeaveEvent ( QDragLeaveEvent * );
	virtual void startDrag() { op()->startDrag(); }
	virtual bool event( QEvent *e );
	virtual bool eventFilter( QObject * watched, QEvent * e );
	QMouseEvent * transformMouseEvent( QMouseEvent * );

protected slots:
	void rename( Q3IconViewItem *item );
	void slotClicked( Q3IconViewItem *item );
	void slotDoubleClicked( Q3IconViewItem *item );
	void slotItemDescription( Q3IconViewItem * );
	void slotCurrentChanged( Q3IconViewItem *item );
	void handleContextMenu( Q3IconViewItem*, const QPoint& );
	virtual void renameCurrentItem();
	virtual void showContextMenu( );
	void inplaceRenameFinished( Q3IconViewItem *it );
	void setNameToMakeCurrent( Q3IconViewItem *it );
	void sortOrderChanged();
	void slotRightButtonPressed(Q3IconViewItem*, const QPoint& point);
	void transformCurrentChanged( Q3IconViewItem * item ) { emit currentChanged( dynamic_cast<KrViewItem *>(item ) ); }

	/**
	  * used internally to produce the signal middleButtonClicked()
	 */
	void slotMouseClicked( int button, Q3IconViewItem * item, const QPoint & pos );
	inline void slotExecuted( Q3IconViewItem* i ) {
		QString tmp = dynamic_cast<KrViewItem*>( i ) ->name();
		op()->emitExecuted( tmp );
	}

public slots:
	void refreshColors();
	void quickSearch( const QString &, int = 0 );
	void stopQuickSearch( QKeyEvent* );
	void handleQuickSearchEvent( QKeyEvent* );
	void changeSortOrder();

  
signals:
	void letsDrag(QStringList items, QPixmap icon);
	void gotDrop(QDropEvent *);

private:
	Q3Header * header;
	bool swushSelects;
	QPoint dragStartPos;
	Q3IconViewItem *lastSwushPosition;
	KrViewItem *_currDragItem;
	bool singleClicked;
	bool modifierPressed;
	QTime clickTime;
	Q3IconViewItem *clickedItem;
	QTimer renameTimer;
	QTimer contextMenuTimer;
	QPoint contextMenuPoint;
	KrBriefViewItem *currentlyRenamedItem;
	Q3IconViewItem *pressedItem;
	QMouseEvent *mouseEvent;
	QToolTip *toolTip;
};

#endif
