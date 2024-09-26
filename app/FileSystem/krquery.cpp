/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krquery.h"

// QtCore
#include <QFile>
#include <QMetaMethod>
#include <QRegExp>
#include <QTextCodec>

#include <KFileItem>
#include <KFormat>
#include <KIO/Job>
#include <KIO/TransferJob>
#include <KLocalizedString>
#include <KUrlCompletion>
#include <utility>

#include "../Archive/krarchandler.h"
#include "fileitem.h"
#include "filesystem.h"
#include "krpermhandler.h"

#define STATUS_SEND_DELAY 250
#define MAX_LINE_LEN 1000

// set the defaults
KrQuery::KrQuery()
    : matchesCaseSensitive(true)
    , bNull(true)
    , contain(QString())
    , containCaseSensetive(true)
    , containWholeWord(false)
    , containRegExp(false)
    , minSize(0)
    , maxSize(0)
    , newerThen(0)
    , olderThen(0)
    , owner(QString())
    , group(QString())
    , perm(QString())
    , type(QString())
    , inArchive(false)
    , recurse(true)
    , followLinksP(true)
    , receivedBuffer(nullptr)
    , receivedBufferLen(0)
    , processEventsConnected(0)
    , codec(QTextCodec::codecForLocale())
{
    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();
}

// set the defaults
KrQuery::KrQuery(const QString &name, bool matchCase)
    : bNull(true)
    , contain(QString())
    , containCaseSensetive(true)
    , containWholeWord(false)
    , containRegExp(false)
    , minSize(0)
    , maxSize(0)
    , newerThen(0)
    , olderThen(0)
    , owner(QString())
    , group(QString())
    , perm(QString())
    , type(QString())
    , inArchive(false)
    , recurse(true)
    , followLinksP(true)
    , receivedBuffer(nullptr)
    , receivedBufferLen(0)
    , processEventsConnected(0)
    , codec(QTextCodec::codecForLocale())
{
    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();

    setNameFilter(name, matchCase);
}

KrQuery::KrQuery(const KrQuery &that)
    : QObject()
    , receivedBuffer(nullptr)
    , receivedBufferLen(0)
    , processEventsConnected(0)
{
    *this = that;
}

KrQuery::~KrQuery()
{
    if (receivedBuffer)
        delete[] receivedBuffer;
    receivedBuffer = nullptr;
}

KrQuery &KrQuery::operator=(const KrQuery &old)
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
    excludedFolderNames = old.excludedFolderNames;
    whereNotToSearch = old.whereNotToSearch;
    origFilter = old.origFilter;

    codec = old.codec;

    encodedEnterArray = old.encodedEnterArray;
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();

    return *this;
}

void KrQuery::load(const KConfigGroup &cfg)
{
    *this = KrQuery(); // reset parameters first

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
    newerThen = QDateTime::fromString(cfg.readEntry("NewerThan", QDateTime::fromSecsSinceEpoch(static_cast<uint>(newerThen)).toString())).toSecsSinceEpoch();
    olderThen = QDateTime::fromString(cfg.readEntry("OlderThan", QDateTime::fromSecsSinceEpoch(static_cast<uint>(olderThen)).toString())).toSecsSinceEpoch();
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

void KrQuery::save(KConfigGroup cfg)
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
    cfg.writeEntry("NewerThan", QDateTime::fromSecsSinceEpoch(static_cast<uint>(newerThen)).toString());
    cfg.writeEntry("OlderThan", QDateTime::fromSecsSinceEpoch(static_cast<uint>(olderThen)).toString());
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

void KrQuery::connectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&KrQuery::processEvents))
        processEventsConnected++;
}

void KrQuery::disconnectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&KrQuery::processEvents))
        processEventsConnected--;
}

bool KrQuery::checkPerm(QString filePerm) const
{
    for (int i = 0; i < 9; ++i)
        if (perm[i] != '?' && perm[i] != filePerm[i + 1])
            return false;
    return true;
}

bool KrQuery::checkType(const QString &mime) const
{
    if (type == mime)
        return true;
    if (type == i18n("Archives"))
        return KrArcHandler::arcSupported(mime);
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

bool KrQuery::match(const QString &name) const
{
    return matchCommon(name, matches, excludes);
}

bool KrQuery::matchDirName(const QString &name) const
{
    return matchCommon(name, includedDirs, excludedDirs);
}

bool KrQuery::matchCommon(const QString &nameIn, const QStringList &matchList, const QStringList &excludeList) const
{
    if (excludeList.count() == 0 && matchList.count() == 0) /* true if there's no match condition */
        return true;

    QString name(nameIn);
    qsizetype ndx = nameIn.lastIndexOf('/'); // virtual filenames may contain '/'
    if (ndx != -1) // but the end of the filename is OK
        name = nameIn.mid(ndx + 1);

    for (int i = 0; i < excludeList.count(); ++i) {
        if (QRegExp(excludeList[i], matchesCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(name))
            return false;
    }

    if (matchList.count() == 0)
        return true;

    for (int i = 0; i < matchList.count(); ++i) {
        if (QRegExp(matchList[i], matchesCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(name))
            return true;
    }
    return false;
}

bool KrQuery::match(FileItem *item) const
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
    time_t mtime = item->getModificationTime();
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
        totalBytes = item->getSize();
        receivedBytes = 0;
        if (receivedBuffer)
            delete receivedBuffer;
        receivedBuffer = nullptr;
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
void fixFoundTextForDisplay(QString &haystack, qsizetype start, qsizetype length)
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

bool KrQuery::checkBuffer(const char *data, qsizetype len) const
{
    bool result = false;

    auto *mergedBuffer = new char[len + receivedBufferLen];
    if (receivedBufferLen)
        memcpy(mergedBuffer, receivedBuffer, receivedBufferLen);
    if (len)
        memcpy(mergedBuffer + receivedBufferLen, data, len);

    qsizetype maxLen = len + receivedBufferLen;
    qsizetype maxBuffer = maxLen - encodedEnterLen;
    qsizetype lastLinePosition = 0;

    for (qsizetype enterIndex = 0; enterIndex < maxBuffer; enterIndex++) {
        if (memcmp(mergedBuffer + enterIndex, encodedEnter, encodedEnterLen) == 0) {
            QString str = codec->toUnicode(mergedBuffer + lastLinePosition, static_cast<int>(enterIndex + encodedEnterLen - lastLinePosition));
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
        QString str = codec->toUnicode(mergedBuffer + lastLinePosition, static_cast<int>(maxLen - lastLinePosition));
        result = result || checkLine(str);
        lastLinePosition = maxLen;
    }

    delete[] receivedBuffer;
    receivedBuffer = nullptr;
    receivedBufferLen = maxLen - lastLinePosition;

    if (receivedBufferLen) {
        receivedBuffer = new char[receivedBufferLen];
        memcpy(receivedBuffer, mergedBuffer + lastLinePosition, receivedBufferLen);
    }

    delete[] mergedBuffer;
    return result;
}

bool KrQuery::checkLine(const QString &line, bool backwards) const
{
    if (containRegExp) {
        QRegExp rexp(contain, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp);
        int ndx = backwards ? rexp.lastIndexIn(line) : rexp.indexIn(line);
        bool result = ndx >= 0;
        if (result)
            fixFoundTextForDisplay(lastSuccessfulGrep = line, lastSuccessfulGrepMatchIndex = ndx, lastSuccessfulGrepMatchLength = rexp.matchedLength());
        return result;
    }

    qsizetype ndx = backwards ? -1 : 0;

    if (line.isNull())
        return false;
    if (containWholeWord) {
        while ((ndx = (backwards) ? line.lastIndexOf(contain, ndx, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive)
                                  : line.indexOf(contain, ndx, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive))
               != -1) {
            QChar before = '\n';
            QChar after = '\n';

            if (ndx > 0)
                before = line.at(ndx - 1);
            if (ndx + contain.length() < line.length())
                after = line.at(ndx + contain.length());

            if (!before.isLetterOrNumber() && !after.isLetterOrNumber() && after != '_' && before != '_') {
                lastSuccessfulGrep = line;
                fixFoundTextForDisplay(lastSuccessfulGrep, lastSuccessfulGrepMatchIndex = ndx, lastSuccessfulGrepMatchLength = contain.length());
                return true;
            }

            if (backwards)
                ndx -= line.length() + 1;
            else
                ndx++;
        }
    } else if ((ndx = (backwards) ? line.lastIndexOf(contain, -1, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive)
                                  : line.indexOf(contain, 0, containCaseSensetive ? Qt::CaseSensitive : Qt::CaseInsensitive))
               != -1) {
        lastSuccessfulGrep = line;
        fixFoundTextForDisplay(lastSuccessfulGrep, lastSuccessfulGrepMatchIndex = ndx, lastSuccessfulGrepMatchLength = contain.length());
        return true;
    }
    return false;
}

bool KrQuery::containsContent(const QString &file) const
{
    QFile qf(file);
    if (!qf.open(QIODevice::ReadOnly))
        return false;

    char buffer[1440]; // 2k buffer

    while (!qf.atEnd()) {
        // Note: As it's stated in the documentation, "`qint64 QIODevice::read(char *data,
        // qint64 maxSize)` Reads at most `maxSize` bytes"
        int bytes = static_cast<int>(qf.read(buffer, sizeof(buffer)));

        if (bytes <= 0)
            break;

        receivedBytes += bytes;

        if (checkBuffer(buffer, bytes))
            return true;

        if (checkTimer()) {
            bool stopped = false;
            emit(const_cast<KrQuery *>(this))->processEvents(stopped);
            if (stopped)
                return false;
        }
    }
    if (checkBuffer(buffer, 0))
        return true;

    lastSuccessfulGrep.clear(); // nothing was found
    return false;
}

bool KrQuery::containsContent(const QUrl &url) const
{
    KIO::TransferJob *contentReader = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(contentReader, &KIO::TransferJob::data, this, &KrQuery::containsContentData);
    connect(contentReader, &KIO::Job::result, this, &KrQuery::containsContentFinished);

    busy = true;
    containsContentResult = false;
    bool stopped = false;

    while (busy && !stopped) {
        checkTimer();
        emit(const_cast<KrQuery *>(this))->processEvents(stopped);
    }

    if (busy) {
        contentReader->kill(KJob::EmitResult);
        busy = false;
    }

    return containsContentResult;
}

void KrQuery::containsContentData(KIO::Job *job, const QByteArray &array)
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

void KrQuery::containsContentFinished(KJob *)
{
    busy = false;
}

bool KrQuery::checkTimer() const
{
    if (timer.elapsed() >= STATUS_SEND_DELAY) {
        static KFormat format;
        // clang-format off
        QString message =
            i18nc("%1=filename, %2=percentage", "Searching content of '%1' (%2)", fileName,
                      (totalBytes > 0 && totalBytes >= receivedBytes) ?
                        // The normal case: A percentage is shown.
                        // Note: Instead of rounding, a truncation is performed because each percentage is seen
                        // only briefly —therefore, here speed is more important than precision
                        i18nc("%1=percentage", "%1%", (receivedBytes*100)/totalBytes) :
                        // An unusual case: There are problems when calculating that percentage. Sometimes it's
                        // because the contents of a big symlinked file are read, then the variable `totalBytes`
                        // is very small (it's the size of a symlink) but much more bytes are read
                        format.formatByteSize(static_cast<double>(receivedBytes), 0, KFormat::IECBinaryDialect, KFormat::UnitMegaByte)
                );
        // clang-format on

        timer.start();
        emit(const_cast<KrQuery *>(this))->status(message);
        return true;
    }
    return false;
}

QStringList KrQuery::split(QString str)
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

void KrQuery::setNameFilter(const QString &text, bool cs)
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

void KrQuery::setContent(const QString &content, bool cs, bool wholeWord, const QString &encoding, bool regExp)
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
        if (codec == nullptr)
            codec = QTextCodec::codecForLocale();
    }

    QChar ch = '\n';
    QTextCodec::ConverterState state(QTextCodec::IgnoreHeader);
    encodedEnterArray = codec->fromUnicode(&ch, 1, &state);
    encodedEnter = encodedEnterArray.data();
    encodedEnterLen = encodedEnterArray.size();
}

void KrQuery::setMinimumFileSize(KIO::filesize_t minimumSize)
{
    bNull = false;
    minSize = minimumSize;
}

void KrQuery::setMaximumFileSize(KIO::filesize_t maximumSize)
{
    bNull = false;
    maxSize = maximumSize;
}

void KrQuery::setNewerThan(time_t time)
{
    bNull = false;
    newerThen = time;
}

void KrQuery::setOlderThan(time_t time)
{
    bNull = false;
    olderThen = time;
}

void KrQuery::setOwner(const QString &ownerIn)
{
    bNull = false;
    owner = ownerIn;
}

void KrQuery::setGroup(const QString &groupIn)
{
    bNull = false;
    group = groupIn;
}

void KrQuery::setPermissions(const QString &permIn)
{
    bNull = false;
    perm = permIn;
}

void KrQuery::setMimeType(const QString &typeIn, QStringList customList)
{
    bNull = false;
    type = typeIn;
    customType = std::move(customList);
}

bool KrQuery::isExcluded(const QUrl &url)
{
    for (QUrl &item : whereNotToSearch)
        if (item.isParentOf(url) || url.matches(item, QUrl::StripTrailingSlash))
            return true;

    // Exclude folder names that are configured in settings
    QString filename = url.fileName();
    for (QString &item : excludedFolderNames)
        if (filename == item)
            return true;

    if (!matchDirName(filename))
        return true;

    return false;
}

void KrQuery::setSearchInDirs(const QList<QUrl> &urls)
{
    whereToSearch.clear();
    for (int i = 0; i < urls.count(); ++i) {
        QString url = urls[i].url();
        QUrl completed = QUrl::fromUserInput(KUrlCompletion::replacedPath(url, true, true), QString(), QUrl::AssumeLocalFile);
        whereToSearch.append(completed);
    }
}

void KrQuery::setDontSearchInDirs(const QList<QUrl> &urls)
{
    whereNotToSearch.clear();
    for (int i = 0; i < urls.count(); ++i) {
        QString url = urls[i].url();
        QUrl completed = QUrl::fromUserInput(KUrlCompletion::replacedPath(url, true, true), QString(), QUrl::AssumeLocalFile);
        whereNotToSearch.append(completed);
    }
}

void KrQuery::setExcludeFolderNames(const QStringList &paths)
{
    excludedFolderNames.clear();
    excludedFolderNames.append(paths);
}
