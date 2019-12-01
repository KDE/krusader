/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
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
    QModelIndex getCurrentIndex() override {
        return _itemView->currentIndex();
    }
    bool isSelected(const QModelIndex &ndx) override;
    uint numSelected() const override {
        return _selection.count();
    }
    QList<QUrl> selectedUrls() override;
    void setSelectionUrls(const QList<QUrl> urls) override;
    KrViewItem* getFirst() override;
    KrViewItem* getLast() override;
    KrViewItem* getNext(KrViewItem *current) override;
    KrViewItem* getPrev(KrViewItem *current) override;
    KrViewItem* getCurrentKrViewItem() override;
    KrViewItem* findItemByName(const QString &name) override;
    KrViewItem *findItemByUrl(const QUrl &url) override;
    QString getCurrentItem() const override;
    KrViewItem *getKrViewItemAt(const QPoint &vp) override;
    void setCurrentItem(const QString &name, bool scrollToCurrent = true,
                        const QModelIndex &fallbackToIndex = QModelIndex()) override;
    void setCurrentKrViewItem(KrViewItem *item, bool scrollToCurrent = true) override;
    void makeItemVisible(const KrViewItem *item) override;
    bool isItemVisible(const KrViewItem *item) override;
    void clear() override;
    void sort() override;
    void refreshColors() override;
    void redraw() override;
    void prepareForActive() override;
    void prepareForPassive() override;
    void selectRegion(KrViewItem *i1, KrViewItem *i2, bool select) override;

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
        void select(const QModelIndex &, QItemSelectionModel::SelectionFlags) override {}
        void select(const QItemSelection &, QItemSelectionModel::SelectionFlags) override {}
    };

    KIO::filesize_t calcSize() override;
    KIO::filesize_t calcSelectedSize() override;
    void populate(const QList<FileItem*> &fileItems, FileItem *dummy) override;
    KrViewItem* preAddItem(FileItem *fileitem) override;
    /**
     * Remove an item. Does not handle new current selection.
     */
    void preDeleteItem(KrViewItem *item) override;
    void intSetSelected(const FileItem* fileitem, bool select) override;

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
private:
    void setCurrent(const QModelIndex &index, bool scrollToCurrent);
};

#endif
