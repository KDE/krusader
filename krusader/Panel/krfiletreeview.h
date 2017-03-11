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

#include <QModelIndex>
#include <QList>
#include <QUrl>
#include <QTreeView>
#include <QWidget>

#include <KIOWidgets/KDirModel>
#include <KIOFileWidgets/KDirSortFilterProxyModel>

/**
 * @brief Shows a generic file tree
 */
class KrFileTreeView : public QTreeView
{
    friend class KrDirModel;
    Q_OBJECT

public:
    explicit KrFileTreeView(QWidget *parent = 0);
    virtual ~KrFileTreeView() {}

    QUrl currentUrl() const;
    bool briefMode() const;
    void setBriefMode(bool brief); // show only column with directory names

public slots:
    void setDirOnlyMode(bool enabled);
    void setShowHiddenFiles(bool enabled);
    void setCurrentUrl(const QUrl &url);

signals:
    void activated(const QUrl &url);

private slots:
    void slotActivated(const QModelIndex&);
    void slotExpanded(const QModelIndex&);
    void showHeaderContextMenu();

protected:
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

private:
    QUrl urlForProxyIndex(const QModelIndex &index) const;
    void dropMimeData(const QList<QUrl> & lst, const QUrl &url);

    KDirModel *mSourceModel;
    KDirSortFilterProxyModel *mProxyModel;
};

#endif // KRFILETREEVIEW_H
