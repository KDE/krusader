/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSORT_H
#define KRSORT_H

#include <sys/types.h>

// QtCore
#include <QString>
#include <QVariant>
#include <QVector>

class FileItem;
class KrViewProperties;

/** Implements sorting for the panel list model. */
namespace KrSort
{

class SortProps
{
public:
    SortProps()
    {
    }
    SortProps(const SortProps &other)
    {
        init(other.fileitem(), other.column(), other.properties(), other.isDummy(), other.isAscending(), other.originalIndex(), other.customData());
    }
    SortProps(FileItem *fileitem, int col, const KrViewProperties *props, bool isDummy, bool asc, int origNdx, QVariant customData)
    {
        init(fileitem, col, props, isDummy, asc, origNdx, customData);
    }

    inline int column() const
    {
        return _col;
    }
    inline const KrViewProperties *properties() const
    {
        return _prop;
    }
    inline bool isDummy() const
    {
        return _isdummy;
    }
    inline bool isAscending() const
    {
        return _ascending;
    }
    inline QString name() const
    {
        return _name;
    }
    inline QString extension() const
    {
        return _ext;
    }
    inline FileItem *fileitem() const
    {
        return _fileItem;
    }
    inline int originalIndex() const
    {
        return _index;
    }
    inline QString data() const
    {
        return _data;
    }
    inline const QVariant &customData() const
    {
        return _customData;
    }

private:
    void init(FileItem *fileitem, int col, const KrViewProperties *props, bool isDummy, bool asc, int origNdx, QVariant customData);

    int _col;
    const KrViewProperties *_prop;
    bool _isdummy;
    FileItem *_fileItem;
    bool _ascending;
    QString _name;
    QString _ext;
    int _index;
    QString _data;
    QVariant _customData;
};

bool compareTexts(QString aS1, QString aS2, const KrViewProperties *_viewProperties, bool asc, bool isName);
bool itemLessThan(SortProps *sp, SortProps *sp2);
bool itemGreaterThan(SortProps *sp, SortProps *sp2);
bool compareTime(time_t time1, time_t time2, SortProps *sp, SortProps *sp2);

typedef bool (*LessThanFunc)(SortProps *, SortProps *);

class Sorter
{
public:
    Sorter(qsizetype reserveItems, const KrViewProperties *viewProperties, LessThanFunc lessThanFunc, LessThanFunc greaterThanFunc);
    Sorter(const Sorter &other);

    const QVector<SortProps *> &items() const
    {
        return _items;
    }
    void sort();
    void addItem(FileItem *fileitem, bool isDummy, int idx, QVariant customData);
    int insertIndex(FileItem *fileitem, bool isDummy, QVariant customData);

private:
    bool descending() const;

    const KrViewProperties *_viewProperties;
    QVector<SortProps *> _items;
    QVector<SortProps> _itemStore;
    LessThanFunc _lessThanFunc, _greaterThanFunc;
};

} // namespace KrSort

#endif // KRSORT_H
