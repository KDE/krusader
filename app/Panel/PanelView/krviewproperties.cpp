/*
    SPDX-FileCopyrightText: 2017-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewproperties.h"

#include <utility>

KrViewProperties::KrViewProperties(bool displayIcons,
                                   bool numericPermissions,
                                   KrViewProperties::SortOptions sortOptions,
                                   KrViewProperties::SortMethod sortMethod,
                                   bool humanReadableSize,
                                   bool localeAwareCompareIsCaseSensitive,
                                   QStringList atomicExtensions)
    : numericPermissions(numericPermissions)
    , displayIcons(displayIcons)
    , sortColumn(Name)
    , sortOptions(sortOptions)
    , sortMethod(sortMethod)
    , filter(KrViewProperties::All)
    , filterMask(KrQuery("*"))
    , filterApplysToDirs(false)
    , localeAwareCompareIsCaseSensitive(localeAwareCompareIsCaseSensitive)
    , humanReadableSize(humanReadableSize)
    , atomicExtensions(std::move(atomicExtensions))
    , numberOfColumns(1)
{
}
