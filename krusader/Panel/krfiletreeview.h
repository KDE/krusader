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

#ifndef KRFILETREEVIEW_H
#define KRFILETREEVIEW_H

#include <QList>
#include <QModelIndex>
#include <QTreeView>
#include <QUrl>
#include <QWidget>

#include <KConfigCore/KSharedConfig>
#include <KIOFileWidgets/KDirSortFilterProxyModel>
#include <KIOFileWidgets/KFilePlacesModel>
#include <KIOWidgets/KDirModel>

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
    explicit KrFileTreeView(QWidget *parent = 0);
    virtual ~KrFileTreeView() {}

    void saveSettings(KConfigGroup cfg) const;
    void restoreSettings(const KConfigGroup &cfg);

public slots:
    void setCurrentUrl(const QUrl &url);

signals:
    void urlActivated(const QUrl &url);

private slots:
    void slotActivated(const QModelIndex &index);
    void slotExpanded(const QModelIndex&);
    void showHeaderContextMenu();
    void slotCustomContextMenuRequested(const QPoint &point);

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QUrl urlForProxyIndex(const QModelIndex &index) const;
    void dropMimeData(const QList<QUrl> & lst, const QUrl &url) const;
    void copyToClipBoard(const KFileItem &fileItem, bool cut) const;
    void deleteFile(const KFileItem &fileItem, bool moveToTrash = true) const;
    bool briefMode() const;
    void setBriefMode(bool brief); // show only column with directory names
    void setTree(bool startFromCurrent, bool startFromPlace);
    void setTreeRoot(const QUrl &rootBase);

    KDirModel *mSourceModel;
    KDirSortFilterProxyModel *mProxyModel;
    KFilePlacesModel *mFilePlacesModel;
    QUrl mCurrentUrl;
    QUrl mCurrentTreeBase;

    bool mStartTreeFromCurrent; // NAND...
    bool mStartTreeFromPlace;
};

#endif // KRFILETREEVIEW_H
