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

#include <QSet>
#include <QAbstractItemView>

#include "krview.h"

class KrInterViewItem;
class KrVfsModel;
class KrMouseHandler;
class KrViewItem;

class KrInterView : public KrView
{
    friend class KrInterViewItem;
public:
    KrInterView(KrViewInstance &instance, KConfig *cfg, QAbstractItemView *itemView);
    virtual ~KrInterView();
    virtual QModelIndex getCurrentIndex() {
        return _itemView->currentIndex();
    }
    virtual bool isSelected(const QModelIndex &ndx);
    virtual uint numSelected() const {
        return _selection.count();
    }
    virtual KUrl::List selectedUrls();
    virtual void setSelection(const KUrl::List urls);
    virtual KrViewItem* getFirst();
    virtual KrViewItem* getLast();
    virtual KrViewItem* getNext(KrViewItem *current);
    virtual KrViewItem* getPrev(KrViewItem *current);
    virtual KrViewItem* getCurrentKrViewItem();
    virtual KrViewItem* findItemByName(const QString &name);
    virtual KrViewItem *findItemByVfile(vfile *vf);
    virtual QString getCurrentItem() const;
    virtual KrViewItem* getKrViewItemAt(const QPoint &vp);
    virtual void setCurrentItem(const QString& name);
    virtual void setCurrentKrViewItem(KrViewItem *item);
    virtual void makeItemVisible(const KrViewItem *item);
    virtual void clear();
    virtual void sort();
    virtual void refreshColors();
    virtual void redraw();
    virtual void prepareForActive();
    virtual void prepareForPassive();
    virtual void showContextMenu();
    virtual void selectRegion(KrViewItem *i1, KrViewItem *i2, bool select);

    void sortModeUpdated(int column, Qt::SortOrder order);

    void redrawItem(vfile *vf) {
        _itemView->viewport()->update(itemRect(vf));
    }

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

    virtual KIO::filesize_t calcSize();
    virtual KIO::filesize_t calcSelectedSize();
    virtual void populate(const QList<vfile*> &vfiles, vfile *dummy);
    virtual KrViewItem* preAddItem(vfile *vf);
    virtual void preDelItem(KrViewItem *item);
    virtual void preUpdateItem(vfile *vf);
    virtual void intSetSelected(const vfile* vf, bool select);
    virtual void showContextMenu(const QPoint & p) = 0;

    virtual QRect itemRect(const vfile *vf) = 0;

    KrInterViewItem * getKrInterViewItem(vfile *vf);
    KrInterViewItem * getKrInterViewItem(const QModelIndex &);
    bool isSelected(const vfile *vf) const {
        return _selection.contains(vf);
    }
    void makeCurrentVisible();


    KrVfsModel *_model;
    QAbstractItemView *_itemView;
    KrMouseHandler *_mouseHandler;
    QHash<vfile *, KrInterViewItem*> _itemHash;
    QSet<const vfile*> _selection;
};

#endif
