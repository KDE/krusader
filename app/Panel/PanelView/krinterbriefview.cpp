/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krinterbriefview.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QHashIterator>
#include <QItemSelection>
#include <QItemSelectionRange>
// QtGui
#include <QFontMetrics>
#include <QPainter>
#include <QRegion>
// QtWidgets
#include <QApplication>
#include <QFileSystemModel>
#include <QMenu>
#include <QScrollBar>

#include <KConfig>
#include <KDirLister>
#include <KLocalizedString>

#include "../FileSystem/krpermhandler.h"
#include "../GUI/krstyleproxy.h"
#include "../compat.h"
#include "../defaults.h"
#include "../krcolorcache.h"
#include "krmousehandler.h"
#include "krviewfactory.h"
#include "krviewitem.h"
#include "krviewitemdelegate.h"
#include "listmodel.h"

#define MAX_BRIEF_COLS 5

KrInterBriefView::KrInterBriefView(QWidget *parent, KrViewInstance &instance, KConfig *cfg)
    : QAbstractItemView(parent)
    , KrInterView(instance, cfg, this)
    , _header(nullptr)
{
    setWidget(this);
    setModel(_model);

    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionModel(new DummySelectionModel(_model, this));

    KConfigGroup grpSvr(_config, "Look&Feel");
    _viewFont = grpSvr.readEntry("Filelist Font", _FilelistFont);

    auto *style = new KrStyleProxy();
    style->setParent(this);
    setStyle(style);
    viewport()->setStyle(style); // for custom tooltip delay
    setItemDelegate(new KrViewItemDelegate());
    setMouseTracking(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(_mouseHandler, &KrMouseHandler::renameCurrentItem, this, &KrInterBriefView::renameCurrentItem);

    _model->setExtensionEnabled(false);
    _model->setAlternatingTable(true);
    connect(_model, &ListModel::layoutChanged, this, &KrInterBriefView::updateGeometries);
}

KrInterBriefView::~KrInterBriefView()
{
    delete _properties;
    _properties = nullptr;

    delete _operator;
    _operator = nullptr;
}

void KrInterBriefView::doRestoreSettings(KConfigGroup group)
{
    _properties->numberOfColumns = group.readEntry("Number Of Brief Columns", _NumberOfBriefColumns);
    if (_properties->numberOfColumns < 1)
        _properties->numberOfColumns = 1;
    else if (_properties->numberOfColumns > MAX_BRIEF_COLS)
        _properties->numberOfColumns = MAX_BRIEF_COLS;

    _numOfColumns = _properties->numberOfColumns;

    KrInterView::doRestoreSettings(group);

    updateGeometries();
}

void KrInterBriefView::saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties)
{
    KrInterView::saveSettings(grp, properties);
    if (properties & KrViewProperties::PropColumns)
        grp.writeEntry("Number Of Brief Columns", _numOfColumns);
}

int KrInterBriefView::itemsPerPage()
{
    int height = getItemHeight();
    if (height == 0)
        height++;
    int numRows = viewport()->height() / height;
    return numRows;
}
void KrInterBriefView::updateView()
{
}

void KrInterBriefView::setup()
{
    _header = new QHeaderView(Qt::Horizontal, this);
    _header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _header->setParent(this);
    _header->setModel(_model);
    _header->hideSection(KrViewProperties::Type);
    _header->hideSection(KrViewProperties::Permissions);
    _header->hideSection(KrViewProperties::KrPermissions);
    _header->hideSection(KrViewProperties::Owner);
    _header->hideSection(KrViewProperties::Group);
    _header->hideSection(KrViewProperties::Changed);
    _header->hideSection(KrViewProperties::Accessed);
    _header->setStretchLastSection(true);
    _header->setSectionResizeMode(QHeaderView::Fixed);
    _header->setSectionsClickable(true);
    _header->setSortIndicatorShown(true);
    connect(_header, &QHeaderView::sortIndicatorChanged, _model, QOverload<int, Qt::SortOrder>::of(&ListModel::sort));
    _header->installEventFilter(this);

    _numOfColumns = _properties->numberOfColumns;

    setSortMode(_properties->sortColumn, (_properties->sortOptions & KrViewProperties::Descending));
}

void KrInterBriefView::keyPressEvent(QKeyEvent *e)
{
    if (!e || !_model->ready())
        return; // subclass bug

    if (handleKeyEvent(e))
        return;

    QAbstractItemView::keyPressEvent(e);
}

bool KrInterBriefView::handleKeyEvent(QKeyEvent *e)
{
    if (((e->key() != Qt::Key_Left && e->key() != Qt::Key_Right) || (e->modifiers() == Qt::ControlModifier)) && (KrView::handleKeyEvent(e)))
        // did the view class handled the event?
        return true;

    switch (e->key()) {
    case Qt::Key_Right: {
        KrViewItem *i = getCurrentKrViewItem();
        KrViewItem *newCurrent = i;

        if (!i)
            break;

        int num = itemsPerPage() + 1;

        if (e->modifiers() & Qt::ShiftModifier)
            i->setSelected(!i->isSelected());

        while (i && num > 0) {
            if (e->modifiers() & Qt::ShiftModifier)
                i->setSelected(!i->isSelected());
            newCurrent = i;
            i = getNext(i);
            num--;
        }

        if (newCurrent) {
            setCurrentKrViewItem(newCurrent);
            makeCurrentVisible();
        }
        if (e->modifiers() & Qt::ShiftModifier)
            op()->emitSelectionChanged();
        return true;
    }
    case Qt::Key_Left: {
        KrViewItem *i = getCurrentKrViewItem();
        KrViewItem *newCurrent = i;

        if (!i)
            break;

        int num = itemsPerPage() + 1;

        if (e->modifiers() & Qt::ShiftModifier)
            i->setSelected(!i->isSelected());

        while (i && num > 0) {
            if (e->modifiers() & Qt::ShiftModifier)
                i->setSelected(!i->isSelected());
            newCurrent = i;
            i = getPrev(i);
            num--;
        }

        if (newCurrent) {
            setCurrentKrViewItem(newCurrent);
            makeCurrentVisible();
        }
        if (e->modifiers() & Qt::ShiftModifier)
            op()->emitSelectionChanged();
        return true;
    }
    }

    return false;
}

void KrInterBriefView::wheelEvent(QWheelEvent *ev)
{
    if (!_mouseHandler->wheelEvent(ev))
        QApplication::sendEvent(horizontalScrollBar(), ev);
}

bool KrInterBriefView::eventFilter(QObject *object, QEvent *event)
{
    if (object == _header) {
        if (event->type() == QEvent::ContextMenu) {
            auto *me = dynamic_cast<QContextMenuEvent *>(event);
            showContextMenu(me->globalPos());
            return true;
        }
    }
    return false;
}

void KrInterBriefView::showContextMenu(const QPoint &p)
{
    QMenu popup(this);
    popup.setTitle(i18n("Columns"));

    int COL_ID = 14700;

    for (int i = 1; i <= MAX_BRIEF_COLS; i++) {
        QAction *act = popup.addAction(QString("%1").arg(i));
        act->setData(QVariant(COL_ID + i));
        act->setCheckable(true);
        act->setChecked(properties()->numberOfColumns == i);
    }

    QAction *res = popup.exec(p);
    int result = -1;
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    if (result > COL_ID && result <= COL_ID + MAX_BRIEF_COLS) {
        _properties->numberOfColumns = result - COL_ID;
        _numOfColumns = _properties->numberOfColumns;
        updateGeometries();
        op()->settingsChanged(KrViewProperties::PropColumns);
    }
}

QRect KrInterBriefView::visualRect(const QModelIndex &ndx) const
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

QModelIndex KrInterBriefView::indexAt(const QPoint &p) const
{
    int x = p.x() + horizontalOffset();
    int y = p.y() + verticalOffset();

    int itemWidth = (viewport()->width()) / _numOfColumns;
    if ((viewport()->width()) % _numOfColumns)
        itemWidth++;
    int itemHeight = getItemHeight();

    int numRows = viewport()->height() / itemHeight;
    if (numRows == 0)
        numRows++;

    int row = y / itemHeight;
    int col = x / itemWidth;

    int numColsTotal = _model->rowCount() / numRows;
    if (_model->rowCount() % numRows)
        numColsTotal++;

    if (row < numRows && col < numColsTotal)
        return _model->index((col * numRows) + row, 0);

    return QModelIndex();
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

bool KrInterBriefView::isIndexHidden(const QModelIndex &ndx) const
{
    return ndx.column() != 0;
}

void KrInterBriefView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option;
    initViewItemOption(&option);
    option.widget = this;
    option.decorationSize = QSize(_fileIconSize, _fileIconSize);
    option.decorationPosition = QStyleOptionViewItem::Left;
    QPainter painter(viewport());

    QModelIndex curr = currentIndex();

    QVector<QModelIndex> intersectVector;
    QRect area = e->rect();
    area.adjust(horizontalOffset(), verticalOffset(), horizontalOffset(), verticalOffset());
    intersectionSet(area, intersectVector);

    for (const QModelIndex &mndx : std::as_const(intersectVector)) {
        option.rect = visualRect(mndx);
        painter.save();

        itemDelegate()->paint(&painter, option, mndx);

        // (always) draw dashed line border around current item row
        const bool isCurrent = curr.isValid() && curr.row() == mndx.row();
        if (isCurrent && drawCurrent()) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(option);
            QPalette::ColorGroup cg = QPalette::Normal;
            o.backgroundColor = option.palette.color(cg, QPalette::Window);

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
        for (int i = 0; i != _header->count(); i++)
            if (!_header->isSectionHidden(i))
                items++;
        if (items == 0)
            items++;

        int sectWidth = viewport()->width() / items;
        for (int i = 0; i != _header->count(); i++)
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

void KrInterBriefView::setSortMode(KrViewProperties::ColumnType sortColumn, bool descending)
{
    Qt::SortOrder sortDir = descending ? Qt::DescendingOrder : Qt::AscendingOrder;
    _header->setSortIndicator(sortColumn, sortDir);
}

int KrInterBriefView::elementWidth(const QModelIndex &index)
{
    QString text = index.data(Qt::DisplayRole).toString();

    int textWidth = QFontMetrics(_viewFont).horizontalAdvance(text);

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    textWidth += 2 * textMargin;

    QVariant decor = index.data(Qt::DecorationRole);
    if (decor.isValid() && decor.typeId() == QMetaType::QPixmap) {
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

QRect KrInterBriefView::itemRect(const FileItem *item)
{
    return visualRect(_model->fileItemIndex(item));
}

void KrInterBriefView::copySettingsFrom(KrView *other)
{
    if (other->instance() == instance()) { // the other view is of the same type
        auto *v = dynamic_cast<KrInterBriefView *>(other);
        int column = v->_model->lastSortOrder();
        Qt::SortOrder sortDir = v->_model->lastSortDir();
        _header->setSortIndicator(column, sortDir);
        _model->sort(column, sortDir);
        setFileIconSize(v->fileIconSize());
    }
}

void KrInterBriefView::setFileIconSize(int size)
{
    KrView::setFileIconSize(size);
    setIconSize(QSize(fileIconSize(), fileIconSize()));
    updateGeometries();
}

void KrInterBriefView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (_model->ready()) {
        KrViewItem *item = getKrViewItem(currentIndex());
        op()->emitCurrentChanged(item);
    }
    QAbstractItemView::currentChanged(current, previous);
}

void KrInterBriefView::renameCurrentItem()
{
    QModelIndex nameIndex = _model->index(currentIndex().row(), KrViewProperties::Name);

    // cycle through various text selections if we are in the editing mode already
    if (state() == QAbstractItemView::EditingState) {
        auto delegate = dynamic_cast<KrViewItemDelegate *>(itemDelegateForIndex(nameIndex));
        if (!delegate) {
            qWarning() << "KrInterView item delegate is not KrViewItemDelegate, selection is not updated";
            return;
        }

        delegate->cycleEditorSelection();
        return;
    }

    // create and show file name editor
    edit(nameIndex);
    updateEditorData();
    update(nameIndex);
}

bool KrInterBriefView::event(QEvent *e)
{
    _mouseHandler->otherEvent(e);
    return QAbstractItemView::event(e);
}

void KrInterBriefView::mousePressEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mousePressEvent(ev))
        QAbstractItemView::mousePressEvent(ev);
}

void KrInterBriefView::mouseReleaseEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseReleaseEvent(ev))
        QAbstractItemView::mouseReleaseEvent(ev);
}

void KrInterBriefView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseDoubleClickEvent(ev))
        QAbstractItemView::mouseDoubleClickEvent(ev);
}

void KrInterBriefView::mouseMoveEvent(QMouseEvent *ev)
{
    if (!_mouseHandler->mouseMoveEvent(ev))
        QAbstractItemView::mouseMoveEvent(ev);
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
