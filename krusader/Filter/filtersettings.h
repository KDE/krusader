/***************************************************************************
                        filtersettings.h  -  description
                             -------------------
    copyright            : (C) 2011 + by Jan Lepper <jan_lepper@gmx.de>
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FILTERSETTINGS_H__
#define __FILTERSETTINGS_H__

#include "../VFS/krquery.h"

#include <kconfiggroup.h>

class QWidget;

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
    void load(KConfigGroup cfg);
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

    static void saveDate(QString key, const QDate &date, KConfigGroup &cfg);
    static time_t qdate2time_t(QDate d, bool start);

    bool valid;
    QString searchFor;
    bool searchForCase;
    QString mimeType;
    bool searchInArchives;
    bool recursive;
    bool followLinks;
    KUrl::List searchIn;
    KUrl::List dontSearchIn;
    bool remoteContentSearch;
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

#endif //__FILTERSETTINGS_H__
