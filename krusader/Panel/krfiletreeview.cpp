/*****************************************************************************
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
#include "krfiletreeview.h"

#include "panelfunc.h"

#include <QAction>
#include <QCursor>
#include <QDir>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>

#include <KIO/DropJob>
#include <KIOCore/KFileItem>
#include <KIOWidgets/KDirLister>
#include <KIOWidgets/KFileItemDelegate>
#include <KI18n/KLocalizedString>

class KrDirModel : public KDirModel
{
public:
    KrDirModel(QWidget *parent, KrFileTreeView *ftv) : KDirModel(parent), fileTreeView(ftv) {}

protected:
    virtual Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE {
        Qt::ItemFlags itflags = KDirModel::flags(index);
        if (index.column() != KDirModel::Name)
            itflags &= ~Qt::ItemIsDropEnabled;
        return itflags;
    }
private:
    KrFileTreeView * fileTreeView;
};

KrFileTreeView::KrFileTreeView(QWidget *parent)
        : QTreeView(parent)
{
    mSourceModel = new KrDirModel(this, this);
    mProxyModel = new KDirSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mSourceModel);

    mSourceModel->setDropsAllowed(KDirModel::DropOnDirectory);

    setModel(mProxyModel);
    setItemDelegate(new KFileItemDelegate(this));
    setUniformRowHeights(true);

    mSourceModel->dirLister()->openUrl(QUrl::fromLocalFile(QDir::root().absolutePath()), KDirLister::Keep);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));

    connect(mSourceModel, SIGNAL(expand(QModelIndex)),
            this, SLOT(slotExpanded(QModelIndex)));

    QFontMetrics fontMetrics(viewport()->font());
    header()->resizeSection(KDirModel::Name, fontMetrics.width("WWWWWWWWWWWWWWW"));

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showHeaderContextMenu()));
}

QUrl KrFileTreeView::urlForProxyIndex(const QModelIndex &index) const
{
    const KFileItem item = mSourceModel->itemForIndex(mProxyModel->mapToSource(index));

    return !item.isNull() ? item.url() : QUrl();
}

void KrFileTreeView::slotActivated(const QModelIndex &index)
{
    const QUrl url = urlForProxyIndex(index);
    if (url.isValid())
        emit activated(url);
}

void KrFileTreeView::dropEvent(QDropEvent *event)
{
    QUrl destination = urlForProxyIndex(indexAt(event->pos()));
    if (destination.isEmpty()) {
        return;
    }

    KIO::drop(event, destination);
    // TODO show error message job result and refresh event source
}

void KrFileTreeView::slotExpanded(const QModelIndex &baseIndex)
{
    QModelIndex index = mProxyModel->mapFromSource(baseIndex);

    expand(index); // expand view now after model was expanded
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    scrollTo(index);
}

QUrl KrFileTreeView::currentUrl() const
{
    return urlForProxyIndex(currentIndex());
}

void KrFileTreeView::setDirOnlyMode(bool enabled)
{
    mSourceModel->dirLister()->setDirOnlyMode(enabled);
    mSourceModel->dirLister()->openUrl(mSourceModel->dirLister()->url());
}

void KrFileTreeView::setShowHiddenFiles(bool enabled)
{
    mSourceModel->dirLister()->setShowingDotFiles(enabled);
    mSourceModel->dirLister()->openUrl(mSourceModel->dirLister()->url());
}

void KrFileTreeView::setCurrentUrl(const QUrl &url)
{
    mSourceModel->expandToUrl(url);
}

void KrFileTreeView::showHeaderContextMenu()
{
    QMenu popup(this);

    QAction *action = popup.addAction(i18n("Show Details"));
    action->setCheckable(true);
    action->setChecked(!briefMode());

    if (popup.exec(QCursor::pos())) { // if action was clicked
        setBriefMode(!action->isChecked());
    }
}

bool KrFileTreeView::briefMode() const
{
    return isColumnHidden(mProxyModel->columnCount() - 1); // find out by last column
}

void KrFileTreeView::setBriefMode(bool brief)
{
    for (int i=1; i < mProxyModel->columnCount(); i++) { // show only first column
        setColumnHidden(i, brief);
    }
}
