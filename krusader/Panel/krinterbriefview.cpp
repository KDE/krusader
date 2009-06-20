/*****************************************************************************
 * Copyright (C) 2009 Csaba Karai <cskarai@freemail.hu>                      *
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

#include "krinterbriefview.h"

#include <QDir>
#include <QDirModel>
#include <QHashIterator>
#include <QHeaderView>
#include <QPainter>
#include <QScrollBar>
#include <QRegion>
#include <QItemSelection>
#include <QItemSelectionRange>

#include <kmenu.h>
#include <klocale.h>
#include <kdirlister.h>

#include "krviewfactory.h"
#include "krinterviewitemdelegate.h"
#include "krviewitem.h"
#include "krvfsmodel.h"
#include "krmousehandler.h"
#include "krcolorcache.h"
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "../GUI/krstyleproxy.h"

// dummy. remove this class when no longer needed
class KrInterBriefViewItem: public KrViewItem
{
public:
    KrInterBriefViewItem(KrInterBriefView *parent, vfile *vf): KrViewItem(vf, parent->properties()) {
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
    KrInterBriefView * _view;
};


// code used to register the view
#define INTERBRIEFVIEW_ID 1
KrViewInstance interBriefView(INTERBRIEFVIEW_ID, i18n("&Brief View"), Qt::ALT + Qt::SHIFT + Qt::Key_B,
                              KrInterBriefView::create, KrInterBriefViewItem::itemHeightChanged);
// end of register code

KrInterBriefView::KrInterBriefView(QWidget *parent, bool &left, KConfig *cfg):
        KrView(cfg),
        QAbstractItemView(parent)
{
    _header = 0;

    // fix the context menu problem
    int j = QFontMetrics(font()).height() * 2;
    _mouseHandler = new KrMouseHandler(this, j);
    connect(_mouseHandler, SIGNAL(renameCurrentItem()), this, SLOT(renameCurrentItem()));
    setWidget(this);
    _nameInKConfig = QString("KrInterBriefView") + QString((left ? "Left" : "Right")) ;
    KConfigGroup group(krConfig, "Private");

    KConfigGroup grpSvr(_config, "Look&Feel");
    _viewFont = grpSvr.readEntry("Filelist Font", *_FilelistFont);
    _fileIconSize = (grpSvr.readEntry("Filelist Icon Size", _FilelistIconSize)).toInt();

    _model = new KrVfsModel(this);
    this->setModel(_model);
    _model->sort(KrVfsModel::Name, Qt::AscendingOrder);
    _model->setExtensionEnabled(false);
    _model->setAlternatingTable(true);
    //header()->installEventFilter( this );

    setSelectionMode(QAbstractItemView::NoSelection);

    setStyle(new KrStyleProxy());
    setItemDelegate(new KrInterViewItemDelegate());
    setMouseTracking(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(&KrColorCache::getColorCache(), SIGNAL(colorsRefreshed()), this, SLOT(refreshColors()));
}

KrInterBriefView::~KrInterBriefView()
{
    delete _properties;
    _properties = 0;
    delete _operator;
    _operator = 0;
    delete _model;
    delete _mouseHandler;
    QHashIterator< vfile *, KrInterBriefViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
}

KrViewItem* KrInterBriefView::findItemByName(const QString &name)
{
    if (!_model->ready())
        return 0;

    QModelIndex ndx = _model->nameIndex(name);
    if (!ndx.isValid())
        return 0;
    return getKrInterViewItem(ndx);
}

void KrInterBriefView::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (_model->ready()) {
        KrViewItem * item = getKrInterViewItem(currentIndex());
        op()->emitCurrentChanged(item);
    }
    QAbstractItemView::currentChanged(current, previous);
}

QString KrInterBriefView::getCurrentItem() const
{
    if (!_model->ready())
        return QString();

    vfile * vf = _model->vfileAt(currentIndex());
    if (vf == 0)
        return QString();
    return vf->vfile_getName();
}

KrViewItem* KrInterBriefView::getCurrentKrViewItem()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(currentIndex());
}

KrViewItem* KrInterBriefView::getFirst()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(0, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getKrViewItemAt(const QPoint &vp)
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(indexAt(vp));
}

KrViewItem* KrInterBriefView::getLast()
{
    if (!_model->ready())
        return 0;

    return getKrInterViewItem(_model->index(_model->rowCount() - 1, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getNext(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() >= _model->rowCount() - 1)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() + 1, 0, QModelIndex()));
}

KrViewItem* KrInterBriefView::getPrev(KrViewItem *current)
{
    vfile* vf = (vfile *)current->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.row() <= 0)
        return 0;
    return getKrInterViewItem(_model->index(ndx.row() - 1, 0, QModelIndex()));
}

void KrInterBriefView::slotMakeCurrentVisible()
{
    scrollTo(currentIndex());
}

void KrInterBriefView::makeItemVisible(const KrViewItem *item)
{
    if (item == 0)
        return;
    vfile* vf = (vfile *)item->getVfile();
    QModelIndex ndx = _model->vfileIndex(vf);
    if (ndx.isValid())
        scrollTo(ndx);
}

void KrInterBriefView::setCurrentKrViewItem(KrViewItem *item)
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

KrViewItem* KrInterBriefView::preAddItem(vfile *vf)
{
    return getKrInterViewItem(_model->addItem(vf));
}

bool KrInterBriefView::preDelItem(KrViewItem *item)
{
    if (item == 0)
        return true;
    QModelIndex ndx = _model->removeItem((vfile *)item->getVfile());
    if (ndx.isValid())
        setCurrentIndex(ndx);
    _itemHash.remove((vfile *)item->getVfile());
    return true;
}

void KrInterBriefView::redraw()
{
}

void KrInterBriefView::refreshColors()
{
    if (_model->rowCount() != 0)
        _model->emitChanged();
}

void KrInterBriefView::restoreSettings()
{
    _numOfColumns = _properties->numberOfColumns;

    KConfigGroup group(krConfig, nameInKConfig());

    int column = group.readEntry("Sort Indicator Column", (int)KrVfsModel::Name);
    bool isAscending = group.readEntry("Ascending Sort Order", true);
    Qt::SortOrder sortDir = isAscending ? Qt::AscendingOrder : Qt::DescendingOrder;

    _header->setSortIndicator(column, sortDir);
    _model->sort(column, sortDir);
}

void KrInterBriefView::saveSettings()
{
    KConfigGroup group(krConfig, nameInKConfig());

    group.writeEntry("Sort Indicator Column", (int)_model->getLastSortOrder());
    group.writeEntry("Ascending Sort Order", (_model->getLastSortDir() == Qt::AscendingOrder));
}

void KrInterBriefView::setCurrentItem(const QString& name)
{
    QModelIndex ndx = _model->nameIndex(name);
    if (ndx.isValid())
        setCurrentIndex(ndx);
}

void KrInterBriefView::prepareForActive()
{
    KrView::prepareForActive();
    setFocus();
    KrViewItem * current = getCurrentKrViewItem();
    if (current != 0) {
        QString desc = current->description();
        op()->emitItemDescription(desc);
    }
}

void KrInterBriefView::prepareForPassive()
{
    KrView::prepareForPassive();
    _mouseHandler->cancelTwoClickRename();
    //if ( renameLineEdit() ->isVisible() )
    //renameLineEdit() ->clearFocus();
}

int KrInterBriefView::itemsPerPage()
{
    int height = getItemHeight();
    if (height == 0)
        height ++;
    int numRows = viewport()->height() / height;
    return numRows;
}

void KrInterBriefView::sort()
{
    _model->sort();
}

void KrInterBriefView::updateView()
{
}

void KrInterBriefView::updateItem(KrViewItem* item)
{
    if (item == 0)
        return;
    _model->updateItem((vfile *)item->getVfile());
}

void KrInterBriefView::clear()
{
    clearSelection();
    _model->clear();
    QHashIterator< vfile *, KrInterBriefViewItem *> it(_itemHash);
    while (it.hasNext())
        delete it.next().value();
    _itemHash.clear();
    KrView::clear();
}

void KrInterBriefView::addItems(vfs* v, bool addUpDir)
{
    _model->setVfs(v, addUpDir);
    _count = _model->rowCount();
    if (addUpDir)
        _count--;

    this->setCurrentIndex(_model->index(0, 0));
    if (!nameToMakeCurrent().isEmpty())
        setCurrentItem(nameToMakeCurrent());
}

void KrInterBriefView::setup()
{
    _header = new QHeaderView(Qt::Horizontal, this);
    _header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _header->setParent(this);
    _header->setModel(_model);
    _header->hideSection(KrVfsModel::Mime);
    _header->hideSection(KrVfsModel::Permissions);
    _header->hideSection(KrVfsModel::KrPermissions);
    _header->hideSection(KrVfsModel::Owner);
    _header->hideSection(KrVfsModel::Group);
    _header->setStretchLastSection(true);
    _header->setResizeMode(QHeaderView::Fixed);
    _header->setClickable(true);
    _header->setSortIndicatorShown(true);
    _header->setSortIndicator(KrVfsModel::Name, Qt::AscendingOrder);
    connect(_header, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
            _model, SLOT(sort(int, Qt::SortOrder)));
    _header->installEventFilter(this);

    restoreSettings();
}

void KrInterBriefView::initOperator()
{
    _operator = new KrViewOperator(this, this);
    // klistview emits selection changed, so chain them to operator
    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), _operator, SLOT(emitSelectionChanged()));
}

void KrInterBriefView::keyPressEvent(QKeyEvent *e)
{
    if (!e || !_model->ready())
        return ; // subclass bug
    if ((e->key() != Qt::Key_Left && e->key() != Qt::Key_Right) &&
            (handleKeyEvent(e)))      // did the view class handled the event?
        return;
    switch (e->key()) {
    case Qt::Key_Right : {
        if (e->modifiers() == Qt::ControlModifier) {   // let the panel handle it
            e->ignore();
            break;
        }
        KrViewItem *i = getCurrentKrViewItem();
        KrViewItem *newCurrent = i;

        if (!i)
            break;

        int num = itemsPerPage() + 1;

        if (e->modifiers() & Qt::ShiftModifier) i->setSelected(!i->isSelected());

        while (i && num > 0) {
            if (e->modifiers() & Qt::ShiftModifier) i->setSelected(!i->isSelected());
            newCurrent = i;
            i = getNext(i);
            num--;
        }

        if (newCurrent) {
            setCurrentKrViewItem(newCurrent);
            slotMakeCurrentVisible();
        }
        if (e->modifiers() & Qt::ShiftModifier)
            op()->emitSelectionChanged();
        break;
    }
    case Qt::Key_Left : {
        if (e->modifiers() == Qt::ControlModifier) {   // let the panel handle it
            e->ignore();
            break;
        }
        KrViewItem *i = getCurrentKrViewItem();
        KrViewItem *newCurrent = i;

        if (!i)
            break;

        int num = itemsPerPage() + 1;

        if (e->modifiers() & Qt::ShiftModifier) i->setSelected(!i->isSelected());

        while (i && num > 0) {
            if (e->modifiers() & Qt::ShiftModifier) i->setSelected(!i->isSelected());
            newCurrent = i;
            i = getPrev(i);
            num--;
        }

        if (newCurrent) {
            setCurrentKrViewItem(newCurrent);
            slotMakeCurrentVisible();
        }
        if (e->modifiers() & Qt::ShiftModifier)
            op()->emitSelectionChanged();
        break;
    }
    default:
        QAbstractItemView::keyPressEvent(e);
    }
}

void KrInterBriefView::mousePressEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mousePressEvent(ev))
        QAbstractItemView::mousePressEvent(ev);
}

void KrInterBriefView::mouseReleaseEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseReleaseEvent(ev))
        QAbstractItemView::mouseReleaseEvent(ev);
}

void KrInterBriefView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseDoubleClickEvent(ev))
        QAbstractItemView::mouseDoubleClickEvent(ev);
}

void KrInterBriefView::mouseMoveEvent(QMouseEvent * ev)
{
    if (!_mouseHandler->mouseMoveEvent(ev))
        QAbstractItemView::mouseMoveEvent(ev);
}

void KrInterBriefView::wheelEvent(QWheelEvent *ev)
{
    if (!_mouseHandler->wheelEvent(ev))
        QAbstractItemView::wheelEvent(ev);
}

void KrInterBriefView::dragEnterEvent(QDragEnterEvent *ev)
{
    if (!_mouseHandler->dragEnterEvent(ev))
        QAbstractItemView::dragEnterEvent(ev);
}

void KrInterBriefView::dragMoveEvent(QDragMoveEvent *ev)
{
    QAbstractItemView::dragMoveEvent(ev);
    _mouseHandler->dragMoveEvent(ev);
}

void KrInterBriefView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    if (!_mouseHandler->dragLeaveEvent(ev))
        QAbstractItemView::dragLeaveEvent(ev);
}

void KrInterBriefView::dropEvent(QDropEvent *ev)
{
    if (!_mouseHandler->dropEvent(ev))
        QAbstractItemView::dropEvent(ev);
}

bool KrInterBriefView::event(QEvent * e)
{
    _mouseHandler->otherEvent(e);
    return QAbstractItemView::event(e);
}

KrInterBriefViewItem * KrInterBriefView::getKrInterViewItem(const QModelIndex & ndx)
{
    if (!ndx.isValid())
        return 0;
    vfile * vf = _model->vfileAt(ndx);
    if (vf == 0)
        return 0;
    QHash<vfile *, KrInterBriefViewItem*>::iterator it = _itemHash.find(vf);
    if (it == _itemHash.end()) {
        KrInterBriefViewItem * newItem =  new KrInterBriefViewItem(this, vf);
        _itemHash[ vf ] = newItem;
        _dict.insert(vf->vfile_getName(), newItem);
        return newItem;
    }
    return *it;
}

void KrInterBriefView::selectRegion(KrViewItem *i1, KrViewItem *i2, bool select)
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

void KrInterBriefView::renameCurrentItem()
{
    QModelIndex cIndex = currentIndex();
    QModelIndex nameIndex = _model->index(cIndex.row(), KrVfsModel::Name);
    edit(nameIndex);
    updateEditorData();
    update(nameIndex);
}

bool KrInterBriefView::eventFilter(QObject *object, QEvent *event)
{
    if (object == _header) {
        if (event->type() == QEvent::ContextMenu) {
            QContextMenuEvent *me = (QContextMenuEvent *)event;
            showContextMenu(me->globalPos());
            return true;
        }
    }
    return false;
}

void KrInterBriefView::showContextMenu(const QPoint & p)
{
    KMenu popup(this);
    popup.setTitle(i18n("Columns"));

    int COL_ID = 14700;

    for (int i = 1; i <= MAX_BRIEF_COLS; i++) {
        QAction *act = popup.addAction(QString("%1").arg(i));
        act->setData(QVariant(COL_ID + i));
        act->setCheckable(true);
        act->setChecked(properties()->numberOfColumns == i);
    }

    QAction * res = popup.exec(p);
    int result = -1;
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    KConfigGroup group(krConfig, nameInKConfig());

    if (result > COL_ID && result <= COL_ID + MAX_BRIEF_COLS) {
        group.writeEntry("Number Of Brief Columns", result - COL_ID);
        _properties->numberOfColumns = result - COL_ID;
        _numOfColumns = result - COL_ID;
        updateGeometries();
    }
}

bool KrInterBriefView::viewportEvent(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(he->pos());

        if (index.isValid()) {
            int width = visualRect(index).width();
            int textWidth = elementWidth(index);

            if (textWidth <= width) {
                event->accept();
                return true;
            }
        }
    }
    return QAbstractItemView::viewportEvent(event);
}


QRect KrInterBriefView::visualRect(const QModelIndex&ndx) const
{
    int width = (viewport()->width()) / _numOfColumns;
    if ((viewport()->width()) % _numOfColumns)
        width++;
    int height = getItemHeight();
    int numRows = viewport()->height() / height;
    if (numRows == 0)
        numRows++;
    int x = width * (ndx.row() / numRows);
    int y = height * (ndx.row() % numRows);
    return mapToViewport(QRect(x, y, width, height));
}

void KrInterBriefView::scrollTo(const QModelIndex &ndx, QAbstractItemView::ScrollHint hint)
{
    const QRect rect = visualRect(ndx);
    if (hint == EnsureVisible && viewport()->rect().contains(rect)) {
        setDirtyRegion(rect);
        return;
    }

    const QRect area = viewport()->rect();

    const bool leftOf = rect.left() < area.left();
    const bool rightOf = rect.right() > area.right();

    int horizontalValue = horizontalScrollBar()->value();

    if (leftOf)
        horizontalValue -= area.left() - rect.left();
    else if (rightOf)
        horizontalValue += rect.right() - area.right();

    horizontalScrollBar()->setValue(horizontalValue);
}

QModelIndex KrInterBriefView::indexAt(const QPoint& p) const
{
    int x = p.x() + horizontalOffset();
    int y = p.y() + verticalOffset();

    int width = (viewport()->width()) / _numOfColumns;
    if ((viewport()->width()) % _numOfColumns)
        width++;
    int height = getItemHeight();
    int numRows = viewport()->height() / height;
    if (numRows == 0)
        numRows++;
    int ys = y / height;
    int xs = (x / width) * numRows;

    return _model->index(xs + ys, 0);
}

QModelIndex KrInterBriefView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers)
{
    if (_model->rowCount() == 0)
        return QModelIndex();

    QModelIndex current = currentIndex();
    if (!current.isValid())
        return _model->index(0, 0);

    switch (cursorAction) {
    case MoveLeft:
    case MovePageDown: {
        int newRow = current.row() - itemsPerPage();
        if (newRow < 0)
            newRow = 0;
        return _model->index(newRow, 0);
    }
    case MoveRight:
    case MovePageUp: {
        int newRow = current.row() + itemsPerPage();
        if (newRow >= _model->rowCount())
            newRow = _model->rowCount() - 1;
        return _model->index(newRow, 0);
    }
    case MovePrevious:
    case MoveUp: {
        int newRow = current.row() - 1;
        if (newRow < 0)
            newRow = 0;
        return _model->index(newRow, 0);
    }
    case MoveNext:
    case MoveDown: {
        int newRow = current.row() + 1;
        if (newRow >= _model->rowCount())
            newRow = _model->rowCount() - 1;
        return _model->index(newRow, 0);
    }
    case MoveHome:
        return _model->index(0, 0);
    case MoveEnd:
        return _model->index(_model->rowCount() - 1, 0);
    }

    return current;
}

int KrInterBriefView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int KrInterBriefView::verticalOffset() const
{
    return 0;
}

bool KrInterBriefView::isIndexHidden(const QModelIndex&ndx) const
{
    return ndx.column() != 0;
}

void KrInterBriefView::setSelection(const QRect&, QFlags<QItemSelectionModel::SelectionFlag>)
{
    /* Don't do anything, selections are handled by the mouse handler and not by QAbstractItemView */
}

QRegion KrInterBriefView::visualRegionForSelection(const QItemSelection &selection) const
{
    if (selection.isEmpty())
        return QRegion();

    QRegion selectionRegion;
    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange range = selection.at(i);
        if (!range.isValid())
            continue;
        QModelIndex leftIndex = range.topLeft();
        if (!leftIndex.isValid())
            continue;
        const QRect leftRect = visualRect(leftIndex);
        int top = leftRect.top();
        QModelIndex rightIndex = range.bottomRight();
        if (!rightIndex.isValid())
            continue;
        const QRect rightRect = visualRect(rightIndex);
        int bottom = rightRect.bottom();
        if (top > bottom)
            qSwap<int>(top, bottom);
        int height = bottom - top + 1;
        QRect combined = leftRect | rightRect;
        combined.setX(range.left());
        selectionRegion += combined;
    }
    return selectionRegion;
}

void KrInterBriefView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItemV4 option = viewOptions();
    option.widget = this;
    option.decorationSize = QSize(_fileIconSize, _fileIconSize);
    option.decorationPosition = QStyleOptionViewItem::Left;
    QPainter painter(viewport());

    QModelIndex curr = currentIndex();

    QVector<QModelIndex> intersectVector;
    QRect area = e->rect();
    area.adjust(horizontalOffset(), verticalOffset(), horizontalOffset(), verticalOffset());
    intersectionSet(area, intersectVector);

    foreach(const QModelIndex &mndx, intersectVector) {
        option.state = QStyle::State_None;
        option.rect = visualRect(mndx);
        painter.save();

        bool focus = curr.isValid() && curr.row() == mndx.row() && hasFocus();

        itemDelegate()->paint(&painter, option, mndx);

        if (focus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(option);
            QPalette::ColorGroup cg = QPalette::Normal;
            o.backgroundColor = option.palette.color(cg, QPalette::Background);
            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, &painter);
        }

        painter.restore();
    }
}

int KrInterBriefView::getItemHeight() const
{
    int textHeight = QFontMetrics(_viewFont).height();
    int height = textHeight;
    int iconSize = 0;
    if (properties()->displayIcons)
        iconSize = _fileIconSize;
    if (iconSize > textHeight)
        height = iconSize;
    if (height == 0)
        height++;
    return height;
}

void KrInterBriefView::updateGeometries()
{
    if (_header) {
        QSize hint = _header->sizeHint();
        setViewportMargins(0, hint.height(), 0, 0);
        QRect vg = viewport()->geometry();
        QRect geometryRect(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());
        _header->setGeometry(geometryRect);

        int items = 0;
        for (int i = 0;  i != _header->count(); i++)
            if (!_header->isSectionHidden(i))
                items++;
        if (items == 0)
            items++;

        int sectWidth = viewport()->width() / items;
        for (int i = 0;  i != _header->count(); i++)
            if (!_header->isSectionHidden(i))
                _header->resizeSection(i, sectWidth);

        QMetaObject::invokeMethod(_header, "updateGeometries");
    }



    if (_model->rowCount() <= 0)
        horizontalScrollBar()->setRange(0, 0);
    else {
        int itemsPerColumn = viewport()->height() / getItemHeight();
        if (itemsPerColumn <= 0)
            itemsPerColumn = 1;
        int columnWidth = (viewport()->width()) / _numOfColumns;
        if ((viewport()->width()) % _numOfColumns)
            columnWidth++;
        int maxWidth = _model->rowCount() / itemsPerColumn;
        if (_model->rowCount() % itemsPerColumn)
            maxWidth++;
        maxWidth *= columnWidth;
        if (maxWidth > viewport()->width()) {
            horizontalScrollBar()->setSingleStep(columnWidth);
            horizontalScrollBar()->setPageStep(columnWidth * _numOfColumns);
            horizontalScrollBar()->setRange(0, maxWidth - viewport()->width());
        } else {
            horizontalScrollBar()->setRange(0, 0);
        }
    }

    QAbstractItemView::updateGeometries();
}

QRect KrInterBriefView::mapToViewport(const QRect &rect) const
{
    if (!rect.isValid())
        return rect;

    QRect result = rect;

    int dx = -horizontalOffset();
    int dy = -verticalOffset();
    result.adjust(dx, dy, dx, dy);
    return result;
}


void KrInterBriefView::setSortMode(KrViewProperties::SortSpec mode)
{
    Qt::SortOrder sortDir;
    int column = _model->convertSortOrderFromKrViewProperties(mode, sortDir);
    if (column == _model->getLastSortOrder() && sortDir == _model->getLastSortDir())
        sortDir = (sortDir == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;

    _header->setSortIndicator(column, sortDir);
    _model->sort(column, sortDir);
}

int KrInterBriefView::elementWidth(const QModelIndex & index)
{
    QString text = index.data(Qt::DisplayRole).toString();

    int textWidth = QFontMetrics(_viewFont).width(text);

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    textWidth += 2 * textMargin;

    QVariant decor = index.data(Qt::DecorationRole);
    if (decor.isValid() && decor.type() == QVariant::Pixmap) {
        QPixmap p = decor.value<QPixmap>();
        textWidth += p.width() + 2 * textMargin;
    }

    return textWidth;
}

void KrInterBriefView::intersectionSet(const QRect &rect, QVector<QModelIndex> &ndxList)
{
    int maxNdx = _model->rowCount();
    int width = (viewport()->width()) / _numOfColumns;
    if ((viewport()->width()) % _numOfColumns)
        width++;

    int height = getItemHeight();
    int items = viewport()->height() / height;
    if (items == 0)
        items++;

    int xmin = -1;
    int ymin = -1;
    int xmax = -1;
    int ymax = -1;

    xmin = rect.x() / width;
    ymin = rect.y() / height;
    xmax = (rect.x() + rect.width()) / width;
    if ((rect.x() + rect.width()) % width)
        xmax++;
    ymax = (rect.y() + rect.height()) / height;
    if ((rect.y() + rect.height()) % height)
        ymax++;

    for (int i = ymin; i < ymax; i++)
        for (int j = xmin; j < xmax; j++) {
            int ndx = j * items + i;
            if (ndx < maxNdx)
                ndxList.append(_model->index(ndx, 0));
        }
}
