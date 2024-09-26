/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRQUERY_H
#define KRQUERY_H

// QtCore
#include <QDateTime>
#include <QElapsedTimer>
#include <QStringList>
#include <QUrl>

#include <KConfigGroup>
#include <KIO/Global>
#include <KIO/Job>

class QTextCodec;

class FileItem;

/**
 * @brief A search query for files
 *
 * Can be used for finding or selecting files and folders by multiple limiting search criteria
 */
class KrQuery : public QObject
{
    Q_OBJECT

public:
    // null query
    KrQuery();
    // query only with name filter
    explicit KrQuery(const QString &name, bool matchCase = true);
    // copy constructor
    KrQuery(const KrQuery &);
    // let operator
    KrQuery &operator=(const KrQuery &);
    // destructor
    ~KrQuery() override;

    // load parameters from config
    void load(const KConfigGroup &cfg);
    // save parameters to config
    void save(KConfigGroup cfg);

    // matching a file with the query
    bool match(FileItem *file) const; // checks if the given fileItem object matches the conditions
    // matching a name with the query
    bool match(const QString &name) const; // matching the filename only
    // matching the name of the directory
    bool matchDirName(const QString &name) const;

    // sets the text for name filtering
    void setNameFilter(const QString &text, bool cs = true);
    // returns the current filter mask
    const QString &nameFilter() const
    {
        return origFilter;
    }
    // returns whether the filter is case sensitive
    bool isCaseSensitive()
    {
        return matchesCaseSensitive;
    }

    // returns if the filter is null (was cancelled)
    bool isNull()
    {
        return bNull;
    }

    // sets the content part of the query
    void setContent(const QString &content, bool cs = true, bool wholeWord = false, const QString &encoding = QString(), bool regExp = false);
    const QString content()
    {
        return contain;
    }

    // sets the minimum file size limit
    void setMinimumFileSize(KIO::filesize_t);
    // sets the maximum file size limit
    void setMaximumFileSize(KIO::filesize_t);

    // sets the time the file newer than
    void setNewerThan(time_t time);
    // sets the time the file older than
    void setOlderThan(time_t time);

    // sets the owner
    void setOwner(const QString &ownerIn);
    // sets the group
    void setGroup(const QString &groupIn);
    // sets the permissions
    void setPermissions(const QString &permIn);

    // sets the mimetype for the query
    // type, must be one of the following:
    // 1. a valid mime type name
    // 2. one of: i18n("Archives"),   i18n("Folders"), i18n("Image Files")
    //            i18n("Text Files"), i18n("Video Files"), i18n("Audio Files")
    // 3. i18n("Custom") in which case you must supply a list of valid mime-types
    //    in the member QStringList customType
    void setMimeType(const QString &typeIn, QStringList customList = QStringList());
    // true if setMimeType was called
    bool hasMimeType()
    {
        return type.isEmpty();
    }

    // sets the search in archive flag
    void setSearchInArchives(bool flag)
    {
        inArchive = flag;
    }
    // gets the search in archive flag
    bool searchInArchives()
    {
        return inArchive;
    }
    // sets the recursive flag
    void setRecursive(bool flag)
    {
        recurse = flag;
    }
    // gets the recursive flag
    bool isRecursive()
    {
        return recurse;
    }
    // sets whether to follow symbolic links
    void setFollowLinks(bool flag)
    {
        followLinksP = flag;
    }
    // gets whether to follow symbolic links
    bool followLinks()
    {
        return followLinksP;
    }

    // sets the folder names which the searcher will exclude from traversing
    void setExcludeFolderNames(const QStringList &urls);
    // gets the folder names which the searcher excludes
    const QStringList excludeFolderNames()
    {
        return excludedFolderNames;
    }
    // sets the folders where the searcher will search
    void setSearchInDirs(const QList<QUrl> &urls);
    // gets the folders where the searcher searches
    const QList<QUrl> &searchInDirs()
    {
        return whereToSearch;
    }
    // sets the folders where search is not permitted
    void setDontSearchInDirs(const QList<QUrl> &urls);
    // gets the folders where search is not permitted
    const QList<QUrl> &dontSearchInDirs()
    {
        return whereNotToSearch;
    }
    // checks if a URL is excluded
    bool isExcluded(const QUrl &url);
    // gives whether we search for content
    bool isContentSearched() const
    {
        return !contain.isEmpty();
    }

    bool checkLine(const QString &line, bool backwards = false) const;
    const QString &foundText() const
    {
        return lastSuccessfulGrep;
    }
    qsizetype matchIndex() const
    {
        return lastSuccessfulGrepMatchIndex;
    }
    qsizetype matchLength() const
    {
        return lastSuccessfulGrepMatchLength;
    }

protected:
    // important to know whether the event processor is connected
    void connectNotify(const QMetaMethod &signal) override;
    // important to know whether the event processor is connected
    void disconnectNotify(const QMetaMethod &signal) override;

protected:
    QStringList matches; // what to search
    QStringList excludes; // what to exclude
    QStringList includedDirs; // what dirs to include
    QStringList excludedDirs; // what dirs to exclude
    bool matchesCaseSensitive;

    bool bNull; // flag if the query is null

    QString contain; // file must contain this string
    bool containCaseSensetive;
    bool containWholeWord;
    bool containRegExp;

    KIO::filesize_t minSize;
    KIO::filesize_t maxSize;

    time_t newerThen;
    time_t olderThen;

    QString owner;
    QString group;
    QString perm;

    QString type;
    QStringList customType;

    bool inArchive; // if true- search in archive.
    bool recurse; // if true recurse ob sub-dirs...
    bool followLinksP;

    QStringList excludedFolderNames; // substrings of paths where not to search
    QList<QUrl> whereToSearch; // directories to search
    QList<QUrl> whereNotToSearch; // directories NOT to search

signals:
    void status(const QString &name);
    void processEvents(bool &stopped);

private:
    bool matchCommon(const QString &, const QStringList &, const QStringList &) const;
    bool checkPerm(QString perm) const;
    bool checkType(const QString &mime) const;
    bool containsContent(const QString &file) const;
    bool containsContent(const QUrl &url) const;
    bool checkBuffer(const char *data, qsizetype len) const;
    bool checkTimer() const;
    QStringList split(QString);

private slots:
    void containsContentData(KIO::Job *, const QByteArray &);
    void containsContentFinished(KJob *);

private:
    QString origFilter;
    mutable bool busy;
    mutable bool containsContentResult;
    mutable char *receivedBuffer;
    mutable qsizetype receivedBufferLen;
    mutable QString lastSuccessfulGrep;
    mutable qsizetype lastSuccessfulGrepMatchIndex;
    mutable qsizetype lastSuccessfulGrepMatchLength;
    mutable QString fileName;
    mutable KIO::filesize_t receivedBytes;
    mutable KIO::filesize_t totalBytes;
    mutable int processEventsConnected;
    mutable QElapsedTimer timer;

    QTextCodec *codec;

    const char *encodedEnter;
    qsizetype encodedEnterLen;
    QByteArray encodedEnterArray;
};

#endif
