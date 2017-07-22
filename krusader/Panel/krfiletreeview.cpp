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

#include "../defaults.h"
#include "../krglobal.h"
#include "../FileSystem/filesystemprovider.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QProxyStyle>

#include <KFileItemListProperties>
#include <KJobWidgets>
#include <KUrlMimeData>

#include <KI18n/KLocalizedString>
#include <KIO/DropJob>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIOCore/KFileItem>
#include <KIOWidgets/KDirLister>
#include <KIOWidgets/KFileItemDelegate>
#include <KIOWidgets/KPropertiesDialog>


class KrDirModel : public KDirModel
{
public:
    KrDirModel(QWidget *parent, KrFileTreeView *ftv) : KDirModel(parent), fileTreeView(ftv) {}

protected:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        Qt::ItemFlags itflags = KDirModel::flags(index);
        if (index.column() != KDirModel::Name)
            itflags &= ~Qt::ItemIsDropEnabled;
        return itflags;
    }

private:
    KrFileTreeView *fileTreeView;
};

class TreeStyle : public QProxyStyle
{
public:
    explicit TreeStyle(QStyle *style) : QProxyStyle(style) {}

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                  QStyleHintReturn *returnData) const Q_DECL_OVERRIDE
    {
        if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick) {
            return true;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

KrFileTreeView::KrFileTreeView(QWidget *parent)
    : QTreeView(parent), mCurrentUrl(),
      mCurrentTreeBase(), mStartTreeFromCurrent(false), mStartTreeFromPlace(false)
{
    mSourceModel = new KrDirModel(this, this);
    mSourceModel->dirLister()->setDirOnlyMode(true);

    mProxyModel = new KDirSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mSourceModel);
    setModel(mProxyModel);

    mFilePlacesModel = new KFilePlacesModel(this);

    setItemDelegate(new KFileItemDelegate(this));
    setUniformRowHeights(true);

    // drag&drop
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    mSourceModel->setDropsAllowed(KDirModel::DropOnDirectory);

    setStyle(new TreeStyle(style()));
    connect(this, &KrFileTreeView::activated, this, &KrFileTreeView::slotActivated);

    connect(mSourceModel, &KDirModel::expand, this, &KrFileTreeView::slotExpanded);

    QFontMetrics fontMetrics(viewport()->font());
    header()->resizeSection(KDirModel::Name, fontMetrics.width("WWWWWWWWWWWWWWW"));

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header(), &QHeaderView::customContextMenuRequested, this,
            &KrFileTreeView::showHeaderContextMenu);

    setBriefMode(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &KrFileTreeView::customContextMenuRequested, this,
            &KrFileTreeView::slotCustomContextMenuRequested);
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
        emit urlActivated(url);
}

void KrFileTreeView::dropEvent(QDropEvent *event)
{
    QUrl destination = urlForProxyIndex(indexAt(event->pos()));
    if (destination.isEmpty()) {
        return;
    }

    FileSystemProvider::instance().startDropFiles(event, destination);
}

void KrFileTreeView::slotExpanded(const QModelIndex &baseIndex)
{
    QModelIndex index = mProxyModel->mapFromSource(baseIndex);

    expand(index); // expand view now after model was expanded
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    scrollTo(index);
}

void KrFileTreeView::setCurrentUrl(const QUrl &url)
{
    mCurrentUrl = url;
    if (mStartTreeFromCurrent) {
        setTreeRoot(url);
    } else {
        if (mStartTreeFromPlace) {
            const QModelIndex index = mFilePlacesModel->closestItem(url); // magic here
            const QUrl rootBase = index.isValid() ? mFilePlacesModel->url(index) :
                                                    QUrl::fromLocalFile(QDir::root().path());
            setTreeRoot(rootBase);
        }
        mSourceModel->expandToUrl(url);
    }
}

void KrFileTreeView::showHeaderContextMenu()
{
    QMenu popup(this);
    popup.setToolTipsVisible(true);

    QAction *detailAction = popup.addAction(i18n("Show Details"));
    detailAction->setCheckable(true);
    detailAction->setChecked(!briefMode());
    detailAction->setToolTip(i18n("Show columns with details."));
    QAction *showHiddenAction = popup.addAction(i18n("Show Hidden Folders"));
    showHiddenAction->setCheckable(true);
    showHiddenAction->setChecked(mSourceModel->dirLister()->showingDotFiles());
    showHiddenAction->setToolTip(i18n("Show folders starting with a dot."));

    popup.addSeparator();
    QActionGroup *rootActionGroup = new QActionGroup(this);

    QAction *startFromRootAction = popup.addAction(i18n("Start From Root"));
    startFromRootAction->setCheckable(true);
    startFromRootAction->setChecked(!mStartTreeFromCurrent && !mStartTreeFromPlace);
    startFromRootAction->setToolTip(i18n("Set root of the tree to root of filesystem."));
    startFromRootAction->setActionGroup(rootActionGroup);

    QAction *startFromCurrentAction = popup.addAction(i18n("Start From Current"));
    startFromCurrentAction->setCheckable(true);
    startFromCurrentAction->setChecked(mStartTreeFromCurrent);
    startFromCurrentAction->setToolTip(i18n("Set root of the tree to the current folder."));
    startFromCurrentAction->setActionGroup(rootActionGroup);

    QAction *startFromPlaceAction = popup.addAction(i18n("Start From Place"));
    startFromPlaceAction->setCheckable(true);
    startFromPlaceAction->setChecked(mStartTreeFromPlace);
    startFromPlaceAction->setToolTip(
        i18n("Set root of the tree to closest folder listed in 'Places'."));
    startFromPlaceAction->setActionGroup(rootActionGroup);

    QAction *triggeredAction = popup.exec(QCursor::pos());
    if (triggeredAction == detailAction) {
        setBriefMode(!detailAction->isChecked());
    } else if (triggeredAction == showHiddenAction) {
        KDirLister *dirLister = mSourceModel->dirLister();
        dirLister->setShowingDotFiles(showHiddenAction->isChecked());
        dirLister->emitChanges();
    } else if (triggeredAction && triggeredAction->actionGroup() == rootActionGroup) {
        setTree(startFromCurrentAction->isChecked(), startFromPlaceAction->isChecked());
    }
}

void KrFileTreeView::slotCustomContextMenuRequested(const QPoint &point)
{
    const QModelIndex index = indexAt(point);
    if (!index.isValid())
        return;

    const KFileItem fileItem = mSourceModel->itemForIndex(mProxyModel->mapToSource(index));
    const KFileItemListProperties capabilities(KFileItemList() << fileItem);

    QMenu* popup = new QMenu(this);

    // TODO nice to have: "open with"

    // cut/copy/paste
    QAction* cutAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-cut")), i18nc("@action:inmenu", "Cut"), this);
    cutAction->setEnabled(capabilities.supportsMoving());
    connect(cutAction, &QAction::triggered, this, [=]() { copyToClipBoard(fileItem, true); });
    popup->addAction(cutAction);

    QAction* copyAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "Copy"), this);
    connect(copyAction, &QAction::triggered, this, [=]() { copyToClipBoard(fileItem, false); });
    popup->addAction(copyAction);

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    bool canPaste;
    const QString text = KIO::pasteActionText(mimeData, &canPaste, fileItem);
    QAction* pasteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), text, this);
    connect(pasteAction, &QAction::triggered, this, [=]() {
        KIO::PasteJob *job = KIO::paste(QApplication::clipboard()->mimeData(), fileItem.url());
        KJobWidgets::setWindow(job, this);
    });
    pasteAction->setEnabled(canPaste);
    popup->addAction(pasteAction);

    popup->addSeparator();

    // TODO nice to have: rename

    // trash
    if (KConfigGroup(krConfig, "General").readEntry("Move To Trash", _MoveToTrash)) {
        QAction* moveToTrashAction = new QAction(QIcon::fromTheme(QStringLiteral("user-trash")),
                                                i18nc("@action:inmenu", "Move to Trash"), this);
        const bool enableMoveToTrash = capabilities.isLocal() && capabilities.supportsMoving();
        moveToTrashAction->setEnabled(enableMoveToTrash);
        connect(moveToTrashAction, &QAction::triggered, this, [=]() {
            deleteFile(fileItem, true);
        });
        popup->addAction(moveToTrashAction);
    }

    // delete
    QAction *deleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                        i18nc("@action:inmenu", "Delete"), this);
    deleteAction->setEnabled(capabilities.supportsDeleting());
    connect(deleteAction, &QAction::triggered, this, [=]() {
        deleteFile(fileItem, false);
    });
    popup->addAction(deleteAction);

    popup->addSeparator();

    // properties
    if (!fileItem.isNull()) {
        QAction* propertiesAction = new QAction(i18nc("@action:inmenu", "Properties"), this);
        propertiesAction->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
        connect(propertiesAction, &QAction::triggered, this, [=]() {
            KPropertiesDialog* dialog = new KPropertiesDialog(fileItem.url(), this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
        });
        popup->addAction(propertiesAction);
    }

    QPointer<QMenu> popupPtr = popup;
    popup->exec(QCursor::pos());
    if (popupPtr.data()) {
        popupPtr.data()->deleteLater();
    }
}

void KrFileTreeView::copyToClipBoard(const KFileItem &fileItem, bool cut) const
{
    QMimeData* mimeData = new QMimeData();

    QList<QUrl> kdeUrls;
    kdeUrls.append(fileItem.url());
    QList<QUrl> mostLocalUrls;
    bool dummy;
    mostLocalUrls.append(fileItem.mostLocalUrl(dummy));

    KIO::setClipboardDataCut(mimeData, cut);
    KUrlMimeData::setUrls(kdeUrls, mostLocalUrls, mimeData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void KrFileTreeView::deleteFile(const KFileItem &fileItem, bool moveToTrash) const
{
    const QList<QUrl> confirmedFiles =
        ListPanelFunc::confirmDeletion(QList<QUrl>() << fileItem.url(), moveToTrash, false, true);
    if (confirmedFiles.isEmpty())
        return;

    FileSystemProvider::instance().startDeleteFiles(confirmedFiles, moveToTrash);
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

void KrFileTreeView::setTree(bool startFromCurrent, bool startFromPlace)
{
    mStartTreeFromCurrent = startFromCurrent;
    mStartTreeFromPlace = startFromPlace;

    if (!mStartTreeFromCurrent && !mStartTreeFromPlace) {
        setTreeRoot(QUrl::fromLocalFile(QDir::root().path()));
    }
    setCurrentUrl(mCurrentUrl); // refresh
}

void KrFileTreeView::setTreeRoot(const QUrl &rootBase)
{
    if (rootBase == mCurrentTreeBase) // avoid collapsing the subdirs in tree
        return;

    mCurrentTreeBase = rootBase;
    mSourceModel->dirLister()->openUrl(mCurrentTreeBase);
}

void KrFileTreeView::saveSettings(KConfigGroup cfg) const
{
    KConfigGroup group = KConfigGroup(&cfg, "TreeView");
    group.writeEntry("BriefMode", briefMode());
    group.writeEntry("ShowHiddenFolders", mSourceModel->dirLister()->showingDotFiles());
    group.writeEntry("StartFromCurrent", mStartTreeFromCurrent);
    group.writeEntry("StartFromPlace", mStartTreeFromPlace);
}

void KrFileTreeView::restoreSettings(const KConfigGroup &cfg)
{
    const KConfigGroup group = KConfigGroup(&cfg, "TreeView");
    setBriefMode(group.readEntry("BriefMode", true));
    mSourceModel->dirLister()->setShowingDotFiles(group.readEntry("ShowHiddenFolders", false));
    setTree(group.readEntry("StartFromCurrent", false),
                group.readEntry("StartFromPlace", false));
}
