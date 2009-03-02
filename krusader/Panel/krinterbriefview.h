/***************************************************************************
                       krinterbriefview.h  -  description
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

#ifndef __krinterbriefview__
#define __krinterbriefview__

#include "krview.h"

#include <QAbstractItemView>
#include <QVector>
#include <QFont>

class KrInterBriefViewItem;
class KrMouseHandler;
class KrVfsModel;
class QHeaderView;

class KrInterBriefView : public QAbstractItemView, public KrView {
	friend class KrInterBriefViewItem;
	Q_OBJECT

public:
	KrInterBriefView( QWidget *parent, bool &left, KConfig *cfg = krConfig );
	virtual ~KrInterBriefView();

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
	virtual void restoreSettings();
	virtual void saveSettings();
	virtual void setCurrentItem(const QString& name);
	virtual void setCurrentKrViewItem(KrViewItem *current);
	virtual void sort();
	virtual void clear();
	virtual void updateView();
	virtual void updateItem(KrViewItem* item);
	virtual QModelIndex getCurrentIndex() { return currentIndex(); }
	virtual bool isSelected( const QModelIndex &ndx ) { return selectionModel()->isSelected( ndx ); }
	virtual void selectRegion( KrViewItem *, KrViewItem *, bool );
	KrInterBriefViewItem * getKrInterViewItem( const QModelIndex & );
	
	static KrView* create( QWidget *parent, bool &left, KConfig *cfg ) { return new KrInterBriefView( parent, left, cfg ); }
	
	virtual void prepareForActive();
	virtual void prepareForPassive();
	virtual bool ensureVisibilityAfterSelect() { return false; }
	virtual int  itemsPerPage();
	virtual void setSortMode(KrViewProperties::SortSpec mode);
	
	// abstract item view classes
	virtual QRect visualRect(const QModelIndex&) const;
	virtual void scrollTo(const QModelIndex&, QAbstractItemView::ScrollHint = QAbstractItemView::EnsureVisible);
	virtual QModelIndex indexAt(const QPoint&) const;
	virtual QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers);
	virtual int horizontalOffset() const;
	virtual int verticalOffset() const;
	virtual bool isIndexHidden(const QModelIndex&) const;
	virtual void setSelection(const QRect&, QFlags<QItemSelectionModel::SelectionFlag>);
	virtual QRegion visualRegionForSelection(const QItemSelection&) const;
	virtual void updateGeometries();
	virtual QRect mapToViewport(const QRect &rect) const;
	
public slots:
	virtual void refreshColors();
	
protected slots:
	void slotMakeCurrentVisible();
	virtual void renameCurrentItem();
	
protected:
	virtual void setup();
	virtual void initOperator();
	
	virtual void keyPressEvent( QKeyEvent *e );
	virtual void mousePressEvent ( QMouseEvent * );
	virtual void mouseReleaseEvent ( QMouseEvent * );
	virtual void mouseDoubleClickEvent ( QMouseEvent *ev );
	virtual void mouseMoveEvent ( QMouseEvent * );
	virtual void wheelEvent ( QWheelEvent * );
	virtual bool event( QEvent * e );
	virtual void dragEnterEvent(QDragEnterEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *e);
	virtual void dragLeaveEvent(QDragLeaveEvent *e);
	virtual void dropEvent ( QDropEvent * );
	virtual bool eventFilter(QObject *object, QEvent *event);
	virtual bool viewportEvent ( QEvent * event );
	virtual void paintEvent(QPaintEvent *e);
	
	void showContextMenu( const QPoint & p );
	int getItemHeight() const;
	int elementWidth( const QModelIndex & index );
	
	void intersectionSet( const QRect &, QVector<QModelIndex> & );
	
private:
	KrVfsModel *_model;
	KrMouseHandler *_mouseHandler;
	QHash<vfile *,KrInterBriefViewItem*> _itemHash;
	QFont _viewFont;
	int _numOfColumns;
	int _fileIconSize;
	QHeaderView * _header;
};
#endif // __krinterbriefview__
