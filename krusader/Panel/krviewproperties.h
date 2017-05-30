/*****************************************************************************
 * Copyright (C) 2017 Krusader Krew                                          *
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

#ifndef KRVIEWPROPERTIES_H
#define KRVIEWPROPERTIES_H

#include "../Filter/filtersettings.h"
#include "../FileSystem/krquery.h"

// QtCore
#include <QStringList>

class FilterSettings;
class KRQuery;

/**
 * This class is an interface class between KrView and KrViewItem
 *
 * In order for KrViewItem to be as independent as possible, KrView holds
 * an instance of this class, and fills it with the correct data. A reference
 * to this should be given to each KrViewItem, which then queries it for
 * information regarding how things should be displayed in the current view.
 *
 * Every property that the item needs to know about the view must be here!
 */
class KrViewProperties
{
public:
    enum PropertyType {
        NoProperty = 0x0,
        PropIconSize = 0x1,
        PropShowPreviews = 0x2,
        PropSortMode = 0x4,
        PropColumns = 0x8,
        PropFilter = 0x10,
        AllProperties = PropIconSize | PropShowPreviews | PropSortMode | PropColumns | PropFilter
    };
    enum ColumnType {
        NoColumn = -1,
        Name = 0x0,
        Ext = 0x1,
        Size = 0x2,
        Type = 0x3,
        Modified = 0x4,
        Permissions = 0x5,
        KrPermissions = 0x6,
        Owner = 0x7,
        Group = 0x8,
        Changed = 0x9,
        Accessed = 0xa,
        MAX_COLUMNS = 0x0b
    };
    enum SortOptions {
        Descending = 0x200,
        DirsFirst = 0x400,
        IgnoreCase = 0x800,
        AlwaysSortDirsByName = 0x1000,
        LocaleAwareSort = 0x2000
    };
    enum SortMethod {
        Alphabetical = 0x1,
        AlphabeticalNumbers = 0x2,
        CharacterCode = 0x4,
        CharacterCodeNumbers = 0x8,
        Krusader = 0x10
    };
    enum FilterSpec { Dirs = 0x1, Files = 0x2, All = 0x3, Custom = 0x4 };

    KrViewProperties(bool displayIcons, bool numericPermissions, SortOptions sortOptions,
                     SortMethod sortMethod, bool humanReadableSize,
                     bool localeAwareCompareIsCaseSensitive, QStringList atomicExtensions);

    const bool numericPermissions; // show full permission column as octal numbers
    const bool displayIcons;             // true if icons should be displayed in this view

    ColumnType sortColumn;
    SortOptions sortOptions;

    const SortMethod sortMethod; // sort method for names and extensions

    FilterSpec filter;     // what items to show (all, custom, exec)
    KRQuery filterMask;    // what items to show (*.cpp, *.h etc)
    FilterSettings filterSettings;
    bool filterApplysToDirs;

    const bool localeAwareCompareIsCaseSensitive; // mostly, it is not! depends on LC_COLLATE
    const bool humanReadableSize;                 // display size as KB, MB or just as a long number
    // list of strings, which will be treated as one extension. Must start with a dot.
    const QStringList atomicExtensions;
    int numberOfColumns;                          // the number of columns in the brief view
};

#endif // KRVIEWPROPERTIES_H
