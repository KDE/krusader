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

#include "krinterview.h"

#include <QDebug>

#include "krmousehandler.h"
#include "listmodel.h"
#include "krviewitem.h"
#include "../krpreviews.h"
#include "../krcolorcache.h"
#include "../FileSystem/dirlisterinterface.h"
#include "../FileSystem/fileitem.h"

KrInterView::KrInterView(KrViewInstance &instance, KConfig *cfg,
                         QAbstractItemView *itemView) :
        KrView(instance, cfg), _itemView(itemView), _mouseHandler(0)
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
    _model = 0;
    delete _mouseHandler;
    _mouseHandler = 0;
    QHashIterator< FileItem *, KrViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
}

void KrInterView::selectRegion(KrViewItem *i1, KrViewItem *i2, bool select)
{
    FileItem* file1 = (FileItem *)i1->getFileItem();
    QModelIndex mi1 = _model->fileItemIndex(file1);
    FileItem* file2 = (FileItem *)i2->getFileItem();
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

void KrInterView::intSetSelected(const FileItem* item, bool select)
{
    if(select)
        _selection.insert(item);
    else
        _selection.remove(item);
}

bool KrInterView::isSelected(const QModelIndex &ndx)
{
    return isSelected(_model->fileItemAt(ndx));
}

KrViewItem* KrInterView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return 0;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return 0;
    return getKrViewItem(ndx);
}

KrViewItem *KrInterView::findItemByUrl(const QUrl &url)
{
    if (!_model->ready())
        return 0;

    const QModelIndex ndx = _model->indexFromUrl(url);
    if (!ndx.isValid())
        return 0;

    return getKrViewItem(ndx);
}

QString KrInterView::getCurrentItem() const
{
    if (!_model->ready())
        return QString();

    FileItem *fileitem = _model->fileItemAt(_itemView->currentIndex());
    if (fileitem == 0)
        return QString();
    return fileitem->getName();
}

KrViewItem* KrInterView::getCurrentKrViewItem()
{
    if (!_model->ready())
        return 0;

    return getKrViewItem(_itemView->currentIndex());
}

KrViewItem* KrInterView::getFirst()
{
    if (!_model->ready())
        return 0;

    return getKrViewItem(_model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterView::getLast()
{
    if (!_model->ready())
        return 0;

    return getKrViewItem(_model->index(_model->rowCount() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getNext(KrViewItem *current)
{
    FileItem* fileItem = (FileItem *)current->getFileItem();
    QModelIndex ndx = _model->fileItemIndex(fileItem);
    if (ndx.row() >= _model->rowCount() - 1)
        return 0;
    return getKrViewItem(_model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getPrev(KrViewItem *current)
{
    FileItem* fileItem = (FileItem *)current->getFileItem();
    QModelIndex ndx = _model->fileItemIndex(fileItem);
    if (ndx.row() <= 0)
        return 0;
    return getKrViewItem(_model->index(ndx.row() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getKrViewItemAt(const QPoint &vp)
{
    if (!_model->ready())
        return 0;

    return getKrViewItem(_itemView->indexAt(vp));
}

KrViewItem *KrInterView::findItemByFileItem(FileItem *fileItem) {
    return getKrViewItem(fileItem);
}

KrViewItem * KrInterView::getKrViewItem(FileItem *fileItem)
{
    QHash<FileItem *, KrViewItem*>::iterator it = _itemHash.find(fileItem);
    if (it == _itemHash.end()) {
        KrViewItem * newItem =  new KrViewItem(fileItem, this);
        _itemHash[ fileItem ] = newItem;
        return newItem;
    }
    return *it;
}

KrViewItem * KrInterView::getKrViewItem(const QModelIndex & ndx)
{
    if (!ndx.isValid())
        return 0;
    FileItem * fileitem = _model->fileItemAt(ndx);
    if (fileitem == 0)
        return 0;
    else
        return getKrViewItem(fileitem);
}

void KrInterView::makeCurrentVisible()
{
    qDebug() << "scroll to current index=" << _itemView->currentIndex();
    _itemView->scrollTo(_itemView->currentIndex());
}

void KrInterView::makeItemVisible(const KrViewItem *item)
{
    if (item == 0)
        return;

    FileItem* fileitem = (FileItem *)item->getFileItem();
    QModelIndex ndx = _model->fileItemIndex(fileitem);
    qDebug() << "scroll to item name=" << fileitem->getName();
    if (ndx.isValid())
        _itemView->scrollTo(ndx);
}

bool KrInterView::isItemVisible(const KrViewItem *item)
{
    return _itemView->viewport()->rect().contains(item->itemRect());
}

void KrInterView::setCurrentItem(const QString& name, const QModelIndex &fallbackToIndex)
{
    // find index by given name and set it as current
    QModelIndex ndx = _model->nameIndex(name);
    if (ndx.isValid()) {
         // also sets the scrolling position
        _itemView->setCurrentIndex(ndx);
    } else if (fallbackToIndex.isValid()) {
        // set fallback index as current index
        // when fallback index is too big, set the last item as current
        if (fallbackToIndex.row() >= _itemView->model()->rowCount()) {
            setCurrentKrViewItem(getLast());
        } else {
            _itemView->setCurrentIndex(fallbackToIndex);
        }
    } else {
        // when given parameters fail, set the first item as current
        setCurrentKrViewItem(getFirst());
    }
}

void KrInterView::setCurrentKrViewItem(KrViewItem *item)
{
    if (item == 0) {
        _itemView->setCurrentIndex(QModelIndex());
        return;
    }
    FileItem* fileitem = (FileItem *)item->getFileItem();
    QModelIndex ndx = _model->fileItemIndex(fileitem);
    if (ndx.isValid() && ndx.row() != _itemView->currentIndex().row()) {
        _mouseHandler->cancelTwoClickRename();
        _itemView->setCurrentIndex(ndx);
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
    QHashIterator< FileItem *, KrViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();

    KrView::clear();
}

void KrInterView::populate(const QList<FileItem*> &fileItems, FileItem *dummy)
{
    _model->populate(fileItems, dummy);
}

KrViewItem* KrInterView::preAddItem(FileItem *fileitem)
{
    QModelIndex idx = _model->addItem(fileitem);
    if(_model->rowCount() == 1) // if this is the fist item to be added, make it current
        _itemView->setCurrentIndex(idx);
    return getKrViewItem(idx);
}

void KrInterView::preDelItem(KrViewItem *item)
{
    setSelected(item->getFileItem(), false);
    QModelIndex ndx = _model->removeItem((FileItem *)item->getFileItem());
    if (ndx.isValid())
        _itemView->setCurrentIndex(ndx);
    _itemHash.remove((FileItem *)item->getFileItem());
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
    //if ( renameLineEdit() ->isVisible() )
    //renameLineEdit() ->clearFocus();
}

void KrInterView::redraw()
{
    _itemView->viewport()->update();
}

void KrInterView::refreshColors()
{
    QPalette p(_itemView->palette());
    KrColorGroup cg;
    KrColorCache::getColorCache().getColors(cg, KrColorItemType(KrColorItemType::File,
        false, _focused, false, false));
    p.setColor(QPalette::Text, cg.text());
    p.setColor(QPalette::Base, cg.background());
    _itemView->setPalette(p);
    redraw();
}

void KrInterView::sortModeUpdated(int column, Qt::SortOrder order)
{
    KrView::sortModeUpdated(static_cast<KrViewProperties::ColumnType>(column),
                            order == Qt::DescendingOrder);
}

KIO::filesize_t KrInterView::calcSize()
{
    KIO::filesize_t size = 0;
    foreach(FileItem *fileitem, _model->fileItems()) {
        size += fileitem->getSize();
    }
    return size;
}

KIO::filesize_t KrInterView::calcSelectedSize()
{
    KIO::filesize_t size = 0;
    foreach(const FileItem *fileitem, _selection) {
        size += fileitem->getSize();
    }
    return size;
}

QList<QUrl> KrInterView::selectedUrls()
{
    QList<QUrl> list;
    foreach(const FileItem *fileitem, _selection) {
        list << fileitem->getUrl();
    }
    return list;
}

void KrInterView::setSelectionUrls(const QList<QUrl> urls)
{
    op()->setMassSelectionUpdate(true);

    _selection.clear();

    foreach(const QUrl &url, urls) {
        const QModelIndex idx = _model->indexFromUrl(url);
        if(idx.isValid())
            setSelected(_model->fileItemAt(idx), true);
    }

    op()->setMassSelectionUpdate(false);
}
