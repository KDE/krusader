/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef __KRSORT_H__
#define __KRSORT_H__

#include "krview.h"

#include <QString>
#include <QVector>
#include <QVariant>

#define PERM_BITMASK (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

namespace KrSort {

class SortProps
{
public:
    SortProps() {}
    SortProps(const SortProps& other) {
        init(other.vf(), other.column(), other.properties(), other.isDummy(),
             other.isAscending(), other.originalIndex(), other.customData());
    }
    SortProps(vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc, int origNdx, QVariant customData) {
        init(vf, col, props, isDummy, asc, origNdx, customData);
    }

    inline int column() const {
        return _col;
    }
    inline const KrViewProperties * properties() const {
        return _prop;
    }
    inline bool isDummy() const  {
        return _isdummy;
    }
    inline bool isAscending() const {
        return _ascending;
    }
    inline QString name() const {
        return _name;
    }
    inline QString extension() const {
        return _ext;
    }
    inline vfile * vf() const {
        return _vfile;
    }
    inline int originalIndex() const {
        return _index;
    }
    inline QString data() const {
        return _data;
    }
    inline const QVariant& customData() const {
        return _customData;
    }

private:
    void init(vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc, int origNdx, QVariant customData);

    int _col;
    const KrViewProperties * _prop;
    bool _isdummy;
    vfile * _vfile;
    bool _ascending;
    QString _name;
    QString _ext;
    int _index;
    QString _data;
    QVariant _customData;
};


bool compareTexts(QString aS1, QString aS2, const KrViewProperties * _viewProperties, bool asc, bool isName);
bool itemLessThan(SortProps *sp, SortProps *sp2);
bool itemGreaterThan(SortProps *sp, SortProps *sp2);

typedef bool(*LessThanFunc)(SortProps*, SortProps*);

class Sorter
{
public:
    Sorter(int reserveItems, const KrViewProperties *viewProperties, LessThanFunc lessThanFunc, LessThanFunc greaterThanFunc);
    Sorter(const Sorter &other);

    const QVector<SortProps*> &items() const {
        return _items;
    }
    void sort();
    void addItem(vfile *vf, bool isDummy, int idx, QVariant customData);
    int insertIndex(vfile *vf, bool isDummy, QVariant customData);

private:
    bool descending() const {
        return _viewProperties->sortOptions & KrViewProperties::Descending;
    }

    const KrViewProperties *_viewProperties;
    QVector<SortProps*> _items;
    QVector<SortProps> _itemStore;
    LessThanFunc _lessThanFunc, _greaterThanFunc;
};


} // namespace KrSort

#endif // __KRSORT_H__
