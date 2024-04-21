/*
    SPDX-FileCopyrightText: 2011 Jan Lepper <jan_lepper@gmx.de>
    SPDX-FileCopyrightText: 2011-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERSETTINGS_H
#define FILTERSETTINGS_H

#include "../FileSystem/krquery.h"

#include <KConfigGroup>

class FilterSettings
{
    friend class FilterTabs;
    friend class GeneralFilter;
    friend class AdvancedFilter;

public:
    FilterSettings();
    FilterSettings(const FilterSettings &other);
    FilterSettings &operator=(const FilterSettings &other);

    bool isValid() const
    {
        return valid;
    }
    void load(const KConfigGroup &cfg);
    void save(KConfigGroup cfg) const;
    KrQuery toQuery() const;

private:
    enum SizeUnit { Byte = 0, KiloByte, MegaByte, GigaByte };

    enum TimeUnit { Day = 0, Week, Month, Year };

    class FileSize
    {
    public:
        FileSize()
            : amount(0)
            , unit(Byte)
        {
        }
        FileSize(const FileSize &other);
        FileSize &operator=(const FileSize &other);

        KIO::filesize_t size() const;

        KIO::filesize_t amount;
        SizeUnit unit;
    };

    class TimeSpan
    {
    public:
        TimeSpan()
            : amount(0)
            , unit(Day)
        {
        }
        TimeSpan(const TimeSpan &other);
        TimeSpan &operator=(const TimeSpan &other);

        int days() const;

        int amount;
        TimeUnit unit;
    };

    static void saveDate(const QString &key, const QDate &date, KConfigGroup &cfg);
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

#endif // FILTERSETTINGS_H
