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
#include <QApplication>

#include <klocale.h>
#include <kdirlister.h>
#include <kmenu.h>

#include "krinterviewitem.h"
#include "krviewfactory.h"
#include "krinterviewitemdelegate.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "krmousehandler.h"
#include "krcolorcache.h"
#include "../GUI/krstyleproxy.h"
#if 0
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
    void redraw() {
        _view->viewport()->update(itemRect());
    }

private:
    vfile *_vfile;
    KrInterDetailedView * _view;
};
#endif

// code used to register the view
#define INTERVIEW_ID 0
KrViewInstance interDetailedView(INTERVIEW_ID,"KrInterDetailedView", i18n("&Detailed View"), "view-list-details",
                                 Qt::ALT + Qt::SHIFT + Qt::Key_D,
                                 KrInterDetailedView::create, KrInterViewItem::itemHeightChanged);
// end of register code

KrInterDetailedView::KrInterDetailedView(QWidget *parent, bool &left, KConfig *cfg, KrMainWindow *mainWindow):
        QTreeView(parent),
        KrInterView(cfg, mainWindow, this),
        _autoResizeColumns(true)
{
    // fix the context menu problem
//     int j = QFontMetrics(font()).height() * 2;
//     _mouseHandler = new KrMouseHandler(this, j);
    connect(_mouseHandler, SIGNAL(renameCurrentItem()), this, SLOT(renameCurrentItem()));
    setWidget(this);
    KConfigGroup group(krConfig, "Private");

    KConfigGroup grpSvr(_config, "Look&Feel");
    _viewFont = grpSvr.readEntry("Filelist Font", *_FilelistFont);

//     _model = new KrVfsModel(this);
    this->setModel(_model);
    this->setRootIsDecorated(false);
    this->setSortingEnabled(true);
    this->sortByColumn(KrVfsModel::Name, Qt::AscendingOrder);
//     _model->sort(KrVfsModel::Name, Qt::AscendingOrder);

    setSelectionModel(new DummySelectionModel(_model, this));

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
//     connect(&KrColorCache::getColorCache(), SIGNAL(colorsRefreshed()), this, SLOT(refreshColors()));
}

KrInterDetailedView::~KrInterDetailedView()
{
    setModel(0);
    delete _properties;
    _properties = 0;
    delete _operator;
    _operator = 0;

//     delete _model;
//     delete _mouseHandler;
//     QHashIterator< vfile *, KrInterDetailedViewItem *> it(_itemHash);
//     while (it.hasNext())
//         delete it.next().value();
//     _itemHash.clear();
}

KrViewInstance* KrInterDetailedView::instance() const
{
    return &interDetailedView;
}

#if 0
KrViewItem* KrInterDetailedView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return 0;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return 0;
    return getKrInterViewItem(ndx);
}
#endif
void KrInterDetailedView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (_model->ready()) {
        KrViewItem * item = getKrInterViewItem(currentIndex());
        op()->emitCurrentChanged(item);
    }
    QTreeView::currentChanged(current, previous);
}
#if 0
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
    viewport()->update();
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
#endif

void KrInterDetailedView::doRestoreSettings(KConfigGroup &grpSvr)
{
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

void KrInterDetailedView::doSaveSettings(KConfigGroup &grpSvr)
{
    QByteArray state = header()->saveState();
    grpSvr.writeEntry("Saved State", state);
}

#if 0
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
#endif
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
#if 0
void KrInterDetailedView::sort()
{
    _model->sort();
}
#endif
void KrInterDetailedView::updateView()
{
}
#if 0
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
#endif
void KrInterDetailedView::setup()
{

}

void KrInterDetailedView::initOperator()
{
    _operator = new KrViewOperator(this, this);
    // klistview emits selection changed, so chain them to operator
//     connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), _operator, SLOT(emitSelectionChanged()));
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
#if 0
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
#endif

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

    QVector<QAction*> actions;

    for(int i = KrVfsModel::Name; i < KrVfsModel::MAX_COLUMNS; i++) {
        QString text = (_model->headerData(i, Qt::Horizontal)).toString();
        QAction *act = popup.addAction(text);
        act->setCheckable(true);
        act->setChecked(!header()->isSectionHidden(i));
        actions.append(act);
    }

    popup.addSeparator();
    QAction *actAutoResize = popup.addAction(i18n("Automatically Resize Columns"));
    actAutoResize->setCheckable(true);
    actAutoResize->setChecked(_autoResizeColumns);

    QAction *res = popup.exec(p);

    if(res == actAutoResize) {
        _autoResizeColumns = !_autoResizeColumns;
        recalculateColumnSizes();
    } else {
        int idx = actions.indexOf(res);
        if(idx < 0)
            return;

        if(header()->isSectionHidden(idx))
            header()->showSection(idx);
        else
            header()->hideSection(idx);

        if(KrVfsModel::Extension == idx)
            _model->setExtensionEnabled(!header()->isSectionHidden(KrVfsModel::Extension));
    }
}

void KrInterDetailedView::sectionResized(int column, int oldSize, int newSize)
{
    // *** taken from dolphin ***
    // If the user changes the size of the headers, the autoresize feature should be
    // turned off. As there is no dedicated interface to find out whether the header
    // section has been resized by the user or by a resize event, another approach is used.
    // Attention: Take care when changing the if-condition to verify that there is no
    // regression in combination with bug 178630 (see fix in comment #8).
    if ((QApplication::mouseButtons() & Qt::LeftButton) && header()->underMouse()) {
        _autoResizeColumns = false;
    }

    if (oldSize == newSize || !_model->ready())
        return;

    recalculateColumnSizes();
}

void KrInterDetailedView::recalculateColumnSizes()
{
    if(!_autoResizeColumns)
        return;
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

void KrInterDetailedView::setFileIconSize(int size)
{
    KrView::setFileIconSize(size);
    setIconSize(QSize(fileIconSize(), fileIconSize()));
}
