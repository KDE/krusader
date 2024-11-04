/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dulistview.h"
#include "../FileSystem/krpermhandler.h"
#include "../icon.h"
#include "../krglobal.h"

// QtCore
#include <QMimeDatabase>
#include <QMimeType>
// QtGui
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
// QtWidgets
#include <QHeaderView>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../compat.h"

DUListView::DUListView(DiskUsage *usage)
    : KrTreeWidget(usage)
    , diskUsage(usage)
{
    setAllColumnsShowFocus(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setRootIsDecorated(true);
    setIndentation(10);
    setItemsExpandable(true);

    QStringList labels;
    labels << i18n("Name");
    labels << i18n("Percent");
    labels << i18n("Total size");
    labels << i18n("Own size");
    labels << i18n("Type");
    labels << i18n("Date");
    labels << i18n("Permissions");
    labels << i18n("Owner");
    labels << i18n("Group");
    setHeaderLabels(labels);

    header()->setSectionResizeMode(QHeaderView::Interactive);

    KConfigGroup group(krConfig, diskUsage->getConfigGroup());

    if (group.hasKey("D State"))
        header()->restoreState(group.readEntry("D State", QByteArray()));
    else {
        int defaultSize = QFontMetrics(font()).horizontalAdvance("W");

        setColumnWidth(0, defaultSize * 20);
        setColumnWidth(1, defaultSize * 5);
        setColumnWidth(2, defaultSize * 10);
        setColumnWidth(3, defaultSize * 10);
        setColumnWidth(4, defaultSize * 10);
        setColumnWidth(5, defaultSize * 10);
        setColumnWidth(6, defaultSize * 6);
        setColumnWidth(7, defaultSize * 5);
        setColumnWidth(8, defaultSize * 5);
    }

    header()->setSortIndicatorShown(true);
    sortItems(2, Qt::AscendingOrder);

    connect(diskUsage, &DiskUsage::enteringDirectory, this, &DUListView::slotDirChanged);
    connect(diskUsage, &DiskUsage::clearing, this, &DUListView::clear);
    connect(diskUsage, &DiskUsage::changed, this, &DUListView::slotChanged);
    connect(diskUsage, &DiskUsage::deleted, this, &DUListView::slotDeleted);

    connect(this, &DUListView::itemRightClicked, this, &DUListView::slotRightClicked);
    connect(this, &DUListView::itemExpanded, this, &DUListView::slotExpanded);
}

DUListView::~DUListView()
{
    KConfigGroup group(krConfig, diskUsage->getConfigGroup());
    group.writeEntry("D State", header()->saveState());
}

void DUListView::addDirectory(Directory *dirEntry, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *lastItem = nullptr;

    if (parent == nullptr && !(dirEntry->parent() == nullptr)) {
        lastItem = new QTreeWidgetItem(this);
        lastItem->setText(0, "..");
        lastItem->setIcon(0, Icon("go-up"));
        lastItem->setFlags(Qt::ItemIsEnabled);
    }

    for (Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it) {
        File *item = *it;

        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForName(item->mime());
        QString mime;
        if (mt.isValid())
            mime = mt.comment();

        time_t tma = item->time();
        struct tm *t = localtime((time_t *)&tma);
        QDateTime tmp(QDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
        QString date = QLocale().toString(tmp, QLocale::ShortFormat);

        QString totalSize = KrPermHandler::parseSize(item->size()) + ' ';
        QString ownSize = KrPermHandler::parseSize(item->ownSize()) + ' ';
        QString percent = item->percent();

        if (lastItem == nullptr && parent == nullptr)
            lastItem =
                new DUListViewItem(diskUsage, item, this, item->name(), percent, totalSize, ownSize, mime, date, item->perm(), item->owner(), item->group());
        else if (lastItem == nullptr)
            lastItem =
                new DUListViewItem(diskUsage, item, parent, item->name(), percent, totalSize, ownSize, mime, date, item->perm(), item->owner(), item->group());
        else if (parent == nullptr)
            lastItem = new DUListViewItem(diskUsage,
                                          item,
                                          this,
                                          lastItem,
                                          item->name(),
                                          percent,
                                          totalSize,
                                          ownSize,
                                          mime,
                                          date,
                                          item->perm(),
                                          item->owner(),
                                          item->group());
        else
            lastItem = new DUListViewItem(diskUsage,
                                          item,
                                          parent,
                                          lastItem,
                                          item->name(),
                                          percent,
                                          totalSize,
                                          ownSize,
                                          mime,
                                          date,
                                          item->perm(),
                                          item->owner(),
                                          item->group());

        if (item->isExcluded())
            lastItem->setHidden(true);

        lastItem->setIcon(0, diskUsage->getIcon(item->mime()));

        if (item->isDir() && !item->isSymLink())
            lastItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }

    if (topLevelItemCount() > 0) {
        setCurrentItem(topLevelItem(0));
    }
}

void DUListView::slotDirChanged(Directory *dirEntry)
{
    clear();
    addDirectory(dirEntry, nullptr);
}

File *DUListView::getCurrentFile()
{
    QTreeWidgetItem *item = currentItem();

    if (item == nullptr || item->text(0) == "..")
        return nullptr;

    return (dynamic_cast<DUListViewItem *>(item))->getFile();
}

void DUListView::slotChanged(File *item)
{
    void *itemPtr = diskUsage->getProperty(item, "ListView-Ref");
    if (itemPtr == nullptr)
        return;

    auto *duItem = (DUListViewItem *)itemPtr;
    duItem->setHidden(item->isExcluded());
    duItem->setText(1, item->percent());
    duItem->setText(2, KrPermHandler::parseSize(item->size()) + ' ');
    duItem->setText(3, KrPermHandler::parseSize(item->ownSize()) + ' ');
}

void DUListView::slotDeleted(File *item)
{
    void *itemPtr = diskUsage->getProperty(item, "ListView-Ref");
    if (itemPtr == nullptr)
        return;

    auto *duItem = (DUListViewItem *)itemPtr;
    delete duItem;
}

void DUListView::slotRightClicked(QTreeWidgetItem *item, const QPoint &pos)
{
    File *file = nullptr;

    if (item && item->text(0) != "..")
        file = (dynamic_cast<DUListViewItem *>(item))->getFile();

    diskUsage->rightClickMenu(pos, file);
}

bool DUListView::doubleClicked(QTreeWidgetItem *item)
{
    if (item) {
        if (item->text(0) != "..") {
            File *fileItem = (dynamic_cast<DUListViewItem *>(item))->getFile();
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

void DUListView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e || e->button() == Qt::LeftButton) {
        QPoint vp = viewport()->mapFromGlobal(e->globalPosition().toPoint());
        QTreeWidgetItem *item = itemAt(vp);

        if (doubleClicked(item))
            return;
    }
    KrTreeWidget::mouseDoubleClickEvent(e);
}

void DUListView::keyPressEvent(QKeyEvent *e)
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

void DUListView::slotExpanded(QTreeWidgetItem *item)
{
    if (item == nullptr || item->text(0) == "..")
        return;

    if (item->childCount() == 0) {
        File *fileItem = (dynamic_cast<DUListViewItem *>(item))->getFile();
        if (fileItem->isDir())
            addDirectory(dynamic_cast<Directory *>(fileItem), item);
    }
}
