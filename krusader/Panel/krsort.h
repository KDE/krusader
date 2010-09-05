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

#include <QString>
#include <QVector>


#define PERM_BITMASK (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

class KrViewProperties;
class vfile;

namespace KrSort {

class SortProps
{
public:
    SortProps(vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc, int origNdx);

    inline int column() {
        return _col;
    }
    inline const KrViewProperties * properties() {
        return _prop;
    }
    inline bool isDummy() {
        return _isdummy;
    }
    inline bool isAscending() {
        return _ascending;
    }
    inline QString name() {
        return _name;
    }
    inline QString extension() {
        return _ext;
    }
    inline vfile * vf() {
        return _vfile;
    }
    inline int originalIndex() {
        return _index;
    }
    inline QString data() {
        return _data;
    }

private:
    int _col;
    const KrViewProperties * _prop;
    bool _isdummy;
    vfile * _vfile;
    bool _ascending;
    QString _name;
    QString _ext;
    int _index;
    QString _data;
};


void sort(QVector<SortProps*> &sorting, bool descending);
QVector<SortProps*>::iterator lowerBound(QVector<SortProps*> &sorting, SortProps *item, bool descending);

}; // namespace KrSort

#endif // __KRSORT_H__
