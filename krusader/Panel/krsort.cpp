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

#include "krsort.h"

#include "krview.h"
#include "../VFS/krpermhandler.h"


namespace KrSort {

void SortProps::init(vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc, int origNdx, QVariant customData) {
    _col = col;
    _prop = props;
    _isdummy = isDummy;
    _ascending = asc;
    _vfile = vf;
    _index = origNdx;
    _name = vf->vfile_getName();
    _customData = customData;

    if(_prop->sortOptions & KrViewProperties::IgnoreCase)
        _name = _name.toLower();

    switch (_col) {
    case KrViewProperties::Ext: {
        if (vf->vfile_isDir()) {
            _ext = "";
        } else {
            // check if the file has an extension
            const QString& vfName = vf->vfile_getName();
            int loc = vfName.lastIndexOf('.');
            if (loc > 0) { // avoid mishandling of .bashrc and friend
                // check if it has one of the predefined 'atomic extensions'
                for (QStringList::const_iterator i = props->atomicExtensions.begin(); i != props->atomicExtensions.end(); ++i) {
                    if (vfName.endsWith(*i) && vfName != *i) {
                        loc = vfName.length() - (*i).length();
                        break;
                    }
                }
                _ext = _name.mid(loc);
            } else
                _ext = "";
        }
        break;
    }
    case KrViewProperties::Type: {
        if (isDummy)
            _data = "";
        else {
            KMimeType::Ptr mt = KMimeType::mimeType(vf->vfile_getMime());
            if (mt)
                _data = mt->comment();
        }
        break;
    }
    case KrViewProperties::Permissions: {
        if (isDummy)
            _data = "";
        else {
            if (properties()->numericPermissions) {
                QString perm;
                _data = perm.sprintf("%.4o", vf->vfile_getMode() & PERM_BITMASK);
            } else
                _data = vf->vfile_getPerm();
        }
        break;
    }
    case KrViewProperties::KrPermissions: {
        if (isDummy)
            _data = "";
        else {
            _data = KrView::krPermissionString(vf);
        }
        break;
    }
    case KrViewProperties::Owner: {
        if (isDummy)
            _data = "";
        else
            _data = vf->vfile_getOwner();
    }
    case KrViewProperties::Group: {
        if (isDummy)
            _data = "";
        else
            _data = vf->vfile_getGroup();
    }
    default:
        break;
    }
}


// compares numbers within two strings
int compareNumbers(QString& aS1, int& aPos1, QString& aS2, int& aPos2)
{
    int res = 0;
    int start1 = aPos1;
    int start2 = aPos2;
    while (aPos1 < aS1.length() && aS1.at(aPos1).isDigit()) aPos1++;
    while (aPos2 < aS2.length() && aS2.at(aPos2).isDigit()) aPos2++;
    // the left-most difference determines what's bigger
    int i1 = aPos1 - 1;
    int i2 = aPos2 - 1;
    for (; i1 >= start1 || i2 >= start2; i1--, i2--) {
        int c1 = 0;
        int c2 = 0;
        if (i1 >= start1) c1 = aS1.at(i1).digitValue();
        if (i2 >= start2) c2 = aS2.at(i2).digitValue();
        if (c1 < c2) res = -1;
        else if (c1 > c2) res = 1;
    }
    return res;
}

bool compareTextsAlphabetical(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool aNumbers)
{
    int lPositionS1 = 0;
    int lPositionS2 = 0;
    // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
    bool lUseLocaleAware = ((_viewProperties->sortOptions & KrViewProperties::IgnoreCase)
                || _viewProperties->localeAwareCompareIsCaseSensitive)
            && (_viewProperties->sortOptions & KrViewProperties::LocaleAwareSort);
    int j = 0;
    QChar lchar1;
    QChar lchar2;
    while (true) {
        lchar1 = aS1[lPositionS1];
        lchar2 = aS2[lPositionS2];
        // detect numbers
        if (aNumbers && lchar1.isDigit() && lchar2.isDigit()) {
            int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
            if (j != 0) return j < 0;
        } else if (lUseLocaleAware
                   &&
                   ((lchar1 >= 128
                     && ((lchar2 >= 'A' && lchar2 <= 'Z') || (lchar2 >= 'a' && lchar2 <= 'z') || lchar2 >= 128))
                    ||
                    (lchar2 >= 128
                     && ((lchar1 >= 'A' && lchar1 <= 'Z') || (lchar1 >= 'a' && lchar1 <= 'z') || lchar1 >= 128))
                   )
                  ) {
            // use localeAwareCompare when a unicode character is encountered
            j = QString::localeAwareCompare(lchar1, lchar2);
            if (j != 0) return j < 0;
            lPositionS1++;
            lPositionS2++;
        } else {
            // if characters are latin or localeAwareCompare is not case sensitive then use simple characters compare is enough
            if (lchar1 < lchar2) return true;
            if (lchar1 > lchar2) return false;
            lPositionS1++;
            lPositionS2++;
        }
        // at this point strings are equal, check if ends of strings are reached
        if (lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return false;
        if (lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return true;
        if (lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return false;
    }
}

bool compareTextsCharacterCode(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool aNumbers)
{
    int lPositionS1 = 0;
    int lPositionS2 = 0;
    while (true) {
        // detect numbers
        if (aNumbers && aS1[lPositionS1].isDigit() && aS2[lPositionS2].isDigit()) {
            int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
            if (j != 0) return j < 0;
        } else {
            if (aS1[lPositionS1] < aS2[lPositionS2]) return true;
            if (aS1[lPositionS1] > aS2[lPositionS2]) return false;
            lPositionS1++;
            lPositionS2++;
        }
        // at this point strings are equal, check if ends of strings are reached
        if (lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return false;
        if (lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return true;
        if (lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return false;
    }
}

bool compareTextsKrusader(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool asc, bool isName)
{
    // ensure "hidden" before others
    if (isName) {
        if (aS1[0] == '.' && aS2[0] != '.') return asc;
        if (aS1[0] != '.' && aS2[0] == '.') return !asc;
    }

    // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
    if (((_viewProperties->sortOptions & KrViewProperties::IgnoreCase)
                || _viewProperties->localeAwareCompareIsCaseSensitive)
            && (_viewProperties->sortOptions & KrViewProperties::LocaleAwareSort))
        return QString::localeAwareCompare(aS1, aS2) < 0;
    else
        // if localeAwareCompare is not case sensitive then use simple compare is enough
        return QString::compare(aS1, aS2) < 0;
}

bool compareTexts(QString aS1, QString aS2, const KrViewProperties * _viewProperties, bool asc, bool isName)
{
    //check empty strings
    if (aS1.length() == 0) {
        return false;
    } else {
        if (aS2.length() == 0)
            return true;
    }

    if (isName) {
        if (aS1 == "..") {
            return !asc;
        } else {
            if (aS2 == "..")
                return asc;
        }
    }

    switch (_viewProperties->sortMethod) {
    case KrViewProperties::Alphabetical:
        return compareTextsAlphabetical(aS1, aS2, _viewProperties, false);
    case KrViewProperties::AlphabeticalNumbers:
        return compareTextsAlphabetical(aS1, aS2, _viewProperties, true);
    case KrViewProperties::CharacterCode:
        return compareTextsCharacterCode(aS1, aS2, _viewProperties, false);
    case KrViewProperties::CharacterCodeNumbers:
        return compareTextsCharacterCode(aS1, aS2, _viewProperties, true);
    case KrViewProperties::Krusader:
    default:
        return compareTextsKrusader(aS1, aS2, _viewProperties, asc, isName);
    }
}

bool itemLessThan(SortProps *sp, SortProps *sp2)
{
    vfile * file1 = sp->vf();
    vfile * file2 = sp2->vf();
    bool isdir1 = file1->vfile_isDir();
    bool isdir2 = file2->vfile_isDir();

    if(sp->properties()->sortOptions  & KrViewProperties::DirsFirst) {
        if (isdir1 && !isdir2)
            return sp->isAscending();
        if (isdir2 && !isdir1)
            return !sp->isAscending();
    }

    if (sp->isDummy())
        return sp->isAscending();
    if (sp2->isDummy())
        return !sp->isAscending();

    bool alwaysSortDirsByName = (sp->properties()->sortOptions & KrViewProperties::AlwaysSortDirsByName);
    int column = sp->column();
    if (alwaysSortDirsByName)
        column = KrViewProperties::Name;

    switch (sp->column()) {
    case KrViewProperties::Name:
        return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
    case KrViewProperties::Ext:
        if (sp->extension() == sp2->extension())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return compareTexts(sp->extension(), sp2->extension(), sp->properties(), sp->isAscending(), true);
    case KrViewProperties::Size:
        if (file1->vfile_getSize() == file2->vfile_getSize())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return file1->vfile_getSize() < file2->vfile_getSize();
    case KrViewProperties::Modified:
        if (file1->vfile_getTime_t() == file2->vfile_getTime_t())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return file1->vfile_getTime_t() < file2->vfile_getTime_t();
    case KrViewProperties::Type:
    case KrViewProperties::Permissions:
    case KrViewProperties::KrPermissions:
    case KrViewProperties::Owner:
    case KrViewProperties::Group:
        if (sp->data() == sp2->data())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return compareTexts(sp->data(), sp2->data(), sp->properties(), sp->isAscending(), true);
    }
    return sp->name() < sp2->name();
}

bool itemGreaterThan(SortProps *sp, SortProps *sp2)
{
    return !itemLessThan(sp, sp2);
}


void sort(QVector<SortProps*> &sorting, bool descending)
{
    qStableSort(sorting.begin(), sorting.end(),
                descending ? &itemGreaterThan : &itemLessThan);
}

Sorter::Sorter(int reserveItems, const KrViewProperties *viewProperties,
        LessThanFunc lessThanFunc, LessThanFunc greaterThanFunc) :
    _viewProperties(viewProperties),
    _lessThanFunc(lessThanFunc),
    _greaterThanFunc(greaterThanFunc)
 {
    _items.reserve(reserveItems);
    _itemStore.reserve(reserveItems);
 }
 
void Sorter::addItem(vfile *vf, bool isDummy, int idx, QVariant customData)
{
    _itemStore << SortProps(vf, _viewProperties->sortColumn, _viewProperties, isDummy, !descending(), idx, customData);
    _items << &_itemStore.last();
}

void Sorter::sort()
{
    qStableSort(_items.begin(), _items.end(),
                descending() ? _greaterThanFunc : _lessThanFunc);
}

int Sorter::insertIndex(vfile *vf, bool isDummy, QVariant customData)
{
    SortProps props(vf,  _viewProperties->sortColumn, _viewProperties, isDummy, !descending(), -1, customData);
    const QVector<SortProps*>::iterator it =
        qLowerBound(_items.begin(), _items.end(), &props,
                        descending() ? _greaterThanFunc : _lessThanFunc);

    if(it != _items.end())
         return _items.indexOf((*it));
    else
        return _items.count();
}

} // namespace KrSort
