/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2011 Jan Lepper <jan_lepper@gmx.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtersettings.h"

#include "../krservices.h"

#include <QDebug>

#include <KCharsets>

FilterSettings::FileSize::FileSize(const FileSize &other) = default;

FilterSettings::FileSize &FilterSettings::FileSize::operator=(const FileSize &other) = default;

KIO::filesize_t FilterSettings::FileSize::size() const
{
    switch (unit) {
    case Byte:
        return amount;
    case KiloByte:
        return amount * 1024;
    case MegaByte:
        return amount * 1024 * 1024;
    case GigaByte:
        return amount * 1024 * 1024 * 1024;
    default:
        qWarning() << "invalid size unit: " << unit;
        return amount;
    }
}

FilterSettings::TimeSpan::TimeSpan(const TimeSpan &other) = default;

FilterSettings::TimeSpan &FilterSettings::TimeSpan::operator=(const TimeSpan &other) = default;

int FilterSettings::TimeSpan::days() const
{
    switch (unit) {
    case Day:
        return amount;
    case Week:
        return amount * 7;
    case Month:
        return amount * 30;
    case Year:
        return amount * 365;
    default:
        qWarning() << "invalid time unit: " << unit;
        return amount;
    }
}

FilterSettings::FilterSettings()
    : valid(false)
    , searchFor("*")
    , searchForCase(false)
    , searchInArchives(false)
    , recursive(false)
    , followLinks(false)
    , containsTextCase(false)
    , containsWholeWord(false)
    , containsRegExp(false)
    , minSizeEnabled(false)
    , maxSizeEnabled(false)
    , modifiedBetweenEnabled(false)
    , notModifiedAfterEnabled(false)
    , modifiedInTheLastEnabled(false)
    , ownerEnabled(false)
    , groupEnabled(false)
    , permissionsEnabled(false)
{
}

FilterSettings::FilterSettings(const FilterSettings &other) = default;

FilterSettings &FilterSettings::operator=(const FilterSettings &other) = default;

void FilterSettings::load(const KConfigGroup &cfg)
{
    *this = FilterSettings();
#define LOAD(key, var)                                                                                                                                         \
    {                                                                                                                                                          \
        var = cfg.readEntry(key, var);                                                                                                                         \
    }
    LOAD("IsValid", valid);
    if (!isValid())
        return;
    LOAD("SearchFor", searchFor);
    LOAD("MimeType", mimeType);
    LOAD("SearchInArchives", searchInArchives);
    LOAD("Recursive", recursive);
    LOAD("FollowLinks", followLinks);
    searchIn = KrServices::toUrlList(cfg.readEntry("SearchIn", QStringList()));
    dontSearchIn = KrServices::toUrlList(cfg.readEntry("DontSearchIn", QStringList()));
    excludeFolderNames = QStringList();
    LOAD("ContentEncoding", contentEncoding);
    LOAD("ContainsText", containsText);
    LOAD("ContainsTextCase", containsTextCase);
    LOAD("ContainsWholeWord", containsWholeWord);
    LOAD("ContainsRegExp", containsRegExp);
    LOAD("MinSizeEnabled", minSizeEnabled);
    LOAD("MinSizeAmount", minSize.amount);
    minSize.unit = static_cast<SizeUnit>(cfg.readEntry("MinSizeUnit", 0));
    LOAD("MaxSizeEnabled", maxSizeEnabled);
    LOAD("MaxSizeAmount", maxSize.amount);
    maxSize.unit = static_cast<SizeUnit>(cfg.readEntry("MaxSizeUnit", 0));
    LOAD("ModifiedBetweenEnabled", modifiedBetweenEnabled);
    LOAD("ModifiedBetween1", modifiedBetween1);
    LOAD("ModifiedBetween2", modifiedBetween2);
    LOAD("NotModifiedAfterEnabled", notModifiedAfterEnabled);
    LOAD("NotModifiedAfter", notModifiedAfter);
    LOAD("ModifiedInTheLastEnabled", modifiedInTheLastEnabled);
    LOAD("ModifiedInTheLastAmount", modifiedInTheLast.amount);
    modifiedInTheLast.unit = static_cast<TimeUnit>(cfg.readEntry("ModifiedInTheLastUnit", 0));
    LOAD("NotModifiedInTheLastAmount", notModifiedInTheLast.amount);
    notModifiedInTheLast.unit = static_cast<TimeUnit>(cfg.readEntry("NotModifiedInTheLastUnit", 0));
    LOAD("OwnerEnabled", ownerEnabled);
    LOAD("Owner", owner);
    LOAD("GroupEnabled", groupEnabled);
    LOAD("Group", group);
    LOAD("PermissionsEnabled", permissionsEnabled);
    LOAD("Permissions", permissions);
#undef LOAD
}

void FilterSettings::saveDate(const QString &key, const QDate &date, KConfigGroup &cfg)
{
    if (date.isValid())
        cfg.writeEntry(key, date);
    else
        cfg.deleteEntry(key);
}

void FilterSettings::save(KConfigGroup cfg) const
{
    cfg.writeEntry("IsValid", valid);
    if (!isValid())
        return;
    cfg.writeEntry("SearchFor", searchFor);
    cfg.writeEntry("MimeType", mimeType);
    cfg.writeEntry("SearchInArchives", searchInArchives);
    cfg.writeEntry("Recursive", recursive);
    cfg.writeEntry("FollowLinks", followLinks);
    cfg.writeEntry("SearchIn", KrServices::toStringList(searchIn));
    cfg.writeEntry("DontSearchIn", KrServices::toStringList(dontSearchIn));
    cfg.writeEntry("ContentEncoding", contentEncoding);
    cfg.writeEntry("ContainsText", containsText);
    cfg.writeEntry("ContainsTextCase", containsTextCase);
    cfg.writeEntry("ContainsWholeWord", containsWholeWord);
    cfg.writeEntry("ContainsRegExp", containsRegExp);
    cfg.writeEntry("MinSizeEnabled", minSizeEnabled);
    cfg.writeEntry("MinSizeAmount", minSize.amount);
    cfg.writeEntry("MinSizeUnit", static_cast<int>(minSize.unit));
    cfg.writeEntry("MaxSizeEnabled", maxSizeEnabled);
    cfg.writeEntry("MaxSizeAmount", maxSize.amount);
    cfg.writeEntry("MaxSizeUnit", static_cast<int>(maxSize.unit));
    cfg.writeEntry("ModifiedBetweenEnabled", modifiedBetweenEnabled);
    saveDate("ModifiedBetween1", modifiedBetween1, cfg);
    saveDate("ModifiedBetween2", modifiedBetween2, cfg);
    cfg.writeEntry("NotModifiedAfterEnabled", notModifiedAfterEnabled);
    saveDate("NotModifiedAfter", notModifiedAfter, cfg);
    cfg.writeEntry("ModifiedInTheLastEnabled", modifiedInTheLastEnabled);
    cfg.writeEntry("ModifiedInTheLastAmount", modifiedInTheLast.amount);
    cfg.writeEntry("ModifiedInTheLastUnit", static_cast<int>(modifiedInTheLast.unit));
    cfg.writeEntry("NotModifiedInTheLastAmount", notModifiedInTheLast.amount);
    cfg.writeEntry("NotModifiedInTheLastUnit", static_cast<int>(notModifiedInTheLast.unit));
    cfg.writeEntry("OwnerEnabled", ownerEnabled);
    cfg.writeEntry("Owner", owner);
    cfg.writeEntry("GroupEnabled", groupEnabled);
    cfg.writeEntry("Group", group);
    cfg.writeEntry("PermissionsEnabled", permissionsEnabled);
    cfg.writeEntry("Permissions", permissions);
}

// bool start: set it to true if this date is the beginning of the search,
// if it's the end, set it to false
time_t FilterSettings::qdate2time_t(QDate d, bool start)
{
    struct tm t;
    t.tm_sec = (start ? 0 : 59);
    t.tm_min = (start ? 0 : 59);
    t.tm_hour = (start ? 0 : 23);
    t.tm_mday = d.day();
    t.tm_mon = d.month() - 1;
    t.tm_year = d.year() - 1900;
    t.tm_wday = d.dayOfWeek() - 1; // actually ignored by mktime
    t.tm_yday = d.dayOfYear() - 1; // actually ignored by mktime
    t.tm_isdst = -1; // daylight saving time information isn't available

    return mktime(&t);
}

KrQuery FilterSettings::toQuery() const
{
    if (!isValid())
        return KrQuery();

    KrQuery query;

    ////////////// General Options //////////////

    query.setNameFilter(searchFor, searchForCase);

    query.setMimeType(mimeType);

    QString charset;
    if (!contentEncoding.isEmpty())
        charset = KCharsets::charsets()->encodingForName(contentEncoding);

    if (!containsText.isEmpty()) {
        query.setContent(containsText, containsTextCase, containsWholeWord, charset, containsRegExp);
    }

    query.setRecursive(recursive);
    query.setSearchInArchives(searchInArchives);
    query.setFollowLinks(followLinks);

    if (!searchIn.isEmpty())
        query.setSearchInDirs(searchIn);

    if (!dontSearchIn.isEmpty())
        query.setDontSearchInDirs(dontSearchIn);

    query.setExcludeFolderNames(excludeFolderNames);

    ////////////// Advanced Options //////////////

    if (minSizeEnabled)
        query.setMinimumFileSize(minSize.size());

    if (maxSizeEnabled)
        query.setMaximumFileSize(maxSize.size());

    if (modifiedBetweenEnabled) {
        if (modifiedBetween1.isValid())
            query.setNewerThan(qdate2time_t(modifiedBetween1, true));
        if (modifiedBetween2.isValid())
            query.setOlderThan(qdate2time_t(modifiedBetween2, false));
    } else if (notModifiedAfterEnabled && notModifiedAfter.isValid()) {
        query.setOlderThan(qdate2time_t(notModifiedAfter, false));
    } else if (modifiedInTheLastEnabled) {
        if (modifiedInTheLast.amount) {
            QDate d = QDate::currentDate().addDays((-1) * modifiedInTheLast.days());
            query.setNewerThan(qdate2time_t(d, true));
        }
        if (notModifiedInTheLast.amount) {
            QDate d = QDate::currentDate().addDays((-1) * notModifiedInTheLast.days());
            query.setOlderThan(qdate2time_t(d, true));
        }
    }

    if (ownerEnabled && !owner.isEmpty())
        query.setOwner(owner);
    if (groupEnabled && !group.isEmpty())
        query.setGroup(group);
    if (permissionsEnabled && !permissions.isEmpty())
        query.setPermissions(permissions);

    return query;
}
