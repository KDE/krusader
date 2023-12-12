/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LISTMODEL_H
#define LISTMODEL_H

// QtCore
#include <QAbstractListModel>
// QtGui
#include <QFont>

#include "krsort.h"

class FileItem;
class KrInterView;
class KrViewProperties;

/**
 * @brief The list model for all panel views.
 */
class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ListModel(KrInterView *);
    ~ListModel() override;

    inline bool ready() const
    {
        return _ready;
    }
    void populate(const QList<FileItem *> &files, FileItem *dummy);
    QModelIndex addItem(FileItem *);
    void removeItem(FileItem *);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setExtensionEnabled(bool exten)
    {
        _extensionEnabled = exten;
    }
    const KrViewProperties *properties() const;
    void sort()
    {
        sort(lastSortOrder(), lastSortDir());
    }
    void clear(bool emitLayoutChanged = true);
    QList<FileItem *> fileItems()
    {
        return _fileItems;
    }
    FileItem *fileItemAt(const QModelIndex &index);
    FileItem *dummyFileItem() const
    {
        return _dummyFileItem;
    }
    const QModelIndex &fileItemIndex(const FileItem *);
    const QModelIndex &nameIndex(const QString &);
    const QModelIndex &indexFromUrl(const QUrl &url);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void emitChanged()
    {
        emit layoutChanged();
    }
    Qt::SortOrder lastSortDir() const;
    int lastSortOrder() const;
    void setAlternatingTable(bool altTable)
    {
        _alternatingTable = altTable;
    }

public slots:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected:
    KrSort::LessThanFunc lessThanFunc()
    {
        return KrSort::itemLessThan;
    }
    KrSort::LessThanFunc greaterThanFunc() const
    {
        return KrSort::itemGreaterThan;
    }
    QVariant customSortData(FileItem *) const
    {
        return QVariant();
    }
    KrSort::Sorter createSorter();
    QString nameWithoutExtension(const FileItem *fileitem, bool checkEnabled = true) const;

private:
    void updateIndices(FileItem *file, int index);
    QString toolTipText(FileItem *fileItem) const;
    static QString dateText(time_t time);

    QList<FileItem *> _fileItems;
    QHash<FileItem *, QModelIndex> _fileItemNdx;
    QHash<QString, QModelIndex> _nameNdx;
    QHash<QUrl, QModelIndex> _urlNdx;
    bool _extensionEnabled;
    KrInterView *_view;
    FileItem *_dummyFileItem;
    bool _ready;
    QFont _defaultFont;
    bool _justForSizeHint;
    bool _alternatingTable;
};

#endif // __listmodel__
