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
#include "krvfsmodel.h"
#include "krinterviewitem.h"
#include "krcolorcache.h"
#include "krmousehandler.h"
#include "krpreviews.h"
#include "../VFS/vfilecontainer.h"

KrInterView::KrInterView(KrViewInstance &instance, const bool &left, KConfig *cfg,
                         QAbstractItemView *itemView) :
        KrView(instance, left, cfg), _itemView(itemView), _mouseHandler(0), _dummyVfile(0)
{
    _model = new KrVfsModel(this);

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
    _model->clear();
    _model->deleteLater();
    _model = 0;
    delete _mouseHandler;
    _mouseHandler = 0;
    QHashIterator< vfile *, KrInterViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
    delete _dummyVfile;
    _dummyVfile = 0;
}

void KrInterView::selectRegion(KrViewItem *i1, KrViewItem *i2, bool select)
{
    vfile* vf1 = (vfile *)i1->getVfile();
    QModelIndex mi1 = _model->vfileIndex(vf1);
    vfile* vf2 = (vfile *)i2->getVfile();
    QModelIndex mi2 = _model->vfileIndex(vf2);

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
            setSelected(_model->vfileAt(_model->index(row, 0)), select);
        op()->setMassSelectionUpdate(false);

        redraw();

    } else if (mi1.isValid() && !mi2.isValid())
        i1->setSelected(select);
    else if (mi2.isValid() && !mi1.isValid())
        i2->setSelected(select);
}

void KrInterView::setSelected(const vfile* vf, bool select)
{
    if(!select)
        _selected.remove(vf);
    else if(_model->dummyVfile() != vf)
        _selected.insert(vf);
}

bool KrInterView::isSelected(const vfile *vf)
{
    return _selected.contains(vf);
}

bool KrInterView::isSelected(const QModelIndex &ndx)
{
    return isSelected(_model->vfileAt(ndx));
}

KrViewItem* KrInterView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return 0;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return 0;
    return getKrInterViewItem(ndx);
}

QString KrInterView::getCurrentItem() const
{
    if (!_model->ready())
        return QString();

    vfile *vf = _model->vfileAt(_itemView->currentIndex());
    if (vf == 0)
        return QString();
    return vf->vfile_getName();
}

KrViewItem* KrInterView::getCurrentKrViewItem()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_itemView->currentIndex());
}

KrViewItem* KrInterView::getFirst()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterView::getLast()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(_model->rowCount() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getNext(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() >= _model->rowCount() - 1)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getPrev(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() <= 0)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterView::getKrViewItemAt(const QPoint &vp)
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_itemView->indexAt(vp));
}

KrViewItem *KrInterView::findItemByVfile(vfile *vf) {
    return getKrInterViewItem(vf);
}

KrInterViewItem * KrInterView::getKrInterViewItem(vfile *vf)
{
    QHash<vfile *, KrInterViewItem*>::iterator it = _itemHash.find(vf);
    if (it == _itemHash.end()) {
        KrInterViewItem * newItem =  new KrInterViewItem(this, vf);
        _itemHash[ vf ] = newItem;
        return newItem;
    }
    return *it;
}

KrInterViewItem * KrInterView::getKrInterViewItem(const QModelIndex & ndx)
{
    if (!ndx.isValid())
        return 0;
    vfile * vf = _model->vfileAt(ndx);
    if (vf == 0)
        return 0;
    else
        return getKrInterViewItem(vf);
}

void KrInterView::makeCurrentVisible()
{
    _itemView->scrollTo(_itemView->currentIndex());
}

void KrInterView::makeItemVisible(const KrViewItem *item)
{
    if (item == 0)
        return;
    vfile* vf = (vfile *)item->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.isValid())
        _itemView->scrollTo(ndx);
}

void KrInterView::setCurrentItem(const QString& name)
{
    QModelIndex ndx = _model->nameIndex(name);
    if (ndx.isValid())
        _itemView->setCurrentIndex(ndx);
}

void KrInterView::setCurrentKrViewItem(KrViewItem *item)
{
    if (item == 0) {
        _itemView->setCurrentIndex(QModelIndex());
        return;
    }
    vfile* vf = (vfile *)item->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
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
    _selected.clear();
    _itemView->clearSelection();
    _itemView->setCurrentIndex(QModelIndex());
    _model->clear();
    QHashIterator< vfile *, KrInterViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();

    KrView::clear();

    delete _dummyVfile;
    _dummyVfile = 0;
}

void KrInterView::refresh()
{
    QString current = getCurrentItem();

    clear();

    if(!_files)
        return;

    // if we are not at the root add the ".." entery
    if(!_files->isRoot()) {
        _dummyVfile = new vfile("..", 0, "drwxrwxrwx", 0, false, 0, 0, "", "", 0, -1);
        _dummyVfile->vfile_setIcon("go-up");
    }

    _model->populate(_files->vfiles(), _dummyVfile);
    _count = _model->rowCount();

    if (_dummyVfile )
        _count--;

    _itemView->setCurrentIndex(_model->index(0, 0));
    if (!nameToMakeCurrent().isEmpty())
        setCurrentItem(nameToMakeCurrent());
    else if (!current.isEmpty())
        setCurrentItem(current);

    updatePreviews();
    redraw();

    op()->emitSelectionChanged();
}

KrViewItem* KrInterView::preAddItem(vfile *vf)
{
    QModelIndex idx = _model->addItem(vf);
    if(_model->rowCount() == 1) // if this is the fist item to be added, make it current
        _itemView->setCurrentIndex(idx);
    return getKrInterViewItem(idx);
}

void KrInterView::preDelItem(KrViewItem *item)
{
    setSelected(item->getVfile(), false);
    QModelIndex ndx = _model->removeItem((vfile *)item->getVfile());
    if (ndx.isValid())
        _itemView->setCurrentIndex(ndx);
    _itemHash.remove((vfile *)item->getVfile());
}

void KrInterView::preUpdateItem(vfile *vf)
{
    _model->updateItem(vf);
}

void KrInterView::prepareForActive()
{
    KrView::prepareForActive();
    _itemView->setFocus();
    KrViewItem * current = getCurrentKrViewItem();
    if (current != 0) {
        QString desc = current->description();
        op()->emitItemDescription(desc);
    }
}

void KrInterView::prepareForPassive()
{
    KrView::prepareForPassive();
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

void KrInterView::showContextMenu()
{
    showContextMenu(_itemView->viewport()->mapToGlobal(QPoint(0,0)));
}

void KrInterView::sortModeUpdated(int column, Qt::SortOrder order)
{
    KrView::sortModeUpdated(static_cast<KrViewProperties::ColumnType>(column),
                            order == Qt::DescendingOrder);
}
