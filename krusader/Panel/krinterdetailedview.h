/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRINTERDETAILEDVIEW_H
#define KRINTERDETAILEDVIEW_H

#include <QTreeView>
#include <QVector>
#include <QFont>

#include "krinterview.h"

class KrVfsModel;
class KrInterDetailedViewItem;
class QMouseEvent;
class QKeyEvent;
class QDragEnterEvent;
class QContextMenuEvent;
class KrMouseHandler;

class KrInterDetailedView : public QTreeView, public KrInterView
{
    friend class KrInterDetailedViewItem;
    Q_OBJECT

public:
    KrInterDetailedView(QWidget *parent, bool &left, KConfig *cfg = krConfig);
    virtual ~KrInterDetailedView();
/*
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
*/
    virtual void restoreSettings();
    virtual void saveSettings();
/*
    virtual void setCurrentItem(const QString& name);
    virtual void setCurrentKrViewItem(KrViewItem *current);
    virtual void sort();
    virtual void clear();
*/
    virtual void updateView();
/*
    virtual void updateItem(KrViewItem* item);
    virtual void updateItem(vfile* item);
    virtual QModelIndex getCurrentIndex() {
        return currentIndex();
    }
    virtual bool isSelected(const QModelIndex &ndx) {
        return selectionModel()->isSelected(ndx);
    }
*/
    virtual void selectRegion(KrViewItem *, KrViewItem *, bool);
//     KrInterDetailedViewItem * getKrInterViewItem(const QModelIndex &);

    static KrView* create(QWidget *parent, bool &left, KConfig *cfg) {
        return new KrInterDetailedView(parent, left, cfg);
    }

//     virtual void prepareForActive();
//     virtual void prepareForPassive();
    virtual bool ensureVisibilityAfterSelect() {
        return false;
    }
    virtual int  itemsPerPage();
    virtual void setSortMode(KrViewProperties::SortSpec mode);
    virtual void setFileIconSize(int size);

// public slots:
//     virtual void refreshColors();

protected slots:
//     void slotMakeCurrentVisible();
    virtual void renameCurrentItem();
    void sectionResized(int, int, int);
    virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous);

protected:
    virtual void setup();
    virtual void initOperator();

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual bool event(QEvent * e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dropEvent(QDropEvent *);
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual bool viewportEvent(QEvent * event);

    void showContextMenu(const QPoint & p);
    void recalculateColumnSizes();

private:
//     KrVfsModel *_model;
//     KrMouseHandler *_mouseHandler;
//     QHash<vfile *, KrInterDetailedViewItem*> _itemHash;
    QFont _viewFont;
};
#endif // __krinterview__
