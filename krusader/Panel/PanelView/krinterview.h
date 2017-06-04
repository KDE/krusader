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

// QtCore
#include <QSet>
// QtWidgets
#include <QAbstractItemView>

#include "krview.h"

class ListModel;
class KrMouseHandler;
class KrViewItem;

/**
 * @brief Abstract intermediate class between KrView and full view implementations.
 *
 * It contains the methods common to all implementing subclasses of KrView.
 */
class KrInterView : public KrView
{
    friend class KrViewItem;
public:
    KrInterView(KrViewInstance &instance, KConfig *cfg, QAbstractItemView *itemView);
    virtual ~KrInterView();
    QModelIndex getCurrentIndex() Q_DECL_OVERRIDE {
        return _itemView->currentIndex();
    }
    bool isSelected(const QModelIndex &ndx) Q_DECL_OVERRIDE;
    uint numSelected() const Q_DECL_OVERRIDE {
        return _selection.count();
    }
    QList<QUrl> selectedUrls() Q_DECL_OVERRIDE;
    void setSelectionUrls(const QList<QUrl> urls) Q_DECL_OVERRIDE;
    KrViewItem* getFirst() Q_DECL_OVERRIDE;
    KrViewItem* getLast() Q_DECL_OVERRIDE;
    KrViewItem* getNext(KrViewItem *current) Q_DECL_OVERRIDE;
    KrViewItem* getPrev(KrViewItem *current) Q_DECL_OVERRIDE;
    KrViewItem* getCurrentKrViewItem() Q_DECL_OVERRIDE;
    KrViewItem* findItemByName(const QString &name) Q_DECL_OVERRIDE;
    KrViewItem *findItemByFileItem(FileItem *fileitem) Q_DECL_OVERRIDE;
    KrViewItem *findItemByUrl(const QUrl &url) Q_DECL_OVERRIDE;
    QString getCurrentItem() const Q_DECL_OVERRIDE;
    KrViewItem* getKrViewItemAt(const QPoint &vp) Q_DECL_OVERRIDE;
    void setCurrentItem(const QString& name, const QModelIndex &fallbackToIndex=QModelIndex()) Q_DECL_OVERRIDE;
    void setCurrentKrViewItem(KrViewItem *item) Q_DECL_OVERRIDE;
    void makeItemVisible(const KrViewItem *item) Q_DECL_OVERRIDE;
    bool isItemVisible(const KrViewItem *item) Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;
    void sort() Q_DECL_OVERRIDE;
    void refreshColors() Q_DECL_OVERRIDE;
    void redraw() Q_DECL_OVERRIDE;
    void prepareForActive() Q_DECL_OVERRIDE;
    void prepareForPassive() Q_DECL_OVERRIDE;
    void selectRegion(KrViewItem *i1, KrViewItem *i2, bool select) Q_DECL_OVERRIDE;

    void sortModeUpdated(int column, Qt::SortOrder order);

    void redrawItem(FileItem *fileitem) {
        _itemView->viewport()->update(itemRect(fileitem));
    }

protected:
    class DummySelectionModel : public QItemSelectionModel
    {
    public:
        DummySelectionModel(QAbstractItemModel *model, QObject *parent) :
            QItemSelectionModel(model, parent) {}
        // do nothing - selection is managed by KrInterView
        void select(const QModelIndex &, QItemSelectionModel::SelectionFlags) Q_DECL_OVERRIDE {}
        void select(const QItemSelection &, QItemSelectionModel::SelectionFlags) Q_DECL_OVERRIDE {}
    };

    KIO::filesize_t calcSize() Q_DECL_OVERRIDE;
    KIO::filesize_t calcSelectedSize() Q_DECL_OVERRIDE;
    void populate(const QList<FileItem*> &fileItems, FileItem *dummy) Q_DECL_OVERRIDE;
    KrViewItem* preAddItem(FileItem *fileitem) Q_DECL_OVERRIDE;
    void preDelItem(KrViewItem *item) Q_DECL_OVERRIDE;
    void intSetSelected(const FileItem* fileitem, bool select) Q_DECL_OVERRIDE;

    virtual QRect itemRect(const FileItem *fileitem) = 0;

    KrViewItem * getKrViewItem(FileItem *fileitem);
    KrViewItem * getKrViewItem(const QModelIndex &);
    bool isSelected(const FileItem *fileitem) const {
        return _selection.contains(fileitem);
    }
    void makeCurrentVisible();

    ListModel *_model;
    QAbstractItemView *_itemView;
    KrMouseHandler *_mouseHandler;
    QHash<FileItem *, KrViewItem*> _itemHash;
    QSet<const FileItem*> _selection;
};

#endif
