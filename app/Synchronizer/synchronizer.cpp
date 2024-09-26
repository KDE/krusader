/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "synchronizer.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/krquery.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "synchronizerdirlist.h"

#include <utime.h>

// QtCore
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QRegExp>
#include <QTime>
#include <QTimer>
#include <QUrl>
// QtWidgets
#include <QApplication>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

#include <KIO/DeleteJob>
#include <KIO/JobUiDelegate>
#include <KIO/SkipDialog>
#include <KIO/MkdirJob>
#include <KIO/FileCopyJob>
#include <KIO/WidgetsAskUserActionHandler>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KUrlCompletion>

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
#include <acl/libacl.h>
#endif
#endif

#define DISPLAY_UPDATE_PERIOD 2

Synchronizer::Synchronizer()
    : displayUpdateCount(0)
    , markEquals(true)
    , markDiffers(true)
    , markCopyToLeft(true)
    , markCopyToRight(true)
    , markDeletable(true)
    , parentWidget(nullptr)
    , resultListIt(resultList)
{
}

Synchronizer::~Synchronizer()
{
    clearLists();
}

QUrl Synchronizer::fsUrl(const QString &strUrl)
{
    QUrl result = QUrl::fromUserInput(strUrl, QString(), QUrl::AssumeLocalFile);
    return KrServices::escapeFileUrl(result);
}

void Synchronizer::clearLists()
{
    QListIterator<SynchronizerFileItem *> i1(resultList);
    while (i1.hasNext())
        delete i1.next();
    resultList.clear();

    QListIterator<SynchronizerTask *> i2(stack);
    while (i2.hasNext())
        delete i2.next();
    stack.clear();

    temporaryList.clear();
}

void Synchronizer::reset()
{
    displayUpdateCount = 0;
    markEquals = markDiffers = markCopyToLeft = markCopyToRight = markDeletable = true;
    stopped = false;
    recurseSubDirs = followSymLinks = ignoreDate = asymmetric = cmpByContent = ignoreCase = autoScroll = false;
    markEquals = markDiffers = markCopyToLeft = markCopyToRight = markDeletable = markDuplicates = markSingles = false;
    leftCopyEnabled = rightCopyEnabled = deleteEnabled = overWrite = autoSkip = paused = false;
    leftCopyNr = rightCopyNr = deleteNr = 0;
    leftCopySize = rightCopySize = deleteSize = 0;
    comparedDirs = fileCount = 0;
    leftBaseDir.clear();
    rightBaseDir.clear();
    clearLists();
}

int Synchronizer::compare(QString leftURL,
                          QString rightURL,
                          KrQuery *query,
                          bool subDirs,
                          bool symLinks,
                          bool igDate,
                          bool asymm,
                          bool cmpByCnt,
                          bool igCase,
                          bool autoSc,
                          QStringList &selFiles,
                          int equThres,
                          int timeOffs,
                          int parThreads,
                          bool hiddenFiles)
{
    clearLists();

    recurseSubDirs = subDirs;
    followSymLinks = symLinks;
    ignoreDate = igDate;
    asymmetric = asymm;
    cmpByContent = cmpByCnt;
    autoScroll = autoSc;
    ignoreCase = igCase;
    selectedFiles = selFiles;
    equalsThreshold = equThres;
    timeOffset = timeOffs;
    parallelThreads = parThreads;
    ignoreHidden = hiddenFiles;

    stopped = false;

    this->query = query;

    leftURL = KUrlCompletion::replacedPath(leftURL, true, true);
    rightURL = KUrlCompletion::replacedPath(rightURL, true, true);

    if (!leftURL.endsWith('/'))
        leftURL += '/';
    if (!rightURL.endsWith('/'))
        rightURL += '/';

    excludedPaths = KrServices::toStringList(query->dontSearchInDirs());
    for (int i = 0; i != excludedPaths.count(); i++)
        if (excludedPaths[i].endsWith('/'))
            excludedPaths[i].truncate(excludedPaths[i].length() - 1);

    comparedDirs = fileCount = 0;

    stack.append(new CompareTask(nullptr, leftBaseDir = leftURL, rightBaseDir = rightURL, "", "", ignoreHidden));
    compareLoop();

    QListIterator<SynchronizerFileItem *> it(temporaryList);
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();

        if (item->isTemporary())
            delete item;
    }
    temporaryList.clear();

    if (!autoScroll)
        refresh(true);

    emit statusInfo(i18n("Number of files: %1", fileCount));
    return fileCount;
}

void Synchronizer::compareLoop()
{
    while (!stopped && !stack.isEmpty()) {
        for (int thread = 0; thread < (int)stack.count() && thread < parallelThreads; thread++) {
            SynchronizerTask *entry = stack.at(thread);

            if (entry->state() == ST_STATE_NEW)
                entry->start(parentWidget);

            if (entry->inherits("CompareTask")) {
                if (entry->state() == ST_STATE_READY) {
                    auto *ctentry = qobject_cast<CompareTask *>(entry);
                    if (ctentry->isDuplicate())
                        compareDirectory(ctentry->parent(), ctentry->leftDirList(), ctentry->rightDirList(), ctentry->leftDir(), ctentry->rightDir());
                    else
                        addSingleDirectory(ctentry->parent(), ctentry->dirList(), ctentry->dir(), ctentry->isLeft());
                }
                if (entry->state() == ST_STATE_READY || entry->state() == ST_STATE_ERROR)
                    comparedDirs++;
            }
            switch (entry->state()) {
            case ST_STATE_STATUS:
                emit statusInfo(entry->status());
                break;
            case ST_STATE_READY:
            case ST_STATE_ERROR:
                emit statusInfo(i18n("Number of compared folders: %1", comparedDirs));
                stack.removeAll(entry);
                delete entry;
                continue;
            default:
                break;
            }
        }
        if (!stack.isEmpty())
            qApp->processEvents();
    }

    QListIterator<SynchronizerTask *> it(stack);
    while (it.hasNext())
        delete it.next();
    stack.clear();
}

void Synchronizer::compareDirectory(SynchronizerFileItem *parent,
                                    SynchronizerDirList *left_directory,
                                    SynchronizerDirList *right_directory,
                                    const QString &leftDir,
                                    const QString &rightDir)
{
    const QString &leftURL = left_directory->url();
    const QString &rightURL = right_directory->url();
    FileItem *left_file;
    FileItem *right_file;

    QString file_name;
    bool checkIfSelected = false;

    if (leftDir.isEmpty() && rightDir.isEmpty() && selectedFiles.count())
        checkIfSelected = true;

    /* walking through in the left directory */
    for (left_file = left_directory->first(); left_file != nullptr && !stopped; left_file = left_directory->next()) {
        if (isDir(left_file))
            continue;

        file_name = left_file->getName();

        if (checkIfSelected && !selectedFiles.contains(file_name))
            continue;

        if (!query->match(left_file))
            continue;

        if ((right_file = right_directory->search(file_name, ignoreCase)) == nullptr)
            addLeftOnlyItem(parent,
                            file_name,
                            leftDir,
                            left_file->getSize(),
                            left_file->getModificationTime(),
                            readLink(left_file),
                            left_file->getOwner(),
                            left_file->getGroup(),
                            left_file->getMode(),
                            left_file->getACL());
        else {
            if (isDir(right_file))
                continue;

            addDuplicateItem(parent,
                             file_name,
                             right_file->getName(),
                             leftDir,
                             rightDir,
                             left_file->getSize(),
                             right_file->getSize(),
                             left_file->getModificationTime(),
                             right_file->getModificationTime(),
                             readLink(left_file),
                             readLink(right_file),
                             left_file->getOwner(),
                             right_file->getOwner(),
                             left_file->getGroup(),
                             right_file->getGroup(),
                             left_file->getMode(),
                             right_file->getMode(),
                             left_file->getACL(),
                             right_file->getACL());
        }
    }

    /* walking through in the right directory */
    for (right_file = right_directory->first(); right_file != nullptr && !stopped; right_file = right_directory->next()) {
        if (isDir(right_file))
            continue;

        file_name = right_file->getName();

        if (checkIfSelected && !selectedFiles.contains(file_name))
            continue;

        if (!query->match(right_file))
            continue;

        if (left_directory->search(file_name, ignoreCase) == nullptr)
            addRightOnlyItem(parent,
                             file_name,
                             rightDir,
                             right_file->getSize(),
                             right_file->getModificationTime(),
                             readLink(right_file),
                             right_file->getOwner(),
                             right_file->getGroup(),
                             right_file->getMode(),
                             right_file->getACL());
    }

    /* walking through the subdirectories */
    if (recurseSubDirs) {
        for (left_file = left_directory->first(); left_file != nullptr && !stopped; left_file = left_directory->next()) {
            if (left_file->isDir() && (followSymLinks || !left_file->isSymLink())) {
                QString left_file_name = left_file->getName();

                if (checkIfSelected && !selectedFiles.contains(left_file_name))
                    continue;

                // Exclude the left-side folder if it's inside the exclusion list
                if (query->isExcluded(left_file->getUrl()))
                    continue;

                if (!query->matchDirName(left_file_name))
                    continue;

                if ((right_file = right_directory->search(left_file_name, ignoreCase)) == nullptr) {
                    SynchronizerFileItem *me = addLeftOnlyItem(parent,
                                                               left_file_name,
                                                               leftDir,
                                                               0,
                                                               left_file->getModificationTime(),
                                                               readLink(left_file),
                                                               left_file->getOwner(),
                                                               left_file->getGroup(),
                                                               left_file->getMode(),
                                                               left_file->getACL(),
                                                               true,
                                                               !query->match(left_file));
                    stack.append(new CompareTask(me,
                                                 leftURL + left_file_name + '/',
                                                 leftDir.isEmpty() ? left_file_name : leftDir + '/' + left_file_name,
                                                 true,
                                                 ignoreHidden));
                } else {
                    QString right_file_name = right_file->getName();
                    SynchronizerFileItem *me = addDuplicateItem(parent,
                                                                left_file_name,
                                                                right_file_name,
                                                                leftDir,
                                                                rightDir,
                                                                0,
                                                                0,
                                                                left_file->getModificationTime(),
                                                                right_file->getModificationTime(),
                                                                readLink(left_file),
                                                                readLink(right_file),
                                                                left_file->getOwner(),
                                                                right_file->getOwner(),
                                                                left_file->getGroup(),
                                                                right_file->getGroup(),
                                                                left_file->getMode(),
                                                                right_file->getMode(),
                                                                left_file->getACL(),
                                                                right_file->getACL(),
                                                                true,
                                                                !query->match(left_file));
                    stack.append(new CompareTask(me,
                                                 leftURL + left_file_name + '/',
                                                 rightURL + right_file_name + '/',
                                                 leftDir.isEmpty() ? left_file_name : leftDir + '/' + left_file_name,
                                                 rightDir.isEmpty() ? right_file_name : rightDir + '/' + right_file_name,
                                                 ignoreHidden));
                }
            }
        }

        /* walking through the right side subdirectories */
        for (right_file = right_directory->first(); right_file != nullptr && !stopped; right_file = right_directory->next()) {
            if (right_file->isDir() && (followSymLinks || !right_file->isSymLink())) {
                file_name = right_file->getName();

                if (checkIfSelected && !selectedFiles.contains(file_name))
                    continue;

                // Exclude the right-side folder if it's inside the exclusion list
                if (query->isExcluded(right_file->getUrl()))
                    continue;

                if (!query->matchDirName(file_name))
                    continue;

                if (left_directory->search(file_name, ignoreCase) == nullptr) {
                    SynchronizerFileItem *me = addRightOnlyItem(parent,
                                                                file_name,
                                                                rightDir,
                                                                0,
                                                                right_file->getModificationTime(),
                                                                readLink(right_file),
                                                                right_file->getOwner(),
                                                                right_file->getGroup(),
                                                                right_file->getMode(),
                                                                right_file->getACL(),
                                                                true,
                                                                !query->match(right_file));
                    stack.append(
                        new CompareTask(me, rightURL + file_name + '/', rightDir.isEmpty() ? file_name : rightDir + '/' + file_name, false, ignoreHidden));
                }
            }
        }
    }
}

QString Synchronizer::getTaskTypeName(TaskType taskType)
{
    static QString names[] = {"=", "!=", "<-", "->", "DEL", "?", "?", "?", "?", "?"};

    return names[taskType];
}

SynchronizerFileItem *Synchronizer::addItem(SynchronizerFileItem *parent,
                                            const QString &leftFile,
                                            const QString &rightFile,
                                            const QString &leftDir,
                                            const QString &rightDir,
                                            bool existsLeft,
                                            bool existsRight,
                                            KIO::filesize_t leftSize,
                                            KIO::filesize_t rightSize,
                                            time_t leftDate,
                                            time_t rightDate,
                                            const QString &leftLink,
                                            const QString &rightLink,
                                            const QString &leftOwner,
                                            const QString &rightOwner,
                                            const QString &leftGroup,
                                            const QString &rightGroup,
                                            mode_t leftMode,
                                            mode_t rightMode,
                                            const QString &leftACL,
                                            const QString &rightACL,
                                            TaskType tsk,
                                            bool isDir,
                                            bool isTemp)
{
    bool marked = autoScroll ? !isTemp && isMarked(tsk, existsLeft && existsRight) : false;
    auto *item = new SynchronizerFileItem(leftFile,
                                          rightFile,
                                          leftDir,
                                          rightDir,
                                          marked,
                                          existsLeft,
                                          existsRight,
                                          leftSize,
                                          rightSize,
                                          leftDate,
                                          rightDate,
                                          leftLink,
                                          rightLink,
                                          leftOwner,
                                          rightOwner,
                                          leftGroup,
                                          rightGroup,
                                          leftMode,
                                          rightMode,
                                          leftACL,
                                          rightACL,
                                          tsk,
                                          isDir,
                                          isTemp,
                                          parent);

    if (!isTemp) {
        while (parent && parent->isTemporary())
            setPermanent(parent);

        bool doRefresh = false;

        if (marked) {
            fileCount++;
            if (autoScroll && markParentDirectories(item))
                doRefresh = true;
        }

        resultList.append(item);
        emit comparedFileData(item);

        if (doRefresh)
            refresh(true);

        if (marked && (displayUpdateCount++ % DISPLAY_UPDATE_PERIOD == (DISPLAY_UPDATE_PERIOD - 1)))
            qApp->processEvents();
    } else
        temporaryList.append(item);

    return item;
}

void Synchronizer::compareContentResult(SynchronizerFileItem *item, bool res)
{
    item->compareContentResult(res);
    bool marked = autoScroll ? isMarked(item->task(), item->existsInLeft() && item->existsInRight()) : false;
    item->setMarked(marked);
    if (marked) {
        markParentDirectories(item);
        fileCount++;
        emit markChanged(item, true);
    }
}

void Synchronizer::setPermanent(SynchronizerFileItem *item)
{
    if (item->parent() && item->parent()->isTemporary())
        setPermanent(item->parent());

    item->setPermanent();
    resultList.append(item);
    emit comparedFileData(item);
}

SynchronizerFileItem *Synchronizer::addLeftOnlyItem(SynchronizerFileItem *parent,
                                                    const QString &file_name,
                                                    const QString &dir,
                                                    KIO::filesize_t size,
                                                    time_t date,
                                                    const QString &link,
                                                    const QString &owner,
                                                    const QString &group,
                                                    mode_t mode,
                                                    const QString &acl,
                                                    bool isDir,
                                                    bool isTemp)
{
    return addItem(parent,
                   file_name,
                   file_name,
                   dir,
                   dir,
                   true,
                   false,
                   size,
                   0,
                   date,
                   0,
                   link,
                   QString(),
                   owner,
                   QString(),
                   group,
                   QString(),
                   mode,
                   (mode_t)-1,
                   acl,
                   QString(),
                   asymmetric ? TT_DELETE : TT_COPY_TO_RIGHT,
                   isDir,
                   isTemp);
}

SynchronizerFileItem *Synchronizer::addRightOnlyItem(SynchronizerFileItem *parent,
                                                     const QString &file_name,
                                                     const QString &dir,
                                                     KIO::filesize_t size,
                                                     time_t date,
                                                     const QString &link,
                                                     const QString &owner,
                                                     const QString &group,
                                                     mode_t mode,
                                                     const QString &acl,
                                                     bool isDir,
                                                     bool isTemp)
{
    return addItem(parent,
                   file_name,
                   file_name,
                   dir,
                   dir,
                   false,
                   true,
                   0,
                   size,
                   0,
                   date,
                   QString(),
                   link,
                   QString(),
                   owner,
                   QString(),
                   group,
                   (mode_t)-1,
                   mode,
                   QString(),
                   acl,
                   TT_COPY_TO_LEFT,
                   isDir,
                   isTemp);
}

SynchronizerFileItem *Synchronizer::addDuplicateItem(SynchronizerFileItem *parent,
                                                     const QString &leftName,
                                                     const QString &rightName,
                                                     const QString &leftDir,
                                                     const QString &rightDir,
                                                     KIO::filesize_t leftSize,
                                                     KIO::filesize_t rightSize,
                                                     time_t leftDate,
                                                     time_t rightDate,
                                                     const QString &leftLink,
                                                     const QString &rightLink,
                                                     const QString &leftOwner,
                                                     const QString &rightOwner,
                                                     const QString &leftGroup,
                                                     const QString &rightGroup,
                                                     mode_t leftMode,
                                                     mode_t rightMode,
                                                     const QString &leftACL,
                                                     const QString &rightACL,
                                                     bool isDir,
                                                     bool isTemp)
{
    TaskType task;

    time_t checkedRightDate = rightDate - timeOffset;
    int uncertain = 0;

    do {
        if (isDir) {
            task = TT_EQUALS;
            break;
        }
        if (leftSize == rightSize) {
            if (!leftLink.isNull() || !rightLink.isNull()) {
                if (leftLink == rightLink) {
                    task = TT_EQUALS;
                    break;
                }
            } else if (cmpByContent)
                uncertain = TT_UNKNOWN;
            else {
                if (ignoreDate || leftDate == checkedRightDate) {
                    task = TT_EQUALS;
                    break;
                }
                time_t diff = (leftDate > checkedRightDate) ? leftDate - checkedRightDate : checkedRightDate - leftDate;
                if (diff <= equalsThreshold) {
                    task = TT_EQUALS;
                    break;
                }
            }
        }

        if (asymmetric) {
            // To avoid that the default action is
            // to overwrite a file by an old version of it
            if (!ignoreDate && leftDate > checkedRightDate)
                task = TT_DIFFERS;
            else
                task = TT_COPY_TO_LEFT;
        } else if (ignoreDate)
            task = TT_DIFFERS;
        else if (leftDate > checkedRightDate)
            task = TT_COPY_TO_RIGHT;
        else if (leftDate < checkedRightDate)
            task = TT_COPY_TO_LEFT;
        else
            task = TT_DIFFERS;
    } while (false);

    SynchronizerFileItem *item = addItem(parent,
                                         leftName,
                                         rightName,
                                         leftDir,
                                         rightDir,
                                         true,
                                         true,
                                         leftSize,
                                         rightSize,
                                         leftDate,
                                         rightDate,
                                         leftLink,
                                         rightLink,
                                         leftOwner,
                                         rightOwner,
                                         leftGroup,
                                         rightGroup,
                                         leftMode,
                                         rightMode,
                                         leftACL,
                                         rightACL,
                                         (TaskType)(task + uncertain),
                                         isDir,
                                         isTemp);

    if (uncertain == TT_UNKNOWN) {
        QUrl leftURL = Synchronizer::fsUrl(leftDir.isEmpty() ? (QString) (leftBaseDir + leftName): leftBaseDir + leftDir + QString('/') + leftName);
        QUrl rightURL = Synchronizer::fsUrl(rightDir.isEmpty() ? (QString) (rightBaseDir + rightName) : rightBaseDir + rightDir + '/' + rightName);
        stack.append(new CompareContentTask(this, item, leftURL, rightURL, leftSize));
    }

    return item;
}

void Synchronizer::addSingleDirectory(SynchronizerFileItem *parent, SynchronizerDirList *directory, const QString &dirName, bool isLeft)
{
    const QString &url = directory->url();
    FileItem *file;
    QString file_name;

    /* walking through the directory files */
    for (file = directory->first(); file != nullptr && !stopped; file = directory->next()) {
        if (isDir(file))
            continue;

        file_name = file->getName();

        if (!query->match(file))
            continue;

        if (isLeft)
            addLeftOnlyItem(parent,
                            file_name,
                            dirName,
                            file->getSize(),
                            file->getModificationTime(),
                            readLink(file),
                            file->getOwner(),
                            file->getGroup(),
                            file->getMode(),
                            file->getACL());
        else
            addRightOnlyItem(parent,
                             file_name,
                             dirName,
                             file->getSize(),
                             file->getModificationTime(),
                             readLink(file),
                             file->getOwner(),
                             file->getGroup(),
                             file->getMode(),
                             file->getACL());
    }

    /* walking through the subdirectories */
    for (file = directory->first(); file != nullptr && !stopped; file = directory->next()) {
        if (file->isDir() && (followSymLinks || !file->isSymLink())) {
            file_name = file->getName();

            if (excludedPaths.contains(dirName.isEmpty() ? file_name : dirName + '/' + file_name))
                continue;

            if (!query->matchDirName(file_name))
                continue;

            SynchronizerFileItem *me;

            if (isLeft)
                me = addLeftOnlyItem(parent,
                                     file_name,
                                     dirName,
                                     0,
                                     file->getModificationTime(),
                                     readLink(file),
                                     file->getOwner(),
                                     file->getGroup(),
                                     file->getMode(),
                                     file->getACL(),
                                     true,
                                     !query->match(file));
            else
                me = addRightOnlyItem(parent,
                                      file_name,
                                      dirName,
                                      0,
                                      file->getModificationTime(),
                                      readLink(file),
                                      file->getOwner(),
                                      file->getGroup(),
                                      file->getMode(),
                                      file->getACL(),
                                      true,
                                      !query->match(file));
            stack.append(new CompareTask(me, url + file_name + '/', dirName.isEmpty() ? file_name : dirName + '/' + file_name, isLeft, ignoreHidden));
        }
    }
}

void Synchronizer::setMarkFlags(bool left, bool equal, bool differs, bool right, bool dup, bool sing, bool del)
{
    markEquals = equal;
    markDiffers = differs;
    markCopyToLeft = left;
    markCopyToRight = right;
    markDeletable = del;
    markDuplicates = dup;
    markSingles = sing;
}

bool Synchronizer::isMarked(TaskType task, bool isDuplicate)
{
    if ((isDuplicate && !markDuplicates) || (!isDuplicate && !markSingles))
        return false;

    switch (task) {
    case TT_EQUALS:
        return markEquals;
    case TT_DIFFERS:
        return markDiffers;
    case TT_COPY_TO_LEFT:
        return markCopyToLeft;
    case TT_COPY_TO_RIGHT:
        return markCopyToRight;
    case TT_DELETE:
        return markDeletable;
    default:
        return false;
    }
}

bool Synchronizer::markParentDirectories(SynchronizerFileItem *item)
{
    if (item->parent() == nullptr || item->parent()->isMarked())
        return false;

    markParentDirectories(item->parent());

    item->parent()->setMarked(true);

    fileCount++;
    emit markChanged(item->parent(), false);
    return true;
}

int Synchronizer::refresh(bool nostatus)
{
    fileCount = 0;

    QListIterator<SynchronizerFileItem *> it(resultList);
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();

        bool marked = isMarked(item->task(), item->existsInLeft() && item->existsInRight());
        item->setMarked(marked);

        if (marked) {
            markParentDirectories(item);
            fileCount++;
        }
    }

    it.toFront();
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();
        emit markChanged(item, false);
    }

    if (!nostatus)
        emit statusInfo(i18n("Number of files: %1", fileCount));

    return fileCount;
}

void Synchronizer::operate(SynchronizerFileItem *item, void (*executeOperation)(SynchronizerFileItem *))
{
    executeOperation(item);

    if (item->isDir()) {
        QString leftDirName = (item->leftDirectory().isEmpty()) ? item->leftName() : item->leftDirectory() + '/' + item->leftName();
        QString rightDirName = (item->rightDirectory().isEmpty()) ? item->rightName() : item->rightDirectory() + '/' + item->rightName();

        QListIterator<SynchronizerFileItem *> it(resultList);
        while (it.hasNext()) {
            SynchronizerFileItem *item = it.next();

            if (item->leftDirectory() == leftDirName || item->leftDirectory().startsWith(leftDirName + '/') || item->rightDirectory() == rightDirName
                || item->rightDirectory().startsWith(rightDirName + '/'))
                executeOperation(item);
        }
    }
}

void Synchronizer::excludeOperation(SynchronizerFileItem *item)
{
    item->setTask(TT_DIFFERS);
}

void Synchronizer::exclude(SynchronizerFileItem *item)
{
    if (!item->parent() || item->parent()->task() != TT_DELETE)
        operate(item, excludeOperation); /* exclude only if the parent task is not DEL */
}

void Synchronizer::restoreOperation(SynchronizerFileItem *item)
{
    item->restoreOriginalTask();
}

void Synchronizer::restore(SynchronizerFileItem *item)
{
    operate(item, restoreOperation);

    while ((item = item->parent()) != nullptr) /* in case of restore, the parent directories */
    { /* must be changed for being consistent */
        if (item->task() != TT_DIFFERS)
            break;

        if (item->originalTask() == TT_DELETE) /* if the parent original task is delete */
            break; /* don't touch it */

        item->restoreOriginalTask(); /* restore */
    }
}

void Synchronizer::reverseDirectionOperation(SynchronizerFileItem *item)
{
    if (item->existsInRight() && item->existsInLeft()) {
        if (item->task() == TT_COPY_TO_LEFT)
            item->setTask(TT_COPY_TO_RIGHT);
        else if (item->task() == TT_COPY_TO_RIGHT)
            item->setTask(TT_COPY_TO_LEFT);
    }
}

void Synchronizer::reverseDirection(SynchronizerFileItem *item)
{
    operate(item, reverseDirectionOperation);
}

void Synchronizer::deleteLeftOperation(SynchronizerFileItem *item)
{
    if (!item->existsInRight() && item->existsInLeft())
        item->setTask(TT_DELETE);
}

void Synchronizer::deleteLeft(SynchronizerFileItem *item)
{
    operate(item, deleteLeftOperation);
}

void Synchronizer::copyToLeftOperation(SynchronizerFileItem *item)
{
    if (item->existsInRight()) {
        if (!item->isDir())
            item->setTask(TT_COPY_TO_LEFT);
        else {
            if (item->existsInLeft() && item->existsInRight())
                item->setTask(TT_EQUALS);
            else if (!item->existsInLeft() && item->existsInRight())
                item->setTask(TT_COPY_TO_LEFT);
        }
    }
}

void Synchronizer::copyToLeft(SynchronizerFileItem *item)
{
    operate(item, copyToLeftOperation);

    while ((item = item->parent()) != nullptr) {
        if (item->task() != TT_DIFFERS)
            break;

        if (item->existsInLeft() && item->existsInRight())
            item->setTask(TT_EQUALS);
        else if (!item->existsInLeft() && item->existsInRight())
            item->setTask(TT_COPY_TO_LEFT);
    }
}

void Synchronizer::copyToRightOperation(SynchronizerFileItem *item)
{
    if (item->existsInLeft()) {
        if (!item->isDir())
            item->setTask(TT_COPY_TO_RIGHT);
        else {
            if (item->existsInLeft() && item->existsInRight())
                item->setTask(TT_EQUALS);
            else if (item->existsInLeft() && !item->existsInRight())
                item->setTask(TT_COPY_TO_RIGHT);
        }
    }
}

void Synchronizer::copyToRight(SynchronizerFileItem *item)
{
    operate(item, copyToRightOperation);

    while ((item = item->parent()) != nullptr) {
        if (item->task() != TT_DIFFERS && item->task() != TT_DELETE)
            break;

        if (item->existsInLeft() && item->existsInRight())
            item->setTask(TT_EQUALS);
        else if (item->existsInLeft() && !item->existsInRight())
            item->setTask(TT_COPY_TO_RIGHT);
    }
}

bool Synchronizer::totalSizes(int *leftCopyNr,
                              KIO::filesize_t *leftCopySize,
                              int *rightCopyNr,
                              KIO::filesize_t *rightCopySize,
                              int *deleteNr,
                              KIO::filesize_t *deletableSize)
{
    bool hasAnythingToDo = false;

    *leftCopySize = *rightCopySize = *deletableSize = 0;
    *leftCopyNr = *rightCopyNr = *deleteNr = 0;

    QListIterator<SynchronizerFileItem *> it(resultList);
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();

        if (item->isMarked()) {
            switch (item->task()) {
            case TT_COPY_TO_LEFT:
                *leftCopySize += item->rightSize();
                (*leftCopyNr)++;
                hasAnythingToDo = true;
                break;
            case TT_COPY_TO_RIGHT:
                *rightCopySize += item->leftSize();
                (*rightCopyNr)++;
                hasAnythingToDo = true;
                break;
            case TT_DELETE:
                *deletableSize += item->leftSize();
                (*deleteNr)++;
                hasAnythingToDo = true;
                break;
            default:
                break;
            }
        }
    }

    return hasAnythingToDo;
}

void Synchronizer::swapSides()
{
    QString leftTmp = leftBaseDir;
    leftBaseDir = rightBaseDir;
    rightBaseDir = leftTmp;

    QListIterator<SynchronizerFileItem *> it(resultList);
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();

        item->swap(asymmetric);
    }
}

void Synchronizer::setScrolling(bool scroll)
{
    autoScroll = scroll;
    if (autoScroll) {
        int oldFileCount = fileCount;
        refresh(true);
        fileCount = oldFileCount;
    }
}

void Synchronizer::synchronize(QWidget *syncWdg, bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled, bool overWrite, int parThreads)
{
    this->leftCopyEnabled = leftCopyEnabled;
    this->rightCopyEnabled = rightCopyEnabled;
    this->deleteEnabled = deleteEnabled;
    this->overWrite = overWrite;
    this->parallelThreads = parThreads;
    this->syncDlgWidget = syncWdg;

    autoSkip = paused = disableNewTasks = false;

    leftCopyNr = rightCopyNr = deleteNr = 0;
    leftCopySize = rightCopySize = deleteSize = 0;

    inTaskFinished = 0;
    lastTask = nullptr;

    jobMap.clear();
    receivedMap.clear();

    resultListIt = resultList;
    synchronizeLoop();
}

void Synchronizer::synchronizeLoop()
{
    if (disableNewTasks) {
        if (!resultListIt.hasNext() && jobMap.count() == 0)
            emit synchronizationFinished();
        return;
    }

    while ((int)jobMap.count() < parallelThreads) {
        SynchronizerFileItem *task = getNextTask();
        if (task == nullptr) {
            if (jobMap.count() == 0)
                emit synchronizationFinished();
            return;
        }
        executeTask(task);
        if (disableNewTasks)
            break;
    }
}

SynchronizerFileItem *Synchronizer::getNextTask()
{
    TaskType task;
    SynchronizerFileItem *currentTask;

    do {
        if (!resultListIt.hasNext())
            return nullptr;

        currentTask = resultListIt.next();

        if (currentTask->isMarked()) {
            task = currentTask->task();

            if (leftCopyEnabled && task == TT_COPY_TO_LEFT)
                break;
            else if (rightCopyEnabled && task == TT_COPY_TO_RIGHT)
                break;
            else if (deleteEnabled && task == TT_DELETE)
                break;
        }
    } while (true);

    return lastTask = currentTask;
}

void Synchronizer::executeTask(SynchronizerFileItem *task)
{
    QString leftDirName = task->leftDirectory();
    if (!leftDirName.isEmpty())
        leftDirName += '/';
    QString rightDirName = task->rightDirectory();
    if (!rightDirName.isEmpty())
        rightDirName += '/';

    QUrl leftURL = Synchronizer::fsUrl(leftBaseDir + leftDirName + task->leftName());
    QUrl rightURL = Synchronizer::fsUrl(rightBaseDir + rightDirName + task->rightName());

    switch (task->task()) {
    case TT_COPY_TO_LEFT:
        if (task->isDir()) {
            KIO::SimpleJob *job = KIO::mkdir(leftURL);
            connect(job, &KIO::MkdirJob::result, this, &Synchronizer::slotTaskFinished);
            jobMap[job] = task;
            disableNewTasks = true;
        } else {
            QUrl destURL(leftURL);
            if (!task->destination().isNull())
                destURL = Synchronizer::fsUrl(task->destination());

            if (task->rightLink().isNull()) {
                KIO::FileCopyJob *job =
                    KIO::file_copy(rightURL, destURL, -1, ((overWrite || task->overWrite()) ? KIO::Overwrite : KIO::DefaultFlags) | KIO::HideProgressInfo);
                connect(job, SIGNAL(processedSize(KJob *, qulonglong)), this, SLOT(slotProcessedSize(KJob *, qulonglong)));
                connect(job, &KIO::FileCopyJob::result, this, &Synchronizer::slotTaskFinished);
                jobMap[job] = task;
            } else {
                KIO::SimpleJob *job =
                    KIO::symlink(task->rightLink(), destURL, ((overWrite || task->overWrite()) ? KIO::Overwrite : KIO::DefaultFlags) | KIO::HideProgressInfo);
                connect(job, &KIO::SimpleJob::result, this, &Synchronizer::slotTaskFinished);
                jobMap[job] = task;
            }
        }
        break;
    case TT_COPY_TO_RIGHT:
        if (task->isDir()) {
            KIO::SimpleJob *job = KIO::mkdir(rightURL);
            connect(job, &KIO::SimpleJob::result, this, &Synchronizer::slotTaskFinished);
            jobMap[job] = task;
            disableNewTasks = true;
        } else {
            QUrl destURL(rightURL);
            if (!task->destination().isNull())
                destURL = Synchronizer::fsUrl(task->destination());

            if (task->leftLink().isNull()) {
                KIO::FileCopyJob *job =
                    KIO::file_copy(leftURL, destURL, -1, ((overWrite || task->overWrite()) ? KIO::Overwrite : KIO::DefaultFlags) | KIO::HideProgressInfo);
                connect(job, SIGNAL(processedSize(KJob *, qulonglong)), this, SLOT(slotProcessedSize(KJob *, qulonglong)));
                connect(job, &KIO::FileCopyJob::result, this, &Synchronizer::slotTaskFinished);
                jobMap[job] = task;
            } else {
                KIO::SimpleJob *job =
                    KIO::symlink(task->leftLink(), destURL, ((overWrite || task->overWrite()) ? KIO::Overwrite : KIO::DefaultFlags) | KIO::HideProgressInfo);
                connect(job, &KIO::SimpleJob::result, this, &Synchronizer::slotTaskFinished);
                jobMap[job] = task;
            }
        }
        break;
    case TT_DELETE: {
        KIO::DeleteJob *job = KIO::del(leftURL, KIO::DefaultFlags);
        connect(job, &KIO::DeleteJob::result, this, &Synchronizer::slotTaskFinished);
        jobMap[job] = task;
    } break;
    default:
        break;
    }
}

void Synchronizer::slotTaskFinished(KJob *job)
{
    inTaskFinished++;

    SynchronizerFileItem *item = jobMap[job];
    jobMap.remove(job);

    KIO::filesize_t receivedSize = 0;

    if (receivedMap.contains(job)) {
        receivedSize = receivedMap[job];
        receivedMap.remove(job);
    }

    if (disableNewTasks && item == lastTask)
        disableNewTasks = false; // the blocker task finished

    QString leftDirName = item->leftDirectory().isEmpty() ? QString("") : item->leftDirectory() + '/';
    QString rightDirName = item->rightDirectory().isEmpty() ? QString("") : item->rightDirectory() + '/';
    QUrl leftURL = Synchronizer::fsUrl(leftBaseDir + leftDirName + item->leftName());
    QUrl rightURL = Synchronizer::fsUrl(rightBaseDir + rightDirName + item->rightName());

    do {
        if (!job->error()) {
            switch (item->task()) {
            case TT_COPY_TO_LEFT:
                if (leftURL.isLocalFile()) {
                    struct utimbuf timestamp;

                    timestamp.actime = time(nullptr);
                    timestamp.modtime = item->rightDate() - timeOffset;

                    utime((const char *)(leftURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), &timestamp);

                    auto newOwnerID = (uid_t)-1; // chown(2) : -1 means no change
                    if (!item->rightOwner().isEmpty()) {
                        struct passwd *pw = getpwnam(QFile::encodeName(item->rightOwner()).data());
                        if (pw != nullptr)
                            newOwnerID = pw->pw_uid;
                    }
                    auto newGroupID = (gid_t)-1; // chown(2) : -1 means no change
                    if (!item->rightGroup().isEmpty()) {
                        struct group *g = getgrnam(QFile::encodeName(item->rightGroup()).data());
                        if (g != nullptr)
                            newGroupID = g->gr_gid;
                    }
                    int status1 = chown((const char *)(leftURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), newOwnerID, (gid_t)-1);
                    int status2 = chown((const char *)(leftURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), (uid_t)-1, newGroupID);
                    if (status1 < 0 || status2 < 0) {
                        // synchronizer currently ignores chown errors
                    }

                    chmod((const char *)(leftURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), item->rightMode() & 07777);

#ifdef HAVE_POSIX_ACL
                    if (!item->rightACL().isNull()) {
                        acl_t acl = acl_from_text(item->rightACL().toLatin1().data());
                        if (acl && !acl_valid(acl))
                            acl_set_file(leftURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data(), ACL_TYPE_ACCESS, acl);
                        if (acl)
                            acl_free(acl);
                    }
#endif
                }
                break;
            case TT_COPY_TO_RIGHT:
                if (rightURL.isLocalFile()) {
                    struct utimbuf timestamp;

                    timestamp.actime = time(nullptr);
                    timestamp.modtime = item->leftDate() + timeOffset;

                    utime((const char *)(rightURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), &timestamp);

                    auto newOwnerID = (uid_t)-1; // chown(2) : -1 means no change
                    if (!item->leftOwner().isEmpty()) {
                        struct passwd *pw = getpwnam(QFile::encodeName(item->leftOwner()).data());
                        if (pw != nullptr)
                            newOwnerID = pw->pw_uid;
                    }
                    auto newGroupID = (gid_t)-1; // chown(2) : -1 means no change
                    if (!item->leftGroup().isEmpty()) {
                        struct group *g = getgrnam(QFile::encodeName(item->leftGroup()).data());
                        if (g != nullptr)
                            newGroupID = g->gr_gid;
                    }
                    int status1 = chown((const char *)(rightURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), newOwnerID, (uid_t)-1);
                    int status2 = chown((const char *)(rightURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), (uid_t)-1, newGroupID);
                    if (status1 < 0 || status2 < 0) {
                        // synchronizer currently ignores chown errors
                    }

                    chmod((const char *)(rightURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data()), item->leftMode() & 07777);

#ifdef HAVE_POSIX_ACL
                    if (!item->leftACL().isNull()) {
                        acl_t acl = acl_from_text(item->leftACL().toLatin1().data());
                        if (acl && !acl_valid(acl))
                            acl_set_file(rightURL.adjusted(QUrl::StripTrailingSlash).path().toLocal8Bit().data(), ACL_TYPE_ACCESS, acl);
                        if (acl)
                            acl_free(acl);
                    }
#endif
                }
                break;
            default:
                break;
            }
        } else {
            if (job->error() == KIO::ERR_FILE_ALREADY_EXIST && item->task() != TT_DELETE) {
                if (autoSkip) {
                    break;
                }

                QUrl srcUrl, destUrl;
                KIO::filesize_t srcSize, destSize;
                time_t srcTime, destTime;
                if (item->task() == TT_COPY_TO_LEFT) {
                    srcUrl = rightURL;
                    destUrl = leftURL;
                    srcSize = item->rightSize();
                    destSize = item->leftSize();
                    srcTime = item->rightDate();
                    destTime = item->leftDate();
                } else {
                    srcUrl = leftURL;
                    destUrl = rightURL;
                    srcSize = item->leftSize();
                    destSize = item->rightSize();
                    srcTime = item->leftDate();
                    destTime = item->rightDate();
                }

                KIO::RenameDialog dialog(syncDlgWidget,
                                         i18n("File Already Exists"),
                                         srcUrl,
                                         destUrl,
                                         KIO::RenameDialog_Overwrite | KIO::RenameDialog_Skip | KIO::RenameDialog_MultipleItems,
                                         srcSize,
                                         destSize,
                                         QDateTime(),
                                         QDateTime(),
                                         QDateTime::fromSecsSinceEpoch(static_cast<uint>(srcTime)),
                                         QDateTime::fromSecsSinceEpoch(static_cast<uint>(destTime)));

                switch (dialog.exec()) {
                case KIO::Result_Rename:
                    item->setDestination(dialog.newDestUrl().toString());
                    executeTask(item);
                    inTaskFinished--;
                    return;
                case KIO::Result_Overwrite:
                    item->setOverWrite();
                    executeTask(item);
                    inTaskFinished--;
                    return;
                case KIO::Result_OverwriteAll:
                    overWrite = true;
                    executeTask(item);
                    inTaskFinished--;
                    return;
                case KIO::Result_AutoSkip:
                    autoSkip = true;
                case KIO::Result_Skip:
                default:
                    break;
                }

                break;
            }

            if (job->error() != KIO::ERR_DOES_NOT_EXIST || item->task() != TT_DELETE) {
                if (autoSkip)
                    break;

                QString error;

                switch (item->task()) {
                case TT_COPY_TO_LEFT:
                    error = i18n("Error at copying file %1 to %2.",
                                 rightURL.toDisplayString(QUrl::PreferLocalFile),
                                 leftURL.toDisplayString(QUrl::PreferLocalFile));
                    break;
                case TT_COPY_TO_RIGHT:
                    error = i18n("Error at copying file %1 to %2.",
                                 leftURL.toDisplayString(QUrl::PreferLocalFile),
                                 rightURL.toDisplayString(QUrl::PreferLocalFile));
                    break;
                case TT_DELETE:
                    error = i18n("Error at deleting file %1.", leftURL.toDisplayString(QUrl::PreferLocalFile));
                    break;
                default:
                    break;
                }

                KIO::SkipDialog dialog(syncDlgWidget, KIO::SkipDialog_MultipleItems, error);

                switch (dialog.exec()) {
                case KIO::Result_Cancel:
                    executeTask(item); /* simply retry */
                    inTaskFinished--;
                    return;
                case KIO::Result_AutoSkip:
                    autoSkip = true;
                default:
                    break;
                }
            }
        }
    } while (false);

    switch (item->task()) {
    case TT_COPY_TO_LEFT:
        leftCopyNr++;
        leftCopySize += item->rightSize() - receivedSize;
        break;
    case TT_COPY_TO_RIGHT:
        rightCopyNr++;
        rightCopySize += item->leftSize() - receivedSize;
        break;
    case TT_DELETE:
        deleteNr++;
        deleteSize += item->leftSize() - receivedSize;
        break;
    default:
        break;
    }

    emit processedSizes(leftCopyNr, leftCopySize, rightCopyNr, rightCopySize, deleteNr, deleteSize);

    if (--inTaskFinished == 0) {
        if (paused)
            emit pauseAccepted();
        else
            synchronizeLoop();
    }
}

void Synchronizer::slotProcessedSize(KJob *job, qulonglong size)
{
    KIO::filesize_t dl = 0, dr = 0, dd = 0;
    SynchronizerFileItem *item = jobMap[job];

    KIO::filesize_t lastProcessedSize = 0;
    if (receivedMap.contains(job))
        lastProcessedSize = receivedMap[job];

    receivedMap[job] = size;

    switch (item->task()) {
    case TT_COPY_TO_LEFT:
        dl = size - lastProcessedSize;
        break;
    case TT_COPY_TO_RIGHT:
        dr = size - lastProcessedSize;
        break;
    case TT_DELETE:
        dd = size - lastProcessedSize;
        break;
    default:
        break;
    }

    emit processedSizes(leftCopyNr, leftCopySize += dl, rightCopyNr, rightCopySize += dr, deleteNr, deleteSize += dd);
}

void Synchronizer::pause()
{
    paused = true;
}

void Synchronizer::resume()
{
    paused = false;
    synchronizeLoop();
}

QString Synchronizer::leftBaseDirectory()
{
    return leftBaseDir;
}

QString Synchronizer::rightBaseDirectory()
{
    return rightBaseDir;
}

KgetProgressDialog::KgetProgressDialog(QWidget *parent, const QString &caption, const QString &text, bool modal)
    : QDialog(parent)
{
    if (caption.isEmpty())
        setWindowTitle(caption);
    setModal(modal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addWidget(new QLabel(text));

    mProgressBar = new QProgressBar;
    mainLayout->addWidget(mProgressBar);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    mPauseButton = new QPushButton(i18n("Pause"));
    buttonBox->addButton(mPauseButton, QDialogButtonBox::ActionRole);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    connect(mPauseButton, &QPushButton::clicked, this, &KgetProgressDialog::slotPause);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KgetProgressDialog::slotCancel);

    mCancelled = mPaused = false;
}

void KgetProgressDialog::slotPause()
{
    if ((mPaused = !mPaused) == false)
        mPauseButton->setText(i18n("Pause"));
    else
        mPauseButton->setText(i18n("Resume"));
}

void KgetProgressDialog::slotCancel()
{
    mCancelled = true;
    reject();
}

void Synchronizer::synchronizeWithKGet()
{
    bool isLeftLocal = QUrl::fromUserInput(leftBaseDirectory(), QString(), QUrl::AssumeLocalFile).isLocalFile();
    KgetProgressDialog *progDlg = nullptr;
    int processedCount = 0, totalCount = 0;

    QListIterator<SynchronizerFileItem *> it(resultList);
    while (it.hasNext())
        if (it.next()->isMarked())
            totalCount++;

    it.toFront();
    while (it.hasNext()) {
        SynchronizerFileItem *item = it.next();

        if (item->isMarked()) {
            QUrl downloadURL;
            QUrl destURL;
            QString destDir;
            QString leftDirName = item->leftDirectory().isEmpty() ? QString("") : item->leftDirectory() + '/';
            QString rightDirName = item->rightDirectory().isEmpty() ? QString("") : item->rightDirectory() + '/';

            if (progDlg == nullptr) {
                progDlg = new KgetProgressDialog(krMainWindow, i18n("Krusader::Synchronizer"), i18n("Feeding the URLs to KGet"), true);
                progDlg->progressBar()->setMaximum(totalCount);
                progDlg->show();
                qApp->processEvents();
            }

            if (item->task() == TT_COPY_TO_RIGHT && !isLeftLocal) {
                downloadURL = Synchronizer::fsUrl(leftBaseDirectory() + leftDirName + item->leftName());
                destDir = rightBaseDirectory() + rightDirName;
                destURL = Synchronizer::fsUrl(destDir + item->rightName());

                if (item->isDir())
                    destDir += item->leftName();
            }
            if (item->task() == TT_COPY_TO_LEFT && isLeftLocal) {
                downloadURL = Synchronizer::fsUrl(rightBaseDirectory() + rightDirName + item->rightName());
                destDir = leftBaseDirectory() + leftDirName;
                destURL = Synchronizer::fsUrl(destDir + item->leftName());

                if (item->isDir())
                    destDir += item->rightName();
            }

            // creating the directory system
            for (qsizetype i = 0; i >= 0; i = destDir.indexOf('/', i + 1))
                if (!QDir(destDir.left(i)).exists())
                    QDir().mkdir(destDir.left(i));

            if (!item->isDir() && !downloadURL.isEmpty()) {
                if (QFile(destURL.path()).exists())
                    QFile(destURL.path()).remove();

                QString source = downloadURL.toDisplayString();
                if (source.indexOf('@') >= 2) { /* is this an ftp proxy URL? */
                    qsizetype lastAt = source.lastIndexOf('@');
                    QString startString = source.left(lastAt);
                    QString endString = source.mid(lastAt);
                    startString.replace('@', "%40");
                    source = startString + endString;
                }

                KProcess p;

                p << KrServices::fullPathName("kget") << source << destURL.path();
                if (!p.startDetached())
                    KMessageBox::error(parentWidget, i18n("Error executing %1.", KrServices::fullPathName("kget")));
            }

            progDlg->progressBar()->setValue(++processedCount);

            QElapsedTimer t;
            t.start();
            bool canExit = false;

            do {
                qApp->processEvents();

                if (progDlg->wasCancelled())
                    break;

                canExit = (t.elapsed() > 100);

                if (progDlg->isPaused() || !canExit)
                    usleep(10000);

            } while (progDlg->isPaused() || !canExit);

            if (progDlg->wasCancelled())
                break;
        }
    }

    if (progDlg)
        delete progDlg;
}

bool Synchronizer::isDir(const FileItem *file)
{
    if (followSymLinks) {
        return file->isDir();
    } else {
        return file->isDir() && !file->isSymLink();
    }
}

QString Synchronizer::readLink(const FileItem *file)
{
    if (file->isSymLink())
        return file->getSymDest();
    else
        return QString();
}

SynchronizerFileItem *Synchronizer::getItemAt(unsigned ndx)
{
    if (ndx < (unsigned)resultList.count())
        return resultList.at(ndx);
    else
        return nullptr;
}
