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

#endif
