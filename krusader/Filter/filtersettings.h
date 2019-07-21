/*****************************************************************************
 * Copyright (C) 2011 Jan Lepper <jan_lepper@gmx.de>                         *
 * Copyright (C) 2011-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef FILTERSETTINGS_H
#define FILTERSETTINGS_H

#include "../FileSystem/krquery.h"

#include <KConfigCore/KConfigGroup>


class FilterSettings
{
    friend class FilterTabs;
    friend class GeneralFilter;
    friend class AdvancedFilter;

public:
    FilterSettings();
    FilterSettings& operator=(const FilterSettings& other);

    bool isValid() const {
        return valid;
    }
    void load(const KConfigGroup& cfg);
    void save(KConfigGroup cfg) const;
    KRQuery toQuery() const;

private:
    enum SizeUnit {
        Byte = 0, KiloByte, MegaByte, GigaByte
    };

    enum TimeUnit {
        Day = 0, Week, Month, Year
    };

    class FileSize
    {
    public:
        FileSize() : amount(0), unit(Byte) {}
        FileSize& operator=(const FileSize &other);

        KIO::filesize_t size() const;

        KIO::filesize_t amount;
        SizeUnit unit;
    };

    class TimeSpan
    {
    public:
        TimeSpan() : amount(0), unit(Day) {}
        TimeSpan& operator=(const TimeSpan &other);

        int days() const;

        int amount;
        TimeUnit unit;
    };

    static void saveDate(const QString& key, const QDate &date, KConfigGroup &cfg);
    static time_t qdate2time_t(QDate d, bool start);

    bool valid;
    QString searchFor;
    bool searchForCase;
    QString mimeType;
    bool searchInArchives;
    bool recursive;
    bool followLinks;
    QList<QUrl> searchIn;
    QList<QUrl> dontSearchIn;
    QStringList excludeFolderNames;
    QString contentEncoding;
    QString containsText;
    bool containsTextCase;
    bool containsWholeWord;
    bool containsRegExp;
    bool minSizeEnabled;
    FileSize minSize;
    bool maxSizeEnabled;
    FileSize maxSize;
    bool modifiedBetweenEnabled;
    QDate modifiedBetween1;
    QDate modifiedBetween2;
    bool notModifiedAfterEnabled;
    QDate notModifiedAfter;
    bool modifiedInTheLastEnabled;
    TimeSpan modifiedInTheLast;
    TimeSpan notModifiedInTheLast;
    bool ownerEnabled;
    QString owner;
    bool groupEnabled;
    QString group;
    bool permissionsEnabled;
    QString permissions;
};

#endif //FILTERSETTINGS_H
