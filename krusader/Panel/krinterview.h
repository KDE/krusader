/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
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

#ifndef KRINTERVIEW_H
#define KRINTERVIEW_H

#include <QVector>
#include <QAbstractItemView>

#include "krview.h"
#include "krviewitem.h"

class KrInterViewItem;
class KrVfsModel;
class KrMouseHandler;

class KrInterView : public KrView
{
    friend class KrInterViewItem;
public:
    KrInterView(KrViewInstance &instance, const bool &left, KConfig *cfg, KrMainWindow *mainWindow, QAbstractItemView *itemView);
    virtual ~KrInterView();
    virtual QModelIndex getCurrentIndex() {
        return _itemView->currentIndex();
    }
    virtual bool isSelected(const QModelIndex &ndx);
    virtual KrViewItem* getFirst();
    virtual KrViewItem* getLast();
    virtual KrViewItem* getNext(KrViewItem *current);
    virtual KrViewItem* getPrev(KrViewItem *current);
    virtual KrViewItem* getCurrentKrViewItem();
    virtual KrViewItem* findItemByName(const QString &name);
    virtual void addItems(vfs* v, bool addUpDir = true);
    virtual QString getCurrentItem() const;
    virtual KrViewItem* getKrViewItemAt(const QPoint &vp);
    virtual void setCurrentItem(const QString& name);
    virtual void setCurrentKrViewItem(KrViewItem *item);
    virtual void makeItemVisible(const KrViewItem *item);
    virtual void clear();
    virtual void updateItem(KrViewItem* item);
    virtual void updateItem(vfile* item);
    virtual void sort();
    virtual void refreshColors();
    virtual void redraw();
    virtual void prepareForActive();
    virtual void prepareForPassive();
    virtual void showContextMenu();
    virtual void selectRegion(KrViewItem *i1, KrViewItem *i2, bool select);

protected:
    class DummySelectionModel : public QItemSelectionModel
    {
    public:
        DummySelectionModel(QAbstractItemModel *model, QObject *parent) :
            QItemSelectionModel(model, parent) {}
        // do nothing - selection is managed by KrInterView
        virtual void select (const QModelIndex & index, QItemSelectionModel::SelectionFlags command) {}
        virtual void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command) {}
    };

    virtual KrViewItem* preAddItem(vfile *vf);
    virtual bool preDelItem(KrViewItem *item);
    virtual void showContextMenu(const QPoint & p) = 0;

    virtual QRect itemRect(const vfile *vf) = 0;

    KrInterViewItem * getKrInterViewItem(const QModelIndex &);
    void setSelected(const vfile* vf, bool select);
    bool isSelected(const vfile *vf);
    void makeCurrentVisible();


    KrVfsModel *_model;
    QAbstractItemView *_itemView;
    KrMouseHandler *_mouseHandler;
    QHash<vfile *, KrInterViewItem*> _itemHash;
    QList<const vfile*> _selected;
};

#endif
