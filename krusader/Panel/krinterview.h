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

#include <QtCore/QSet>
#include <QtWidgets/QAbstractItemView>

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
    virtual QModelIndex getCurrentIndex() Q_DECL_OVERRIDE {
        return _itemView->currentIndex();
    }
    virtual bool isSelected(const QModelIndex &ndx) Q_DECL_OVERRIDE;
    virtual uint numSelected() const Q_DECL_OVERRIDE {
        return _selection.count();
    }
    virtual QList<QUrl> selectedUrls() Q_DECL_OVERRIDE;
    virtual void setSelection(const QList<QUrl> urls) Q_DECL_OVERRIDE;
    virtual KrViewItem* getFirst() Q_DECL_OVERRIDE;
    virtual KrViewItem* getLast() Q_DECL_OVERRIDE;
    virtual KrViewItem* getNext(KrViewItem *current) Q_DECL_OVERRIDE;
    virtual KrViewItem* getPrev(KrViewItem *current) Q_DECL_OVERRIDE;
    virtual KrViewItem* getCurrentKrViewItem() Q_DECL_OVERRIDE;
    virtual KrViewItem* findItemByName(const QString &name) Q_DECL_OVERRIDE;
    virtual KrViewItem *findItemByVfile(vfile *vf) Q_DECL_OVERRIDE;
    virtual QString getCurrentItem() const Q_DECL_OVERRIDE;
    virtual KrViewItem* getKrViewItemAt(const QPoint &vp) Q_DECL_OVERRIDE;
    virtual void setCurrentItem(const QString& name) Q_DECL_OVERRIDE;
    virtual void setCurrentKrViewItem(KrViewItem *item) Q_DECL_OVERRIDE;
    virtual void makeItemVisible(const KrViewItem *item) Q_DECL_OVERRIDE;
    virtual void clear() Q_DECL_OVERRIDE;
    virtual void sort() Q_DECL_OVERRIDE;
    virtual void refreshColors() Q_DECL_OVERRIDE;
    virtual void redraw() Q_DECL_OVERRIDE;
    virtual void prepareForActive() Q_DECL_OVERRIDE;
    virtual void prepareForPassive() Q_DECL_OVERRIDE;
    virtual void showContextMenu() Q_DECL_OVERRIDE;
    virtual void selectRegion(KrViewItem *i1, KrViewItem *i2, bool select) Q_DECL_OVERRIDE;

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
        virtual void select (const QModelIndex & index, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE { Q_UNUSED(index); Q_UNUSED(command); }
        virtual void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE { Q_UNUSED(selection); Q_UNUSED(command); }
    };

    virtual KIO::filesize_t calcSize() Q_DECL_OVERRIDE;
    virtual KIO::filesize_t calcSelectedSize() Q_DECL_OVERRIDE;
    virtual void populate(const QList<vfile*> &vfiles, vfile *dummy) Q_DECL_OVERRIDE;
    virtual KrViewItem* preAddItem(vfile *vf) Q_DECL_OVERRIDE;
    virtual void preDelItem(KrViewItem *item) Q_DECL_OVERRIDE;
    virtual void preUpdateItem(vfile *vf) Q_DECL_OVERRIDE;
    virtual void intSetSelected(const vfile* vf, bool select) Q_DECL_OVERRIDE;
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
