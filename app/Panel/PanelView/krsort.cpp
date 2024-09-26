/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krsort.h"

#include <utility>

#include "../FileSystem/fileitem.h"
#include "krview.h"

namespace KrSort
{

void SortProps::init(FileItem *fileitem, int col, const KrViewProperties *props, bool isDummy, bool asc, int origNdx, QVariant customData)
{
    _col = col;
    _prop = props;
    _isdummy = isDummy;
    _ascending = asc;
    _fileItem = fileitem;
    _index = origNdx;
    _name = fileitem->getName();
    _customData = std::move(customData);

    if (_prop->sortOptions & KrViewProperties::IgnoreCase)
        _name = _name.toLower();

    switch (_col) {
    case KrViewProperties::Ext: {
        if (fileitem->isDir()) {
            _ext = "";
        } else {
            // check if the file has an extension
            const QString &fileitemName = fileitem->getName();
            qsizetype loc = fileitemName.lastIndexOf('.');
            if (loc > 0) { // avoid mishandling of .bashrc and friend
                // check if it has one of the predefined 'atomic extensions'
                for (const auto &atomicExtension : props->atomicExtensions) {
                    if (fileitemName.endsWith(atomicExtension) && fileitemName != atomicExtension) {
                        loc = fileitemName.length() - atomicExtension.length();
                        break;
                    }
                }
                _ext = _name.mid(loc);
            } else
                _ext = "";
        }
        break;
    }
    case KrViewProperties::Type:
        _data = isDummy ? "" : KrView::mimeTypeText(fileitem);
        break;
    case KrViewProperties::Permissions:
        _data = isDummy ? "" : KrView::permissionsText(properties(), fileitem);
        break;
    case KrViewProperties::KrPermissions:
        _data = isDummy ? "" : KrView::krPermissionText(fileitem);
        break;
    case KrViewProperties::Owner:
        _data = isDummy ? "" : fileitem->getOwner();
        break;
    case KrViewProperties::Group:
        _data = isDummy ? "" : fileitem->getGroup();
        break;
    default:
        break;
    }
}

// compares numbers within two strings
int compareNumbers(QString &aS1, int &aPos1, QString &aS2, int &aPos2)
{
    int res = 0;
    int start1 = aPos1;
    int start2 = aPos2;
    while (aPos1 < aS1.length() && aS1.at(aPos1).isDigit())
        aPos1++;
    while (aPos2 < aS2.length() && aS2.at(aPos2).isDigit())
        aPos2++;
    // the left-most difference determines what's bigger
    int i1 = aPos1 - 1;
    int i2 = aPos2 - 1;
    for (; i1 >= start1 || i2 >= start2; i1--, i2--) {
        int c1 = 0;
        int c2 = 0;
        if (i1 >= start1)
            c1 = aS1.at(i1).digitValue();
        if (i2 >= start2)
            c2 = aS2.at(i2).digitValue();
        if (c1 < c2)
            res = -1;
        else if (c1 > c2)
            res = 1;
    }
    return res;
}

bool compareTextsAlphabetical(QString &aS1, QString &aS2, const KrViewProperties *_viewProperties, bool aNumbers)
{
    int lPositionS1 = 0;
    int lPositionS2 = 0;
    // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
    bool lUseLocaleAware = ((_viewProperties->sortOptions & KrViewProperties::IgnoreCase) || _viewProperties->localeAwareCompareIsCaseSensitive)
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
            if (j != 0)
                return j < 0;
        } else if (lUseLocaleAware
                   && ((lchar1 >= QChar(128) && ((lchar2 >= 'A' && lchar2 <= 'Z') || (lchar2 >= 'a' && lchar2 <= 'z') || lchar2 >= QChar(128)))
                       || (lchar2 >= QChar(128) && ((lchar1 >= 'A' && lchar1 <= 'Z') || (lchar1 >= 'a' && lchar1 <= 'z') || lchar1 >= QChar(128))))) {
            // use localeAwareCompare when a unicode character is encountered
            j = QString::localeAwareCompare(lchar1, lchar2);
            if (j != 0)
                return j < 0;
            lPositionS1++;
            lPositionS2++;
        } else {
            // if characters are latin or localeAwareCompare is not case sensitive then use simple characters compare is enough
            if (lchar1 < lchar2)
                return true;
            if (lchar1 > lchar2)
                return false;
            lPositionS1++;
            lPositionS2++;
        }
        // at this point strings are equal, check if ends of strings are reached
        if (lPositionS1 == aS1.length() && lPositionS2 == aS2.length())
            return false;
        if (lPositionS1 == aS1.length() && lPositionS2 < aS2.length())
            return true;
        if (lPositionS1 < aS1.length() && lPositionS2 == aS2.length())
            return false;
    }
}

bool compareTextsCharacterCode(QString &aS1, QString &aS2, const KrViewProperties *_viewProperties, bool aNumbers)
{
    Q_UNUSED(_viewProperties);

    int lPositionS1 = 0;
    int lPositionS2 = 0;
    while (true) {
        // detect numbers
        if (aNumbers && aS1[lPositionS1].isDigit() && aS2[lPositionS2].isDigit()) {
            int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
            if (j != 0)
                return j < 0;
        } else {
            if (aS1[lPositionS1] < aS2[lPositionS2])
                return true;
            if (aS1[lPositionS1] > aS2[lPositionS2])
                return false;
            lPositionS1++;
            lPositionS2++;
        }
        // at this point strings are equal, check if ends of strings are reached
        if (lPositionS1 == aS1.length() && lPositionS2 == aS2.length())
            return false;
        if (lPositionS1 == aS1.length() && lPositionS2 < aS2.length())
            return true;
        if (lPositionS1 < aS1.length() && lPositionS2 == aS2.length())
            return false;
    }
}

bool compareTextsKrusader(const QString &aS1, const QString &aS2, const KrViewProperties *_viewProperties)
{
    // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
    if (((_viewProperties->sortOptions & KrViewProperties::IgnoreCase) || _viewProperties->localeAwareCompareIsCaseSensitive)
        && (_viewProperties->sortOptions & KrViewProperties::LocaleAwareSort))
        return QString::localeAwareCompare(aS1, aS2) < 0;
    else
        // if localeAwareCompare is not case sensitive then use simple compare is enough
        return QString::compare(aS1, aS2) < 0;
}

bool compareTexts(QString aS1, QString aS2, const KrViewProperties *_viewProperties, bool asc, bool isName)
{
    // check empty strings
    if (aS1.length() == 0 && aS2.length() == 0) {
        return false;
    } else if (aS1.length() == 0) {
        return true;
    } else if (aS2.length() == 0) {
        return false;
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
        return compareTextsKrusader(aS1, aS2, _viewProperties);
    }
}

bool itemLessThan(SortProps *sp, SortProps *sp2)
{
    FileItem *file1 = sp->fileitem();
    FileItem *file2 = sp2->fileitem();
    bool isdir1 = file1->isDir();
    bool isdir2 = file2->isDir();
    bool dirsFirst = sp->properties()->sortOptions & KrViewProperties::DirsFirst;
    bool alwaysSortDirsByName = sp->properties()->sortOptions & KrViewProperties::AlwaysSortDirsByName && dirsFirst && isdir1 && isdir2;

    if (dirsFirst) {
        if (isdir1 && !isdir2)
            return sp->isAscending();
        if (isdir2 && !isdir1)
            return !sp->isAscending();
    }

    if (sp->isDummy())
        return sp->isAscending();
    if (sp2->isDummy())
        return !sp->isAscending();

    int column = sp->column();

    if (dirsFirst && isdir1 && isdir2 && alwaysSortDirsByName) {
        alwaysSortDirsByName = !sp->isAscending();
        column = KrViewProperties::Name;
    }

    switch (column) {
    case KrViewProperties::Name:
        return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true) ^ alwaysSortDirsByName;
    case KrViewProperties::Ext:
        if (sp->extension() == sp2->extension())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return compareTexts(sp->extension(), sp2->extension(), sp->properties(), sp->isAscending(), true);
    case KrViewProperties::Size:
        if (file1->getSize() == file2->getSize())
            return compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
        return file1->getSize() < file2->getSize();
    case KrViewProperties::Modified:
        return compareTime(file1->getModificationTime(), file2->getModificationTime(), sp, sp2);
    case KrViewProperties::Changed:
        return compareTime(file1->getChangeTime(), file2->getChangeTime(), sp, sp2);
    case KrViewProperties::Accessed:
        return compareTime(file1->getAccessTime(), file2->getAccessTime(), sp, sp2);
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

bool compareTime(time_t time1, time_t time2, SortProps *sp, SortProps *sp2)
{
    return time1 != time2 ? time1 < time2 : compareTexts(sp->name(), sp2->name(), sp->properties(), sp->isAscending(), true);
}

bool itemGreaterThan(SortProps *sp, SortProps *sp2)
{
    return !itemLessThan(sp, sp2);
}

Sorter::Sorter(qsizetype reserveItems, const KrViewProperties *viewProperties, LessThanFunc lessThanFunc, LessThanFunc greaterThanFunc)
    : _viewProperties(viewProperties)
    , _lessThanFunc(lessThanFunc)
    , _greaterThanFunc(greaterThanFunc)
{
    _items.reserve(reserveItems);
    _itemStore.reserve(reserveItems);
}

void Sorter::addItem(FileItem *fileitem, bool isDummy, int idx, QVariant customData)
{
    _itemStore << SortProps(fileitem, _viewProperties->sortColumn, _viewProperties, isDummy, !descending(), idx, std::move(customData));
    _items << &_itemStore.last();
}

void Sorter::sort()
{
    std::stable_sort(_items.begin(), _items.end(), descending() ? _greaterThanFunc : _lessThanFunc);
}

int Sorter::insertIndex(FileItem *fileitem, bool isDummy, QVariant customData)
{
    SortProps props(fileitem, _viewProperties->sortColumn, _viewProperties, isDummy, !descending(), -1, std::move(customData));
    const QVector<SortProps *>::iterator it = std::lower_bound(_items.begin(), _items.end(), &props, descending() ? _greaterThanFunc : _lessThanFunc);

    if (it != _items.end())
        return static_cast<int>(_items.indexOf((*it)));
    else
        return static_cast<int>(_items.count());
}

bool Sorter::descending() const
{
    return _viewProperties->sortOptions & KrViewProperties::Descending;
}

} // namespace KrSort
