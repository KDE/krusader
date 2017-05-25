/***************************************************************************
                                 krquery.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krquery.h"

// QtCore
#include <QFile>
#include <QMetaMethod>
#include <QRegExp>
#include <QTextCodec>

#include <KI18n/KLocalizedString>
#include <KIO/Job>
#include <KIOCore/KFileItem>
#include <KIOWidgets/KUrlCompletion>

#include "../Archive/krarchandler.h"
#include "fileitem.h"
#include "filesystem.h"
#include "krpermhandler.h"

#define STATUS_SEND_DELAY 250
#define MAX_LINE_LEN 1000

// set the defaults
KRQuery::KRQuery()
    : QObject(), matchesCaseSensitive(true), bNull(true), contain(QString()),
      containCaseSensetive(true), containWholeWord(false), containRegExp(false), minSize(0),
      maxSize(0), newerThen(0), olderThen(0), owner(QString()), group(QString()), perm(QString()),
      type(QString()), inArchive(false), recurse(true), followLinksP(true), receivedBuffer(0),
      receivedBufferLen(0), processEventsConnected(0), codec(QTextCodec::codecForLocale())
{
    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();
}

// set the defaults
KRQuery::KRQuery(const QString &name, bool matchCase)
    : QObject(), bNull(true), contain(QString()), containCaseSensetive(true),
      containWholeWord(false), containRegExp(false), minSize(0), maxSize(0), newerThen(0),
      olderThen(0), owner(QString()), group(QString()), perm(QString()), type(QString()),
      inArchive(false), recurse(true), followLinksP(true), receivedBuffer(0), receivedBufferLen(0),
      processEventsConnected(0), codec(QTextCodec::codecForLocale())
{
    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();

    setNameFilter(name, matchCase);
}

KRQuery::KRQuery(const KRQuery &that)
    : QObject(), receivedBuffer(0), receivedBufferLen(0), processEventsConnected(0)
{
    *this = that;
}

KRQuery::~KRQuery()
{
    if (receivedBuffer)
        delete[] receivedBuffer;
    receivedBuffer = 0;
}

KRQuery &KRQuery::operator=(const KRQuery &old)
{
    matches = old.matches;
    excludes = old.excludes;
    includedDirs = old.includedDirs;
    excludedDirs = old.excludedDirs;
    matchesCaseSensitive = old.matchesCaseSensitive;
    bNull = old.bNull;
    contain = old.contain;
    containCaseSensetive = old.containCaseSensetive;
    containWholeWord = old.containWholeWord;
    containRegExp = old.containRegExp;
    minSize = old.minSize;
    maxSize = old.maxSize;
    newerThen = old.newerThen;
    olderThen = old.olderThen;
    owner = old.owner;
    group = old.group;
    perm = old.perm;
    type = old.type;
    customType = old.customType;
    inArchive = old.inArchive;
    recurse = old.recurse;
    followLinksP = old.followLinksP;
    whereToSearch = old.whereToSearch;
    whereNotToSearch = old.whereNotToSearch;
    origFilter = old.origFilter;

    codec = old.codec;

    encodedEnterArray = old.encodedEnterArray;
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();

    return *this;
}

void KRQuery::load(KConfigGroup cfg)
{
    *this = KRQuery(); // reset parameters first

    if (cfg.readEntry("IsNull", true))
        return;

#define LOAD(key, var) (var = cfg.readEntry(key, var))
    LOAD("Matches", matches);
    LOAD("Excludes", excludes);
    LOAD("IncludedDirs", includedDirs);
    LOAD("ExcludedDirs", excludedDirs);
    LOAD("MatchesCaseSensitive", matchesCaseSensitive);
    LOAD("Contain", contain);
    LOAD("ContainCaseSensetive", containCaseSensetive);
    LOAD("ContainWholeWord", containWholeWord);
    LOAD("ContainRegExp", containRegExp);
    LOAD("MinSize", minSize);
    LOAD("MaxSize", maxSize);
    newerThen = QDateTime::fromString(
                    cfg.readEntry("NewerThan", QDateTime::fromTime_t(newerThen).toString()))
                    .toTime_t();
    olderThen = QDateTime::fromString(
                    cfg.readEntry("OlderThan", QDateTime::fromTime_t(olderThen).toString()))
                    .toTime_t();
    LOAD("Owner", owner);
    LOAD("Group", group);
    LOAD("Perm", perm);
    LOAD("Type", type);
    LOAD("CustomType", customType);
    LOAD("InArchive", inArchive);
    LOAD("Recurse", recurse);
    LOAD("FollowLinks", followLinksP);
    // KF5 TODO?
    // LOAD("WhereToSearch", whereToSearch);
    // LOAD("WhereNotToSearch", whereNotToSearch);
    LOAD("OrigFilter", origFilter);

    codec = QTextCodec::codecForName(cfg.readEntry("Codec", codec->name()));
    if (!codec)
        codec = QTextCodec::codecForLocale();

    LOAD("EncodedEnterArray", encodedEnterArray);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();
#undef LOAD

    bNull = false;
}

void KRQuery::save(KConfigGroup cfg)
{
    cfg.writeEntry("IsNull", bNull);

    if (bNull)
        return;

    cfg.writeEntry("Matches", matches);
    cfg.writeEntry("Excludes", excludes);
    cfg.writeEntry("IncludedDirs", includedDirs);
    cfg.writeEntry("ExcludedDirs", excludedDirs);
    cfg.writeEntry("MatchesCaseSensitive", matchesCaseSensitive);
    cfg.writeEntry("Contain", contain);
    cfg.writeEntry("ContainCaseSensetive", containCaseSensetive);
    cfg.writeEntry("ContainWholeWord", containWholeWord);
    cfg.writeEntry("ContainRegExp", containRegExp);
    cfg.writeEntry("MinSize", minSize);
    cfg.writeEntry("MaxSize", maxSize);
    cfg.writeEntry("NewerThan", QDateTime::fromTime_t(newerThen).toString());
    cfg.writeEntry("OlderThan", QDateTime::fromTime_t(olderThen).toString());
    cfg.writeEntry("Owner", owner);
    cfg.writeEntry("Group", group);
    cfg.writeEntry("Perm", perm);
    cfg.writeEntry("Type", type);
    cfg.writeEntry("CustomType", customType);
    cfg.writeEntry("InArchive", inArchive);
    cfg.writeEntry("Recurse", recurse);
    cfg.writeEntry("FollowLinks", followLinksP);
    // KF5 TODO?
    // cfg.writeEntry("WhereToSearch", whereToSearch);
    // cfg.writeEntry("WhereNotToSearch", whereNotToSearch);
    cfg.writeEntry("OrigFilter", origFilter);

    cfg.writeEntry("Codec", codec->name());
    cfg.writeEntry("EncodedEnterArray", encodedEnterArray);
    cfg.writeEntry("EncodedEnter", encodedEnter);
    cfg.writeEntry("EncodedEnterLen", encodedEnterLen);
}

void KRQuery::connectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&KRQuery::processEvents))
        processEventsConnected++;
}

void KRQuery::disconnectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&KRQuery::processEvents))
        processEventsConnected--;
}

bool KRQuery::checkPerm(QString filePerm) const
{
    for (int i = 0; i < 9; ++i)
        if (perm[i] != '?' && perm[i] != filePerm[i + 1])
            return false;
    return true;
}

bool KRQuery::checkType(QString mime) const
{
    if (type == mime)
        return true;
    if (type == i18n("Archives"))
        return KRarcHandler::arcSupported(mime);
    if (type == i18n("Folders"))
        return mime.contains("directory");
    if (type == i18n("Image Files"))
        return mime.contains("image/");
    if (type == i18n("Text Files"))
        return mime.contains("text/");
    if (type == i18n("Video Files"))
        return mime.contains("video/");
    if (type == i18n("Audio Files"))
        return mime.contains("audio/");
    if (type == i18n("Custom"))
        return customType.contains(mime);
    return false;
}

bool KRQuery::match(const QString &name) const { return matchCommon(name, matches, excludes); }

bool KRQuery::matchDirName(const QString &name) const
{
    return matchCommon(name, includedDirs, excludedDirs);
}

bool KRQuery::matchCommon(const QString &nameIn, const QStringList &matchList,
                          const QStringList &excludeList) const
{
    if (excludeList.count() == 0 && matchList.count() == 0) /* true if there's no match condition */
        return true;

    QString name(nameIn);
    int ndx = nameIn.lastIndexOf('/'); // virtual filenames may contain '/'
    if (ndx != -1)                     // but the end of the filename is OK
        name = nameIn.mid(ndx + 1);

    for (int i = 0; i < excludeList.count(); ++i) {
        if (QRegExp(excludeList[i], matchesCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
                    QRegExp::Wildcard)
                .exactMatch(name))
            return false;
    }

    if (matchList.count() == 0)
        return true;

    for (int i = 0; i < matchList.count(); ++i) {
        if (QRegExp(matchList[i], matchesCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
                    QRegExp::Wildcard)
                .exactMatch(name))
            return true;
    }
    return false;
}

bool KRQuery::match(FileItem *item) const
{
    if (item->isDir() && !matchDirName(item->getName()))
        return false;
    // see if the name matches
    if (!match(item->getName()))
        return false;
    // checking the mime
    if (!type.isEmpty() && !checkType(item->getMime()))
        return false;
    // check that the size fit
    KIO::filesize_t size = item->getSize();
    if (minSize && size < minSize)
        return false;
    if (maxSize && size > maxSize)
        return false;
    // check the time frame
    time_t mtime = item->getTime_t();
    if (olderThen && mtime > olderThen)
        return false;
    if (newerThen && mtime < newerThen)
        return false;
    // check owner name
    if (!owner.isEmpty() && item->getOwner() != owner)
        return false;
    // check group name
    if (!group.isEmpty() && item->getGroup() != group)
        return false;
    // check permission
    if (!perm.isEmpty() && !checkPerm(item->getPerm()))
        return false;

    if (!contain.isEmpty()) {
        if ((totalBytes = item->getSize()) == 0)
            totalBytes++; // sanity
        receivedBytes = 0;
        if (receivedBuffer)
            delete receivedBuffer;
        receivedBuffer = 0;
        receivedBufferLen = 0;
        fileName = item->getName();
        timer.start();

        // search locally
        if (item->getUrl().isLocalFile()) {
            return containsContent(item->getUrl().path());
        }

        // search remotely
        if (processEventsConnected == 0) {
            return false;
        }
        return containsContent(item->getUrl());
    }

    return true;
}

// takes the string and adds BOLD to it, so that when it is displayed,
// the grepped text will be bold
void fixFoundTextForDisplay(QString &haystack, int start, int length)
{
    QString before = haystack.left(start);
    QString text = haystack.mid(start, length);
    QString after = haystack.mid(start + length);

    before.replace('&', "&amp;");
    before.replace('<', "&lt;");
    before.replace('>', "&gt;");

    text.replace('&', "&amp;");
    text.replace('<', "&lt;");
    text.replace('>', "&gt;");

    after.replace('&', "&amp;");
    after.replace('<', "&lt;");
    after.replace('>', "&gt;");

    haystack = ("<qt>" + before + "<b>" + text + "</b>" + after + "</qt>");
}

bool KRQuery::checkBuffer(const char *data, int len) const
{
    bool result = false;

    char *mergedBuffer = new char[len + receivedBufferLen];
    if (receivedBufferLen)
        memcpy(mergedBuffer, receivedBuffer, receivedBufferLen);
    if (len)
        memcpy(mergedBuffer + receivedBufferLen, data, len);

    int maxLen = len + receivedBufferLen;
    int maxBuffer = maxLen - encodedEnterLen;
    int lastLinePosition = 0;

    for (int enterIndex = 0; enterIndex < maxBuffer; enterIndex++) {
        if (memcmp(mergedBuffer + enterIndex, encodedEnter, encodedEnterLen) == 0) {
            QString str = codec->toUnicode(mergedBuffer + lastLinePosition,
                                           enterIndex + encodedEnterLen - lastLinePosition);
            if (str.endsWith('\n')) {
                str.chop(1);
                result = result || checkLine(str);
                lastLinePosition = enterIndex + encodedEnterLen;
                enterIndex = lastLinePosition;
                continue;
            }
        }
    }

    if (maxLen - lastLinePosition > MAX_LINE_LEN || len == 0) {
        QString str = codec->toUnicode(mergedBuffer + lastLinePosition, maxLen - lastLinePosition);
        result = result || checkLine(str);
        lastLinePosition = maxLen;
    }

    delete[] receivedBuffer;
    receivedBuffer = 0;
    receivedBufferLen = maxLen - lastLinePosition;

    if (receivedBufferLen) {
        receivedBuffer = new char[receivedBufferLen];
        memcpy(receivedBuffer, mergedBuffer + lastLinePosition, receivedBufferLen);
    }

    delete[] mergedBuffer;
    return result;
}

bool KRQuery::checkLine(const QString &line, bool backwards) const
{
    if (containRegExp) {
        QRegExp rexp(contain, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive,
                     QRegExp::RegExp);
        int ndx = backwards ? rexp.lastIndexIn(line) : rexp.indexIn(line);
        bool result = ndx >= 0;
        if (result)
            fixFoundTextForDisplay(lastSuccessfulGrep = line, lastSuccessfulGrepMatchIndex = ndx,
                                   lastSuccessfulGrepMatchLength = rexp.matchedLength());
        return result;
    }

    int ndx = backwards ? -1 : 0;

    if (line.isNull())
        return false;
    if (containWholeWord) {
        while ((ndx = (backwards) ?
                          line.lastIndexOf(contain, ndx,
                                           containCaseSensetive ? Qt::CaseSensitive :
                                                                  Qt::CaseInsensitive) :
                          line.indexOf(contain, ndx,
                                       containCaseSensetive ? Qt::CaseSensitive :
                                                              Qt::CaseInsensitive)) != -1) {
            QChar before = '\n';
            QChar after = '\n';

            if (ndx > 0)
                before = line.at(ndx - 1);
            if (ndx + contain.length() < line.length())
                after = line.at(ndx + contain.length());

            if (!before.isLetterOrNumber() && !after.isLetterOrNumber() && after != '_' &&
                before != '_') {
                lastSuccessfulGrep = line;
                fixFoundTextForDisplay(lastSuccessfulGrep, lastSuccessfulGrepMatchIndex = ndx,
                                       lastSuccessfulGrepMatchLength = contain.length());
                return true;
            }

            if (backwards)
                ndx -= line.length() + 1;
            else
                ndx++;
        }
    } else if ((ndx = (backwards) ?
                          line.lastIndexOf(contain, -1,
                                           containCaseSensetive ? Qt::CaseSensitive :
                                                                  Qt::CaseInsensitive) :
                          line.indexOf(contain, 0,
                                       containCaseSensetive ? Qt::CaseSensitive :
                                                              Qt::CaseInsensitive)) != -1) {
        lastSuccessfulGrep = line;
        fixFoundTextForDisplay(lastSuccessfulGrep, lastSuccessfulGrepMatchIndex = ndx,
                               lastSuccessfulGrepMatchLength = contain.length());
        return true;
    }
    return false;
}

bool KRQuery::containsContent(QString file) const
{
    QFile qf(file);
    if (!qf.open(QIODevice::ReadOnly))
        return false;

    char buffer[1440]; // 2k buffer

    while (!qf.atEnd()) {
        int bytes = qf.read(buffer, sizeof(buffer));
        if (bytes <= 0)
            break;

        receivedBytes += bytes;

        if (checkBuffer(buffer, bytes))
            return true;

        if (checkTimer()) {
            bool stopped = false;
            emit((KRQuery *)this)->processEvents(stopped);
            if (stopped)
                return false;
        }
    }
    if (checkBuffer(buffer, 0))
        return true;

    lastSuccessfulGrep.clear(); // nothing was found
    return false;
}

bool KRQuery::containsContent(QUrl url) const
{
    KIO::TransferJob *contentReader = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(contentReader, &KIO::TransferJob::data, this, &KRQuery::containsContentData);
    connect(contentReader, &KIO::Job::result, this, &KRQuery::containsContentFinished);

    busy = true;
    containsContentResult = false;
    bool stopped = false;

    while (busy && !stopped) {
        checkTimer();
        emit((KRQuery *)this)->processEvents(stopped);
    }

    if (busy) {
        contentReader->kill(KJob::EmitResult);
        busy = false;
    }

    return containsContentResult;
}

void KRQuery::containsContentData(KIO::Job *job, const QByteArray &array)
{
    receivedBytes += array.size();
    if (checkBuffer(array.data(), array.size())) {
        containsContentResult = true;
        containsContentFinished(job);
        job->kill(KJob::EmitResult);
        return;
    }
    checkTimer();
}

void KRQuery::containsContentFinished(KJob *) { busy = false; }

bool KRQuery::checkTimer() const
{
    if (timer.elapsed() >= STATUS_SEND_DELAY) {
        int pcnt = (int)(100. * (double)receivedBytes / (double)totalBytes + .5);
        QString message =
            i18nc("%1=filename, %2=percentage", "Searching content of '%1' (%2%)", fileName, pcnt);
        timer.start();
        emit((KRQuery *)this)->status(message);
        return true;
    }
    return false;
}

QStringList KRQuery::split(QString str)
{
    QStringList list;
    int splitNdx = 0;
    int startNdx = 0;
    bool quotation = false;

    while (splitNdx < str.length()) {
        if (str[splitNdx] == '"')
            quotation = !quotation;

        if (!quotation && str[splitNdx] == ' ') {
            QString section = str.mid(startNdx, splitNdx - startNdx);
            startNdx = splitNdx + 1;
            if (section.startsWith('\"') && section.endsWith('\"') && section.length() >= 2)
                section = section.mid(1, section.length() - 2);
            if (!section.isEmpty())
                list.append(section);
        }
        splitNdx++;
    }

    if (startNdx < splitNdx) {
        QString section = str.mid(startNdx, splitNdx - startNdx);
        if (section.startsWith('\"') && section.endsWith('\"') && section.length() >= 2)
            section = section.mid(1, section.length() - 2);
        if (!section.isEmpty())
            list.append(section);
    }

    return list;
}

void KRQuery::setNameFilter(const QString &text, bool cs)
{
    bNull = false;
    matchesCaseSensitive = cs;
    origFilter = text;

    QString matchText = text;
    QString excludeText;

    int excludeNdx = 0;
    bool quotationMark = 0;
    while (excludeNdx < matchText.length()) {
        if (matchText[excludeNdx] == '"')
            quotationMark = !quotationMark;
        if (!quotationMark) {
            if (matchText[excludeNdx] == '|')
                break;
        }
        excludeNdx++;
    }

    if (excludeNdx < matchText.length()) {
        excludeText = matchText.mid(excludeNdx + 1).trimmed();
        matchText.truncate(excludeNdx);
        matchText = matchText.trimmed();
        if (matchText.isEmpty())
            matchText = '*';
    }

    int i;

    matches = split(matchText);
    includedDirs.clear();

    for (i = 0; i < matches.count();) {
        if (matches[i].endsWith('/')) {
            includedDirs.push_back(matches[i].left(matches[i].length() - 1));
            matches.removeAll(matches.at(i));
            continue;
        }

        if (!matches[i].contains("*") && !matches[i].contains("?"))
            matches[i] = '*' + matches[i] + '*';

        i++;
    }

    excludes = split(excludeText);
    excludedDirs.clear();

    for (i = 0; i < excludes.count();) {
        if (excludes[i].endsWith('/')) {
            excludedDirs.push_back(excludes[i].left(excludes[i].length() - 1));
            excludes.removeAll(excludes.at(i));
            continue;
        }

        if (!excludes[i].contains("*") && !excludes[i].contains("?"))
            excludes[i] = '*' + excludes[i] + '*';

        i++;
    }
}

void KRQuery::setContent(const QString &content, bool cs, bool wholeWord, QString encoding,
                         bool regExp)
{
    bNull = false;
    contain = content;
    containCaseSensetive = cs;
    containWholeWord = wholeWord;
    containRegExp = regExp;

    if (encoding.isEmpty())
        codec = QTextCodec::codecForLocale();
    else {
        codec = QTextCodec::codecForName(encoding.toLatin1());
        if (codec == 0)
            codec = QTextCodec::codecForLocale();
    }

    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();
}

void KRQuery::setMinimumFileSize(KIO::filesize_t minimumSize)
{
    bNull = false;
    minSize = minimumSize;
}

void KRQuery::setMaximumFileSize(KIO::filesize_t maximumSize)
{
    bNull = false;
    maxSize = maximumSize;
}

void KRQuery::setNewerThan(time_t time)
{
    bNull = false;
    newerThen = time;
}

void KRQuery::setOlderThan(time_t time)
{
    bNull = false;
    olderThen = time;
}

void KRQuery::setOwner(const QString &ownerIn)
{
    bNull = false;
    owner = ownerIn;
}

void KRQuery::setGroup(const QString &groupIn)
{
    bNull = false;
    group = groupIn;
}

void KRQuery::setPermissions(const QString &permIn)
{
    bNull = false;
    perm = permIn;
}

void KRQuery::setMimeType(const QString &typeIn, QStringList customList)
{
    bNull = false;
    type = typeIn;
    customType = customList;
}

bool KRQuery::isExcluded(const QUrl &url)
{
    for (int i = 0; i < whereNotToSearch.count(); ++i)
        if (whereNotToSearch[i].isParentOf(url) ||
            url.matches(whereNotToSearch[i], QUrl::StripTrailingSlash))
            return true;

    if (!matchDirName(url.fileName()))
        return true;

    return false;
}

void KRQuery::setSearchInDirs(const QList<QUrl> &urls)
{
    whereToSearch.clear();
    for (int i = 0; i < urls.count(); ++i) {
        QString url = urls[i].url();
        QUrl completed = QUrl::fromUserInput(KUrlCompletion::replacedPath(url, true, true),
                                             QString(), QUrl::AssumeLocalFile);
        whereToSearch.append(completed);
    }
}

void KRQuery::setDontSearchInDirs(const QList<QUrl> &urls)
{
    whereNotToSearch.clear();
    for (int i = 0; i < urls.count(); ++i) {
        QString url = urls[i].url();
        QUrl completed = QUrl::fromUserInput(KUrlCompletion::replacedPath(url, true, true),
                                             QString(), QUrl::AssumeLocalFile);
        whereNotToSearch.append(completed);
    }
}
