/*
    SPDX-FileCopyrightText: 2017-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewproperties.h"

#include <utility>

KrViewProperties::KrViewProperties(bool showIcons,
                                   bool useNumericPermissions,
                                   KrViewProperties::SortOptions initSortOptions,
                                   KrViewProperties::SortMethod initSortMethod,
                                   bool useHumanReadableSize,
                                   bool useLocaleAwareCompareIsCaseSensitive,
                                   QStringList initAtomicExtensions)
    : numericPermissions(useNumericPermissions)
    , displayIcons(showIcons)
    , sortColumn(Name)
    , sortOptions(initSortOptions)
    , sortMethod(initSortMethod)
    , filter(KrViewProperties::All)
    , filterMask(KrQuery("*"))
    , filterApplysToDirs(false)
    , localeAwareCompareIsCaseSensitive(useLocaleAwareCompareIsCaseSensitive)
    , humanReadableSize(useHumanReadableSize)
    , atomicExtensions(std::move(initAtomicExtensions))
    , numberOfColumns(1)
{
}
