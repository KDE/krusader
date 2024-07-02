/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krinterview.h"

#include <QDebug>

#include "../FileSystem/dirlisterinterface.h"
#include "../FileSystem/fileitem.h"
#include "../krcolorcache.h"
#include "../krpreviews.h"
#include "krmousehandler.h"
#include "krviewitem.h"
#include "listmodel.h"

KrInterView::KrInterView(KrViewInstance &instance, KConfig *cfg, QAbstractItemView *itemView)
    : KrView(instance, cfg)
    , _itemView(itemView)
    , _mouseHandler(nullptr)
{
    _model = new ListModel(this);

    // fix the context menu problem
    int j = QFontMetrics(_itemView->font()).height() * 2;
    _mouseHandler = new KrMouseHandler(this, j);
}

KrInterView::~KrInterView()
{
    // any references to the model should be cleared ar this point,
    // but sometimes for some reason it is still referenced by
    // QPersistentModelIndex instances held by QAbstractItemView and/or QItemSelectionModel(child object) -
    // so schedule _model for later deletion
    _model->clear(false);
    _model->deleteLater();
    _model = nullptr;
    delete _mouseHandler;
    _mouseHandler = nullptr;
    QHashIterator<FileItem *, KrViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
}

void KrInterView::selectRegion(KrViewItem *i1, KrViewItem *i2, bool select)
{
    auto *file1 = const_cast<FileItem *>(i1->getFileItem());
    QModelIndex mi1 = _model->fileItemIndex(file1);
    auto *file2 = const_cast<FileItem *>(i2->getFileItem());
    QModelIndex mi2 = _model->fileItemIndex(file2);

    if (mi1.isValid() && mi2.isValid()) {
        int r1 = mi1.row();
        int r2 = mi2.row();

        if (r1 > r2) {
            int t = r1;
            r1 = r2;
            r2 = t;
        }

        op()->setMassSelectionUpdate(true);
        for (int row = r1; row <= r2; row++)
            setSelected(_model->fileItemAt(_model->index(row, 0)), select);
        op()->setMassSelectionUpdate(false);

        redraw();

    } else if (mi1.isValid() && !mi2.isValid())
        i1->setSelected(select);
    else if (mi2.isValid() && !mi1.isValid())
        i2->setSelected(select);
}

void KrInterView::intSetSelected(const FileItem *item, bool select)
{
    if (select)
        _selection.insert(item);
    else
        _selection.remove(item);
}

bool KrInterView::isSelected(const QModelIndex &ndx)
{
    return isSelected(_model->fileItemAt(ndx));
}

KrViewItem *KrInterView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return nullptr;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return nullptr;
    return getKrViewItem(ndx);
}

KrViewItem *KrInterView::findItemByUrl(const QUrl &url)
{
    if (!_model->ready())
        return nullptr;

    const QModelIndex ndx = _model->indexFromUrl(url);
    if (!ndx.isValid())
        return nullptr;

    return getKrViewItem(ndx);
}

QString KrInterView::getCurrentItem() const
{
    if (!_model->ready())
        return QString();

    FileItem *fileItem = _model->fileItemAt(_itemView->currentIndex());
    return fileItem ? fileItem->getName() : QString();
}

KrViewItem *KrInterView::getCurrentKrViewItem()
{
    if (!_model->ready())
        return nullptr;

    return getKrViewItem(_itemView->currentIndex());
}

KrViewItem *KrInterView::getFirst()
{
    if (!_model->ready())
        return nullptr;

    return getKrViewItem(_model->index(0, 0, QModelIndex()));
}

KrViewItem *KrInterView::getLast()
{
    if (!_model->ready())
        return nullptr;

    return getKrViewItem(_model->index(_model->rowCount() - 1, 0, QModelIndex()));
}

KrViewItem *KrInterView::getNext(KrViewItem *current)
{
    auto *fileItem = const_cast<FileItem *>(current->getFileItem());
    QModelIndex ndx = _model->fileItemIndex(fileItem);
    if (ndx.row() >= _model->rowCount() - 1)
        return nullptr;
    return getKrViewItem(_model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem *KrInterView::getPrev(KrViewItem *current)
{
    auto *fileItem = const_cast<FileItem *>(current->getFileItem());
    QModelIndex ndx = _model->fileItemIndex(fileItem);
    if (ndx.row() <= 0)
        return nullptr;
    return getKrViewItem(_model->index(ndx.row() - 1, 0, QModelIndex()));
}

KrViewItem *KrInterView::getKrViewItemAt(const QPoint &vp)
{
    if (!_model->ready())
        return nullptr;

    return getKrViewItem(_itemView->indexAt(vp));
}

KrViewItem *KrInterView::getKrViewItem(FileItem *fileItem)
{
    QHash<FileItem *, KrViewItem *>::iterator it = _itemHash.find(fileItem);
    if (it == _itemHash.end()) {
        auto *newItem = new KrViewItem(fileItem, this);
        _itemHash[fileItem] = newItem;
        return newItem;
    }
    return *it;
}

KrViewItem *KrInterView::getKrViewItem(const QModelIndex &ndx)
{
    if (!ndx.isValid())
        return nullptr;
    FileItem *fileitem = _model->fileItemAt(ndx);
    if (fileitem == nullptr)
        return nullptr;
    else
        return getKrViewItem(fileitem);
}

void KrInterView::makeCurrentVisible()
{
    makeItemVisible(getCurrentKrViewItem());
}

void KrInterView::makeItemVisible(const KrViewItem *item)
{
    if (item == nullptr)
        return;

    auto *fileitem = const_cast<FileItem *>(item->getFileItem());
    const QModelIndex index = _model->fileItemIndex(fileitem);
    qDebug() << "scroll to item; name=" << fileitem->getName() << " index=" << index;
    if (index.isValid())
        _itemView->scrollTo(index);
}

bool KrInterView::isItemVisible(const KrViewItem *item)
{
    return item && _itemView->viewport()->rect().contains(item->itemRect());
}

void KrInterView::setCurrentItem(const QString &name, bool scrollToCurrent, const QModelIndex &fallbackToIndex)
{
    // find index by given name and set it as current
    const QModelIndex index = _model->nameIndex(name);
    if (index.isValid()) {
        setCurrent(index, scrollToCurrent);
    } else if (fallbackToIndex.isValid()) {
        // set fallback index as current if not too big, else set the last item as current
        if (fallbackToIndex.row() < _itemView->model()->rowCount()) {
            setCurrent(fallbackToIndex, scrollToCurrent);
        } else {
            setCurrentKrViewItem(getLast(), scrollToCurrent);
        }
    } else {
        // when given parameters fail, set the first item as current
        setCurrentKrViewItem(getFirst(), scrollToCurrent);
    }
}

void KrInterView::setCurrentKrViewItem(KrViewItem *item, bool scrollToCurrent)
{
    if (!item) {
        setCurrent(QModelIndex(), scrollToCurrent);
        return;
    }

    auto *fileitem = const_cast<FileItem *>(item->getFileItem());
    const QModelIndex index = _model->fileItemIndex(fileitem);
    if (index.isValid() && index.row() != _itemView->currentIndex().row()) {
        setCurrent(index, scrollToCurrent);
    }
}

void KrInterView::setCurrent(const QModelIndex &index, bool scrollToCurrent)
{
    const bool disableAutoScroll = _itemView->hasAutoScroll() && !scrollToCurrent;
    if (disableAutoScroll) {
        // setCurrentIndex() scrolls to current if autoScroll is turned on
        _itemView->setAutoScroll(false);
    }

    _mouseHandler->cancelTwoClickRename();
    _itemView->setCurrentIndex(index);

    if (disableAutoScroll) {
        _itemView->setAutoScroll(true);
    }
}

void KrInterView::sort()
{
    _model->sort();
}

void KrInterView::clear()
{
    _selection.clear();
    _itemView->clearSelection();
    _itemView->setCurrentIndex(QModelIndex());
    _model->clear();
    QHashIterator<FileItem *, KrViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();

    KrView::clear();
}

void KrInterView::populate(const QList<FileItem *> &fileItems, FileItem *dummy)
{
    _model->populate(fileItems, dummy);
}

KrViewItem *KrInterView::preAddItem(FileItem *fileitem)
{
    const QModelIndex index = _model->addItem(fileitem);
    return getKrViewItem(index);
}

void KrInterView::preDeleteItem(KrViewItem *item)
{
    // update selection
    setSelected(item->getFileItem(), false);

    // close editor if it's opened for the item
    auto file_item = const_cast<FileItem *>(item->getFileItem());
    _itemView->closePersistentEditor(_model->fileItemIndex(file_item));

    // remove the item from the structures
    _model->removeItem(file_item);
    _itemHash.remove(file_item);
}

void KrInterView::prepareForActive()
{
    _focused = true;
    _itemView->setFocus();
}

void KrInterView::prepareForPassive()
{
    _focused = false;
    _mouseHandler->cancelTwoClickRename();
    // if ( renameLineEdit() ->isVisible() )
    // renameLineEdit() ->clearFocus();
}

void KrInterView::redraw()
{
    _itemView->viewport()->update();
}

void KrInterView::refreshColors()
{
    QPalette p(_itemView->palette());
    KrColorGroup cg;
    KrColorCache::getColorCache().getColors(cg, KrColorItemType(KrColorItemType::File, false, _focused, false, false));
    p.setColor(QPalette::Text, cg.text());
    p.setColor(QPalette::Base, cg.background());
    _itemView->setPalette(p);
    redraw();
}

void KrInterView::sortModeUpdated(int column, Qt::SortOrder order)
{
    KrView::sortModeUpdated(static_cast<KrViewProperties::ColumnType>(column), order == Qt::DescendingOrder);
}

KIO::filesize_t KrInterView::calcSize()
{
    KIO::filesize_t size = 0;
    const auto fileItems = _model->fileItems();
    for (FileItem *fileitem : fileItems) {
        size += fileitem->getSize();
    }
    return size;
}

KIO::filesize_t KrInterView::calcSelectedSize()
{
    KIO::filesize_t size = 0;
    for (const FileItem *fileitem : std::as_const(_selection)) {
        size += fileitem->getSize();
    }
    return size;
}

QList<QUrl> KrInterView::selectedUrls()
{
    QList<QUrl> list;
    for (const FileItem *fileitem : std::as_const(_selection)) {
        list << fileitem->getUrl();
    }
    return list;
}

void KrInterView::setSelectionUrls(const QList<QUrl> urls)
{
    op()->setMassSelectionUpdate(true);

    _selection.clear();

    for (const QUrl &url : urls) {
        const QModelIndex idx = _model->indexFromUrl(url);
        if (idx.isValid())
            setSelected(_model->fileItemAt(idx), true);
    }

    op()->setMassSelectionUpdate(false);
}
