/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dulines.h"

#include "../FileSystem/krpermhandler.h"
#include "../icon.h"
#include "../krglobal.h"

// QtCore
#include <QTimer>
// QtGui
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
// QtWidgets
#include <QApplication>
#include <QHeaderView>
#include <QItemDelegate>
#include <QMenu>
#include <QToolTip>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../compat.h"

class DULinesItemDelegate : public QItemDelegate
{
public:
    explicit DULinesItemDelegate(QObject *parent = nullptr)
        : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QItemDelegate::paint(painter, option, index);

        QVariant value = index.data(Qt::UserRole);
        if (value.isValid()) {
            QString text = value.toString();

            value = index.data(Qt::DisplayRole);
            QString display;
            if (value.isValid())
                display = value.toString();

            QSize iconSize;
            value = index.data(Qt::DecorationRole);
            if (value.isValid())
                iconSize = qvariant_cast<QIcon>(value).actualSize(option.decorationSize);

            painter->save();
            painter->setClipRect(option.rect);

            QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
                cg = QPalette::Inactive;
            if (option.state & QStyle::State_Selected) {
                painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
            } else {
                painter->setPen(option.palette.color(cg, QPalette::Text));
            }

            QFont fnt = option.font;
            fnt.setItalic(true);
            painter->setFont(fnt);

            QFontMetrics fm(fnt);
            QString renderedText = text;

            int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
            int pos = 3 * textMargin + option.fontMetrics.horizontalAdvance(display) + iconSize.width();

            bool truncd = false;

            QRect rct = option.rect;
            if (rct.width() > pos) {
                rct.setX(rct.x() + pos);

                if (fm.horizontalAdvance(renderedText) > rct.width()) {
                    truncd = true;

                    int points = fm.horizontalAdvance("...");

                    while (!renderedText.isEmpty() && (fm.horizontalAdvance(renderedText) + points > rct.width()))
                        renderedText.truncate(renderedText.length() - 1);

                    renderedText += "...";
                }

                painter->drawText(rct, Qt::AlignLeft, renderedText);
            } else
                truncd = true;

            if (truncd)
                const_cast<QAbstractItemModel *>(index.model())->setData(index, QVariant(display + "  " + text), Qt::ToolTipRole);
            else
                const_cast<QAbstractItemModel *>(index.model())->setData(index, QVariant(), Qt::ToolTipRole);

            painter->restore();
        }
    }
};

class DULinesItem : public QTreeWidgetItem
{
public:
    DULinesItem(File *fileItem, QTreeWidget *parent, const QString &label1, const QString &label2, const QString &label3)
        : QTreeWidgetItem(parent)
        , file(fileItem)
    {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);

        setTextAlignment(1, Qt::AlignRight);
    }
    DULinesItem(File *fileItem, QTreeWidget *parent, QTreeWidgetItem *after, const QString &label1, const QString &label2, const QString &label3)
        : QTreeWidgetItem(parent, after)
        , file(fileItem)
    {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);

        setTextAlignment(1, Qt::AlignRight);
    }

    bool operator<(const QTreeWidgetItem &other) const override
    {
        int column = treeWidget() ? treeWidget()->sortColumn() : 0;

        if (text(0) == "..")
            return true;

        const auto *compWith = dynamic_cast<const DULinesItem *>(&other);
        if (compWith == nullptr)
            return false;

        switch (column) {
        case 0:
        case 1:
            return file->size() > compWith->file->size();
        default:
            return text(column) < other.text(column);
        }
    }

    inline File *getFile()
    {
        return file;
    }

private:
    File *file;
};

DULines::DULines(DiskUsage *usage)
    : KrTreeWidget(usage)
    , diskUsage(usage)
    , refreshNeeded(false)
    , started(false)
{
    setItemDelegate(itemDelegate = new DULinesItemDelegate());

    setAllColumnsShowFocus(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setIndentation(10);

    int defaultSize = QFontMetrics(font()).horizontalAdvance("W");

    QStringList labels;
    labels << i18n("Line View");
    labels << i18n("Percent");
    labels << i18n("Name");
    setHeaderLabels(labels);

    header()->setSectionResizeMode(QHeaderView::Interactive);

    KConfigGroup group(krConfig, diskUsage->getConfigGroup());

    showFileSize = group.readEntry("L Show File Size", true);

    if (group.hasKey("L State"))
        header()->restoreState(group.readEntry("L State", QByteArray()));
    else {
        setColumnWidth(0, defaultSize * 20);
        setColumnWidth(1, defaultSize * 6);
        setColumnWidth(2, defaultSize * 20);
    }

    setStretchingColumn(0);

    header()->setSortIndicatorShown(true);
    sortItems(1, Qt::AscendingOrder);

    connect(diskUsage, &DiskUsage::enteringDirectory, this, &DULines::slotDirChanged);
    connect(diskUsage, &DiskUsage::clearing, this, &DULines::clear);

    connect(header(), &QHeaderView::sectionResized, this, &DULines::sectionResized);

    connect(this, &DULines::itemRightClicked, this, &DULines::slotRightClicked);
    connect(diskUsage, &DiskUsage::changed, this, &DULines::slotChanged);
    connect(diskUsage, &DiskUsage::deleted, this, &DULines::slotDeleted);

    started = true;
}

DULines::~DULines()
{
    KConfigGroup group(krConfig, diskUsage->getConfigGroup());
    group.writeEntry("L State", header()->saveState());

    delete itemDelegate;
}

bool DULines::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ToolTip: {
        auto *he = dynamic_cast<QHelpEvent *>(event);

        if (viewport()) {
            QPoint pos = viewport()->mapFromGlobal(he->globalPos());

            QTreeWidgetItem *item = itemAt(pos);

            int column = columnAt(pos.x());

            if (item && column == 1) {
                File *fileItem = (dynamic_cast<DULinesItem *>(item))->getFile();
                QToolTip::showText(he->globalPos(), diskUsage->getToolTip(fileItem), this);
                return true;
            }
        }
    } break;
    default:
        break;
    }
    return KrTreeWidget::event(event);
}

void DULines::slotDirChanged(Directory *dirEntry)
{
    clear();

    QTreeWidgetItem *lastItem = nullptr;

    if (!(dirEntry->parent() == nullptr)) {
        lastItem = new QTreeWidgetItem(this);
        lastItem->setText(0, "..");
        lastItem->setIcon(0, Icon("go-up"));
        lastItem->setFlags(lastItem->flags() & (~Qt::ItemIsSelectable));
    }

    int maxPercent = -1;
    for (Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it) {
        File *item = *it;
        if (!item->isExcluded() && item->intPercent() > maxPercent)
            maxPercent = item->intPercent();
    }

    for (Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it) {
        File *item = *it;

        QString fileName = item->name();

        if (lastItem == nullptr)
            lastItem = new DULinesItem(item, this, "", item->percent() + "  ", fileName);
        else
            lastItem = new DULinesItem(item, this, lastItem, "", item->percent() + "  ", fileName);

        if (item->isExcluded())
            lastItem->setHidden(true);

        int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

        lastItem->setIcon(2, diskUsage->getIcon(item->mime()));
        lastItem->setData(0, Qt::DecorationRole, createPixmap(item->intPercent(), maxPercent, header()->sectionSize(0) - 2 * textMargin));

        if (showFileSize)
            lastItem->setData(2, Qt::UserRole, QVariant(QString("  [") + KIO::convertSize(item->size()) + QString("]")).toString());

        QSize size = lastItem->sizeHint(0);
        size.setWidth(16);
        lastItem->setSizeHint(0, size);
    }

    if (topLevelItemCount() > 0) {
        setCurrentItem(topLevelItem(0));
    }
}

QPixmap DULines::createPixmap(int percent, int maxPercent, int maxWidth)
{
    if (percent < 0 || percent > maxPercent || maxWidth < 2 || maxPercent == 0)
        return QPixmap();
    maxWidth -= 2;

    int actualWidth = maxWidth * percent / maxPercent;
    if (actualWidth == 0)
        return QPixmap();

    QPen pen;
    pen.setColor(Qt::black);
    QPainter painter;

    int size = QFontMetrics(font()).height() - 2;
    QRect rect(0, 0, actualWidth, size);
    QRect frameRect(0, 0, actualWidth - 1, size - 1);
    QPixmap pixmap(rect.width(), rect.height());

    painter.begin(&pixmap);
    painter.setPen(pen);

    for (int i = 1; i < actualWidth - 1; i++) {
        int color = (511 * i / (maxWidth - 1));
        if (color < 256)
            pen.setColor(QColor(255 - color, 255, 0));
        else
            pen.setColor(QColor(color - 256, 511 - color, 0));

        painter.setPen(pen);
        painter.drawLine(i, 1, i, size - 1);
    }

    pen.setColor(Qt::black);
    painter.setPen(pen);

    if (actualWidth != 1)
        painter.drawRect(frameRect);
    else
        painter.drawLine(0, 0, 0, size);

    painter.end();
    pixmap.detach();
    return pixmap;
}

void DULines::resizeEvent(QResizeEvent *re)
{
    KrTreeWidget::resizeEvent(re);

    if (started && (re->oldSize() != re->size()))
        sectionResized(0);
}

void DULines::sectionResized(int column)
{
    if (topLevelItemCount() == 0 || column != 0)
        return;

    Directory *currentDir = diskUsage->getCurrentDir();
    if (currentDir == nullptr)
        return;

    int maxPercent = -1;
    for (Iterator<File> it = currentDir->iterator(); it != currentDir->end(); ++it) {
        File *item = *it;

        if (!item->isExcluded() && item->intPercent() > maxPercent)
            maxPercent = item->intPercent();
    }

    QTreeWidgetItemIterator it2(this);
    while (*it2) {
        QTreeWidgetItem *lvitem = *it2;
        if (lvitem->text(0) != "..") {
            auto *duItem = dynamic_cast<DULinesItem *>(lvitem);
            if (duItem) {
                int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                duItem->setData(0, Qt::DecorationRole, createPixmap(duItem->getFile()->intPercent(), maxPercent, header()->sectionSize(0) - 2 * textMargin));
                QSize size = duItem->sizeHint(0);
                size.setWidth(16);
                duItem->setSizeHint(0, size);
            }
        }
        it2++;
    }
}

bool DULines::doubleClicked(QTreeWidgetItem *item)
{
    if (item) {
        if (item->text(0) != "..") {
            File *fileItem = (dynamic_cast<DULinesItem *>(item))->getFile();
            if (fileItem->isDir())
                diskUsage->changeDirectory(dynamic_cast<Directory *>(fileItem));
            return true;
        } else {
            auto *upDir = const_cast<Directory *>(diskUsage->getCurrentDir()->parent());

            if (upDir)
                diskUsage->changeDirectory(upDir);
            return true;
        }
    }
    return false;
}

void DULines::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e || e->button() == Qt::LeftButton) {
        QPoint vp = viewport()->mapFromGlobal(e->globalPosition().toPoint());
        QTreeWidgetItem *item = itemAt(vp);

        if (doubleClicked(item))
            return;
    }
    KrTreeWidget::mouseDoubleClickEvent(e);
}

void DULines::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (doubleClicked(currentItem()))
            return;
        break;
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        if (e->modifiers() == Qt::ShiftModifier) {
            e->ignore();
            return;
        }
        break;
    case Qt::Key_Delete:
        e->ignore();
        return;
    }
    KrTreeWidget::keyPressEvent(e);
}

void DULines::slotRightClicked(QTreeWidgetItem *item, const QPoint &pos)
{
    File *file = nullptr;

    if (item && item->text(0) != "..")
        file = (dynamic_cast<DULinesItem *>(item))->getFile();

    QMenu linesPopup;
    QAction *act = linesPopup.addAction(i18n("Show file sizes"), this, SLOT(slotShowFileSizes()));
    act->setChecked(showFileSize);

    diskUsage->rightClickMenu(pos, file, &linesPopup, i18n("Lines"));
}

void DULines::slotShowFileSizes()
{
    showFileSize = !showFileSize;
    slotDirChanged(diskUsage->getCurrentDir());
}

File *DULines::getCurrentFile()
{
    QTreeWidgetItem *item = currentItem();

    if (item == nullptr || item->text(0) == "..")
        return nullptr;

    return (dynamic_cast<DULinesItem *>(item))->getFile();
}

void DULines::slotChanged(File *item)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        QTreeWidgetItem *lvitem = *it;
        it++;

        if (lvitem->text(0) != "..") {
            auto *duItem = dynamic_cast<DULinesItem *>(lvitem);
            if (duItem->getFile() == item) {
                setSortingEnabled(false);
                duItem->setHidden(item->isExcluded());
                duItem->setText(1, item->percent());
                if (!refreshNeeded) {
                    refreshNeeded = true;
                    QTimer::singleShot(0, this, &DULines::slotRefresh);
                }
                break;
            }
        }
    }
}

void DULines::slotDeleted(File *item)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        QTreeWidgetItem *lvitem = *it;
        it++;

        if (lvitem->text(0) != "..") {
            auto *duItem = dynamic_cast<DULinesItem *>(lvitem);
            if (duItem->getFile() == item) {
                delete duItem;
                break;
            }
        }
    }
}

void DULines::slotRefresh()
{
    if (refreshNeeded) {
        refreshNeeded = false;
        setSortingEnabled(true);
        sortItems(1, Qt::AscendingOrder);
    }
}
