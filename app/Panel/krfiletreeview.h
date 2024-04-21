/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRFILETREEVIEW_H
#define KRFILETREEVIEW_H

#include <QList>
#include <QModelIndex>
#include <QTreeView>
#include <QUrl>
#include <QWidget>

#include <KDirModel>
#include <KDirSortFilterProxyModel>
#include <KFilePlacesModel>
#include <KSharedConfig>

/**
 * Show folders in a tree view.
 *
 * A context menu with settings is accessible through the header.
 *
 * Supports dragging from this view and dropping files on it.
 */
class KrFileTreeView : public QTreeView
{
    friend class KrDirModel;
    Q_OBJECT

public:
    explicit KrFileTreeView(QWidget *parent = nullptr);
    ~KrFileTreeView() override = default;

    void setCurrentUrl(const QUrl &url);

    void saveSettings(KConfigGroup cfg) const;
    void restoreSettings(const KConfigGroup &cfg);

signals:
    void urlActivated(const QUrl &url);

private slots:
    void slotActivated(const QModelIndex &index);
    void slotExpanded(const QModelIndex &);
    void showHeaderContextMenu();
    void slotCustomContextMenuRequested(const QPoint &point);

protected:
    void dropEvent(QDropEvent *event) override;

private:
    QUrl urlForProxyIndex(const QModelIndex &index) const;
    void dropMimeData(const QList<QUrl> &lst, const QUrl &url) const;
    void copyToClipBoard(const KFileItem &fileItem, bool cut) const;
    void deleteFile(const KFileItem &fileItem, bool moveToTrash = true) const;
    bool briefMode() const;
    void setBriefMode(bool brief); // show only column with directory names
    void setTree(bool startFromCurrent, bool startFromPlace);
    void setTreeRoot(const QUrl &rootBase);
    bool isVisible(const QUrl &url);

    KDirModel *mSourceModel;
    KDirSortFilterProxyModel *mProxyModel;
    KFilePlacesModel *mFilePlacesModel;
    QUrl mCurrentUrl;
    QUrl mCurrentTreeBase;

    bool mStartTreeFromCurrent; // NAND...
    bool mStartTreeFromPlace;
};

#endif // KRFILETREEVIEW_H
