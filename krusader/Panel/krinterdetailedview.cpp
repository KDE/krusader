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

#include "krinterdetailedview.h"

#include <QDir>
#include <QDirModel>
#include <QHashIterator>
#include <QHeaderView>

#include <klocale.h>
#include <kdirlister.h>
#include <kmenu.h>

#include "krviewfactory.h"
#include "krinterviewitemdelegate.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "krmousehandler.h"
#include "krcolorcache.h"
#include "../GUI/krstyleproxy.h"

// dummy. remove this class when no longer needed
class KrInterDetailedViewItem: public KrViewItem
{
public:
    KrInterDetailedViewItem(KrInterDetailedView *parent, vfile *vf): KrViewItem(vf, parent->properties()) {
        _view = parent;
        _vfile = vf;
        if (parent->_model->dummyVfile() == vf)
            dummyVfile = true;
    }

    bool isSelected() const {
        const QModelIndex & ndx = _view->_model->vfileIndex(_vfile);
        return _view->selectionModel()->isSelected(ndx);
    }
    void setSelected(bool s) {
        const QModelIndex & ndx = _view->_model->vfileIndex(_vfile);
        _view->selectionModel()->select(ndx, (s ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
                                        | QItemSelectionModel::Rows);
    }
    QRect itemRect() const {
        const QModelIndex & ndx = _view->_model->vfileIndex(_vfile);
        return _view->visualRect(ndx);
    }
    static void itemHeightChanged() {
    } // force the items to resize when icon/font size change
    void redraw() {}

private:
    vfile *_vfile;
    KrInterDetailedView * _view;
};


// code used to register the view
#define INTERVIEW_ID 0
KrViewInstance interDetailedView(INTERVIEW_ID, i18n("&Detailed View"), Qt::ALT + Qt::SHIFT + Qt::Key_D,
                                 KrInterDetailedView::create, KrInterDetailedViewItem::itemHeightChanged);
// end of register code

KrInterDetailedView::KrInterDetailedView(QWidget *parent, bool &left, KConfig *cfg):
        KrView(cfg),
        QTreeView(parent)
{
    // fix the context menu problem
    int j = QFontMetrics(font()).height() * 2;
    _mouseHandler = new KrMouseHandler(this, j);
    connect(_mouseHandler, SIGNAL(renameCurrentItem()), this, SLOT(renameCurrentItem()));
    setWidget(this);
    _nameInKConfig = QString("KrInterDetailedView") + QString((left ? "Left" : "Right")) ;
    KConfigGroup group(krConfig, "Private");

    KConfigGroup grpSvr(_config, "Look&Feel");
    _viewFont = grpSvr.readEntry("Filelist Font", *_FilelistFont);

    _model = new KrVfsModel(this);
    this->setModel(_model);
    this->setRootIsDecorated(false);
    this->setSortingEnabled(true);
    this->sortByColumn(KrVfsModel::Name, Qt::AscendingOrder);
    _model->sort(KrVfsModel::Name, Qt::AscendingOrder);
    header()->installEventFilter(this);

    setSelectionMode(QAbstractItemView::NoSelection);
    setAllColumnsShowFocus(true);

    setStyle(new KrStyleProxy());
    setItemDelegate(new KrInterViewItemDelegate());
    setMouseTracking(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    for (int i = 0; i != KrVfsModel::MAX_COLUMNS; i++)
        header()->setResizeMode(i, QHeaderView::Interactive);
    header()->setStretchLastSection(false);

    restoreSettings();
    connect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(sectionResized(int, int, int)));
    connect(&KrColorCache::getColorCache(), SIGNAL(colorsRefreshed()), this, SLOT(refreshColors()));
}

KrInterDetailedView::~KrInterDetailedView()
{
    delete _properties;
    _properties = 0;
    delete _operator;
    _operator = 0;
    delete _model;
    delete _mouseHandler;
    QHashIterator< vfile *, KrInterDetailedViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
}

KrViewItem* KrInterDetailedView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return 0;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return 0;
    return getKrInterViewItem(ndx);
}

void KrInterDetailedView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (_model->ready()) {
        KrViewItem * item = getKrInterViewItem(currentIndex());
        op()->emitCurrentChanged(item);
    }
    QTreeView::currentChanged(current, previous);
}

QString KrInterDetailedView::getCurrentItem() const
{
    if (!_model->ready())
        return QString();

    vfile * vf = _model->vfileAt(currentIndex());
    if (vf == 0)
        return QString();
    return vf->vfile_getName();
}

KrViewItem* KrInterDetailedView::getCurrentKrViewItem()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(currentIndex());
}

KrViewItem* KrInterDetailedView::getFirst()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterDetailedView::getKrViewItemAt(const QPoint &vp)
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(indexAt(vp));
}

KrViewItem* KrInterDetailedView::getLast()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(_model->rowCount() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterDetailedView::getNext(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() >= _model->rowCount() - 1)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterDetailedView::getPrev(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() <= 0)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() - 1, 0, QModelIndex()));
}

void KrInterDetailedView::slotMakeCurrentVisible()
{
    scrollTo(currentIndex());
}

void KrInterDetailedView::makeItemVisible(const KrViewItem *item)
{
    if (item == 0)
        return;
    vfile* vf = (vfile *)item->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.isValid())
        scrollTo(ndx);
}

void KrInterDetailedView::setCurrentKrViewItem(KrViewItem *item)
{
    if (item == 0) {
        setCurrentIndex(QModelIndex());
        return;
    }
    vfile* vf = (vfile *)item->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.isValid() && ndx.row() != currentIndex().row()) {
        _mouseHandler->cancelTwoClickRename();
        setCurrentIndex(ndx);
    }
}

KrViewItem* KrInterDetailedView::preAddItem(vfile *vf)
{
    return getKrInterViewItem(_model->addItem(vf));
}

bool KrInterDetailedView::preDelItem(KrViewItem *item)
{
    if (item == 0)
        return true;
    QModelIndex ndx = _model->removeItem((vfile *)item->getVfile());
    if (ndx.isValid())
        setCurrentIndex(ndx);
    _itemHash.remove((vfile *)item->getVfile());
    return true;
}

void KrInterDetailedView::redraw()
{
}

void KrInterDetailedView::refreshColors()
{
    QPalette p(palette());
    KrColorGroup cg;
    KrColorCache::getColorCache().getColors(cg, KrColorItemType(KrColorItemType::File,
        false, _focused, false, false));
    p.setColor(QPalette::Text, cg.text());
    p.setColor(QPalette::Base, cg.background());
    setPalette(p);
    viewport()->update();
}

void KrInterDetailedView::restoreSettings()
{
    KConfigGroup grpSvr(krConfig, _nameInKConfig);
    QByteArray savedState = grpSvr.readEntry("Saved State", QByteArray());

    if (savedState.isEmpty()) {
        hideColumn(KrVfsModel::Mime);
        hideColumn(KrVfsModel::Permissions);
        hideColumn(KrVfsModel::Owner);
        hideColumn(KrVfsModel::Group);
        header()->resizeSection(KrVfsModel::Extension, QFontMetrics(_viewFont).width("tar.bz2  "));
        header()->resizeSection(KrVfsModel::KrPermissions, QFontMetrics(_viewFont).width("rwx  "));
        header()->resizeSection(KrVfsModel::Size, QFontMetrics(_viewFont).width("9") * 10);

        QDateTime tmp(QDate(2099, 12, 29), QTime(23, 59));
        QString desc = KGlobal::locale()->formatDateTime(tmp) + "  ";

        header()->resizeSection(KrVfsModel::DateTime, QFontMetrics(_viewFont).width(desc));
    } else {
        header()->restoreState(savedState);
        _model->setExtensionEnabled(!isColumnHidden(KrVfsModel::Extension));
    }
}

void KrInterDetailedView::saveSettings()
{
    QByteArray state = header()->saveState();
    KConfigGroup grpSvr(krConfig, _nameInKConfig);
    grpSvr.writeEntry("Saved State", state);
}

void KrInterDetailedView::setCurrentItem(const QString& name)
{
    QModelIndex ndx = _model->nameIndex(name);
    if (ndx.isValid())
        setCurrentIndex(ndx);
}

void KrInterDetailedView::prepareForActive()
{
    KrView::prepareForActive();
    setFocus();
    KrViewItem * current = getCurrentKrViewItem();
    if (current != 0) {
        QString desc = current->description();
        op()->emitItemDescription(desc);
    }
}

void KrInterDetailedView::prepareForPassive()
{
    KrView::prepareForPassive();
    _mouseHandler->cancelTwoClickRename();
    //if ( renameLineEdit() ->isVisible() )
    //renameLineEdit() ->clearFocus();
}

int KrInterDetailedView::itemsPerPage()
{
    QRect rect = visualRect(currentIndex());
    if (!rect.isValid()) {
        for (int i = 0; i != _model->rowCount(); i++) {
            rect = visualRect(_model->index(i, 0));
            if (rect.isValid())
                break;
        }
    }
    if (!rect.isValid())
        return 0;
    int size = (height() - header()->height()) / rect.height();
    if (size < 0)
        size = 0;
    return size;
}

void KrInterDetailedView::sort()
{
    _model->sort();
}

void KrInterDetailedView::updateView()
{
}

void KrInterDetailedView::updateItem(vfile* item)
{
    if (item == 0)
        return;
    _model->updateItem(item);
    op()->emitSelectionChanged();
}

void KrInterDetailedView::updateItem(KrViewItem* item)
{
    if (item == 0)
        return;
    updateItem((vfile *)item->getVfile());
}

void KrInterDetailedView::clear()
{
    clearSelection();
    _model->clear();
    QHashIterator< vfile *, KrInterDetailedViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
    KrView::clear();
}

void KrInterDetailedView::addItems(vfs* v, bool addUpDir)
{
    _model->setVfs(v, addUpDir);
    _count = _model->rowCount();
    if (addUpDir)
        _count--;

    this->setCurrentIndex(_model->index(0, 0));
    if (!nameToMakeCurrent().isEmpty())
        setCurrentItem(nameToMakeCurrent());
}

void KrInterDetailedView::setup()
{

}

void KrInterDetailedView::initOperator()
{
    _operator = new KrViewOperator(this, this);
    // klistview emits selection changed, so chain them to operator
    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), _operator, SLOT(emitSelectionChanged()));
}

void KrInterDetailedView::keyPressEvent(QKeyEvent *e)
{
    if (!e || !_model->ready())
        return ; // subclass bug
    if (handleKeyEvent(e))    // did the view class handled the event?
        return;
    QTreeView::keyPressEvent(e);
}

void KrInterDetailedView::mousePressEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mousePressEvent(ev))
        QTreeView::mousePressEvent(ev);
}

void KrInterDetailedView::mouseReleaseEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseReleaseEvent(ev))
        QTreeView::mouseReleaseEvent(ev);
}

void KrInterDetailedView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseDoubleClickEvent(ev))
        QTreeView::mouseDoubleClickEvent(ev);
}

void KrInterDetailedView::mouseMoveEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseMoveEvent(ev))
        QTreeView::mouseMoveEvent(ev);
}

void KrInterDetailedView::wheelEvent(QWheelEvent *ev)
{
    if (!_mouseHandler->wheelEvent(ev))
        QTreeView::wheelEvent(ev);
}

void KrInterDetailedView::dragEnterEvent(QDragEnterEvent *ev)
{
    if (!_mouseHandler->dragEnterEvent(ev))
        QTreeView::dragEnterEvent(ev);
}

void KrInterDetailedView::dragMoveEvent(QDragMoveEvent *ev)
{
    QTreeView::dragMoveEvent(ev);
    _mouseHandler->dragMoveEvent(ev);
}

void KrInterDetailedView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    if (!_mouseHandler->dragLeaveEvent(ev))
        QTreeView::dragLeaveEvent(ev);
}

void KrInterDetailedView::dropEvent(QDropEvent *ev)
{
    if (!_mouseHandler->dropEvent(ev))
        QTreeView::dropEvent(ev);
}

bool KrInterDetailedView::event(QEvent * e)
{
    _mouseHandler->otherEvent(e);
    return QTreeView::event(e);
}

KrInterDetailedViewItem * KrInterDetailedView::getKrInterViewItem(const QModelIndex & ndx)
{
    if (!ndx.isValid())
        return 0;
    vfile * vf = _model->vfileAt(ndx);
    if (vf == 0)
        return 0;
    QHash<vfile *, KrInterDetailedViewItem*>::iterator it = _itemHash.find(vf);
    if (it == _itemHash.end()) {
        KrInterDetailedViewItem * newItem =  new KrInterDetailedViewItem(this, vf);
        _itemHash[ vf ] = newItem;
        _dict.insert(vf->vfile_getName(), newItem);
        return newItem;
    }
    return *it;
}

void KrInterDetailedView::selectRegion(KrViewItem *i1, KrViewItem *i2, bool select)
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

        for (int row = r1; row <= r2; row++) {
            const QModelIndex & ndx = _model->index(row, 0);
            selectionModel()->select(ndx, (select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
                                     | QItemSelectionModel::Rows);
        }
    } else if (mi1.isValid() && !mi2.isValid())
        i1->setSelected(select);
    else if (mi2.isValid() && !mi1.isValid())
        i2->setSelected(select);
}

void KrInterDetailedView::renameCurrentItem()
{
    QModelIndex cIndex = currentIndex();
    QModelIndex nameIndex = _model->index(cIndex.row(), KrVfsModel::Name);
    edit(nameIndex);
    updateEditorData();
    update(nameIndex);
}

bool KrInterDetailedView::eventFilter(QObject *object, QEvent *event)
{
    if (object == header()) {
        if (event->type() == QEvent::ContextMenu) {
            QContextMenuEvent *me = (QContextMenuEvent *)event;
            showContextMenu(me->globalPos());
            return true;
        } else if (event->type() == QEvent::Resize) {
            recalculateColumnSizes();
            return false;
        }
    }
    return false;
}

void KrInterDetailedView::showContextMenu(const QPoint & p)
{
    KMenu popup(this);
    popup.setTitle(i18n("Columns"));

    bool hasExtension = !isColumnHidden(KrVfsModel::Extension);
    bool hasMime      = !isColumnHidden(KrVfsModel::Mime);
    bool hasSize      = !isColumnHidden(KrVfsModel::Size);
    bool hasDate      = !isColumnHidden(KrVfsModel::DateTime);
    bool hasPerms     = !isColumnHidden(KrVfsModel::Permissions);
    bool hasKrPerms   = !isColumnHidden(KrVfsModel::KrPermissions);
    bool hasOwner     = !isColumnHidden(KrVfsModel::Owner);
    bool hasGroup     = !isColumnHidden(KrVfsModel::Group);

    QAction *extAct = popup.addAction(i18n("Ext"));
    extAct->setCheckable(true);
    extAct->setChecked(hasExtension);

    QAction *typeAct = popup.addAction(i18n("Type"));
    typeAct->setCheckable(true);
    typeAct->setChecked(hasMime);

    QAction *sizeAct = popup.addAction(i18n("Size"));
    sizeAct->setCheckable(true);
    sizeAct->setChecked(hasSize);

    QAction *modifAct = popup.addAction(i18n("Modified"));
    modifAct->setCheckable(true);
    modifAct->setChecked(hasDate);

    QAction *permAct = popup.addAction(i18n("Perms"));
    permAct->setCheckable(true);
    permAct->setChecked(hasPerms);

    QAction *rwxAct = popup.addAction(i18n("rwx"));
    rwxAct->setCheckable(true);
    rwxAct->setChecked(hasKrPerms);

    QAction *ownerAct = popup.addAction(i18n("Owner"));
    ownerAct->setCheckable(true);
    ownerAct->setChecked(hasOwner);

    QAction *groupAct = popup.addAction(i18n("Group"));
    groupAct->setCheckable(true);
    groupAct->setChecked(hasGroup);

    QAction *res = popup.exec(p);
    if (res == extAct) {
        _model->setExtensionEnabled(!hasExtension);
        if (hasExtension)
            hideColumn(KrVfsModel::Extension);
        else
            showColumn(KrVfsModel::Extension);
    } else if (res == typeAct) {
        _model->setExtensionEnabled(!hasMime);
        if (hasMime)
            hideColumn(KrVfsModel::Mime);
        else
            showColumn(KrVfsModel::Mime);
    } else if (res == sizeAct) {
        _model->setExtensionEnabled(!hasSize);
        if (hasSize)
            hideColumn(KrVfsModel::Size);
        else
            showColumn(KrVfsModel::Size);
    } else if (res == modifAct) {
        _model->setExtensionEnabled(!hasDate);
        if (hasDate)
            hideColumn(KrVfsModel::DateTime);
        else
            showColumn(KrVfsModel::DateTime);
    } else if (res == permAct) {
        _model->setExtensionEnabled(!hasPerms);
        if (hasPerms)
            hideColumn(KrVfsModel::Permissions);
        else
            showColumn(KrVfsModel::Permissions);
    } else if (res == rwxAct) {
        _model->setExtensionEnabled(!hasKrPerms);
        if (hasKrPerms)
            hideColumn(KrVfsModel::KrPermissions);
        else
            showColumn(KrVfsModel::KrPermissions);
    } else if (res == ownerAct) {
        _model->setExtensionEnabled(!hasOwner);
        if (hasOwner)
            hideColumn(KrVfsModel::Owner);
        else
            showColumn(KrVfsModel::Owner);
    } else if (res == groupAct) {
        _model->setExtensionEnabled(!hasGroup);
        if (hasGroup)
            hideColumn(KrVfsModel::Group);
        else
            showColumn(KrVfsModel::Group);
    }
}

void KrInterDetailedView::sectionResized(int column, int oldSize, int newSize)
{
    if (oldSize == newSize || !_model->ready())
        return;

    recalculateColumnSizes();
}

void KrInterDetailedView::recalculateColumnSizes()
{
    int sum = 0;
    for (int i = 0; i != KrVfsModel::MAX_COLUMNS; i++) {
        if (!isColumnHidden(i))
            sum += header()->sectionSize(i);
    }

    if (sum != header()->width()) {
        int delta = sum - header()->width();
        int nameSize = header()->sectionSize(KrVfsModel::Name);
        if (nameSize - delta > 20)
            header()->resizeSection(KrVfsModel::Name, nameSize - delta);
    }
}

bool KrInterDetailedView::viewportEvent(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(he->pos());

        if (index.isValid()) {
            int width = header()->sectionSize(index.column());
            QString text = index.data(Qt::DisplayRole).toString();

            int textWidth = QFontMetrics(_viewFont).width(text);

            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            textWidth += 2 * textMargin;

            QVariant decor = index.data(Qt::DecorationRole);
            if (decor.isValid() && decor.type() == QVariant::Pixmap) {
                QPixmap p = decor.value<QPixmap>();
                textWidth += p.width() + 2 * textMargin;
            }

            if (textWidth <= width) {
                event->accept();
                return true;
            }
        }
    }
    return QTreeView::viewportEvent(event);
}

void KrInterDetailedView::setSortMode(KrViewProperties::SortSpec mode)
{
    Qt::SortOrder sortDir;
    int column = _model->convertSortOrderFromKrViewProperties(mode, sortDir);
    if (column == _model->getLastSortOrder() && sortDir == _model->getLastSortDir())
        sortDir = (sortDir == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->sortByColumn(column, sortDir);
}
