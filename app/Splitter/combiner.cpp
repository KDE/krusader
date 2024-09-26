/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "combiner.h"
#include "../FileSystem/filesystem.h"

// QtCore
#include <QFileInfo>

#include <KFileItem>
#include <KIO/Job>
#include <KIO/StatJob>
#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <kio_version.h>
#include <utility>

// TODO: delete destination file on error
// TODO: cache more than one byte array of data

Combiner::Combiner(QWidget *parent, QUrl baseURLIn, QUrl destinationURLIn, bool unixNamingIn)
    : QProgressDialog(parent, Qt::WindowFlags())
    , baseURL(std::move(baseURLIn))
    , destinationURL(std::move(destinationURLIn))
    , hasValidSplitFile(false)
    , fileCounter(0)
    , permissions(-1)
    , receivedSize(0)
    , statJob(nullptr)
    , combineReadJob(nullptr)
    , combineWriteJob(nullptr)
    , unixNaming(unixNamingIn)
{
    crcContext = new CRC32();

    splitFile = "";

    setMaximum(100);
    setAutoClose(false); /* don't close or reset the dialog automatically */
    setAutoReset(false);
    setLabelText("Krusader::Combiner");
    setWindowModality(Qt::WindowModal);

    firstFileIs000 = true; // start with this assumption, will set it to false as soon as .000 isn't found
}

Combiner::~Combiner()
{
    combineAbortJobs();
    delete crcContext;
}

void Combiner::combine()
{
    setWindowTitle(i18n("Krusader::Combining..."));
    setLabelText(i18n("Combining the file %1...", baseURL.toDisplayString(QUrl::PreferLocalFile)));

    /* check whether the .crc file exists */
    splURL = baseURL.adjusted(QUrl::RemoveFilename);
    splURL.setPath(splURL.path() + baseURL.fileName() + ".crc");
    KFileItem file(splURL);
    // FIXME: works only for local files - use KIO::stat() instead
    file.refresh();

    if (!file.isReadable()) {
        int ret = KMessageBox::questionTwoActions(nullptr,
                                             i18n("The CRC information file (%1) is missing.\n"
                                                  "Validity checking is impossible without it. Continue combining?",
                                                  splURL.toDisplayString(QUrl::PreferLocalFile)),
                                             {},
                                             KStandardGuiItem::cont(),
                                             KStandardGuiItem::cancel());

        if (ret == KMessageBox::SecondaryAction) {
            reject();
            return;
        }

        statDest();
    } else {
        permissions = file.permissions() | QFile::WriteUser;

        combineReadJob = KIO::get(splURL, KIO::NoReload, KIO::HideProgressInfo);

        connect(combineReadJob, &KIO::TransferJob::data, this, &Combiner::combineSplitFileDataReceived);
        connect(combineReadJob, &KIO::TransferJob::result, this, &Combiner::combineSplitFileFinished);
    }

    exec();
}

void Combiner::combineSplitFileDataReceived(KIO::Job *, const QByteArray &byteArray)
{
    splitFile += QString(byteArray);
}

void Combiner::combineSplitFileFinished(KJob *job)
{
    combineReadJob = nullptr;
    QString error;

    if (job->error())
        error = i18n("Error at reading the CRC file (%1).", splURL.toDisplayString(QUrl::PreferLocalFile));
    else {
        splitFile.remove('\r'); // Windows compatibility
        QStringList splitFileContent = splitFile.split('\n');

        bool hasFileName = false, hasSize = false, hasCrc = false;

        for (int i = 0; i != splitFileContent.count(); i++) {
            qsizetype ndx = splitFileContent[i].indexOf('=');
            if (ndx == -1)
                continue;
            QString token = splitFileContent[i].left(ndx).trimmed();
            QString value = splitFileContent[i].mid(ndx + 1);

            if (token == "filename") {
                expectedFileName = value;
                hasFileName = true;
            } else if (token == "size") {
                // FIXME - don't use c functions !!!
                sscanf(value.trimmed().toLocal8Bit().data(), "%llu", &expectedSize);
                hasSize = true;
            }
            if (token == "crc32") {
                expectedCrcSum = value.trimmed().rightJustified(8, '0');
                hasCrc = true;
            }
        }

        if (!hasFileName || !hasSize || !hasCrc)
            error = i18n("Not a valid CRC file.");
        else
            hasValidSplitFile = true;
    }

    if (!error.isEmpty()) {
        int ret = KMessageBox::questionTwoActions(nullptr,
                                             error + i18n("\nValidity checking is impossible without a good CRC file. Continue combining?"),
                                             {},
                                             KStandardGuiItem::cont(),
                                             KStandardGuiItem::cancel());
        if (ret == KMessageBox::SecondaryAction) {
            reject();
            return;
        }
    }

    statDest();
}

void Combiner::statDest()
{
    if (writeURL.isEmpty()) {
        writeURL = FileSystem::ensureTrailingSlash(destinationURL);
        if (hasValidSplitFile)
            writeURL.setPath(writeURL.path() + expectedFileName);
        else if (unixNaming)
            writeURL.setPath(writeURL.path() + baseURL.fileName() + ".out");
        else
            writeURL.setPath(writeURL.path() + baseURL.fileName());
    }

    statJob = KIO::stat(writeURL, KIO::StatJob::DestinationSide, KIO::StatNoDetails, KIO::HideProgressInfo);
    connect(statJob, &KIO::StatJob::result, this, &Combiner::statDestResult);
}

void Combiner::statDestResult(KJob *job)
{
    statJob = nullptr;

    if (job->error()) {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST) {
            openNextFile();
        } else {
            dynamic_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
            reject();
        }
    } else { // destination already exists
        KIO::RenameDialog_Options mode = dynamic_cast<KIO::StatJob *>(job)->statResult().isDir() ? KIO::RenameDialog_DestIsDirectory : KIO::RenameDialog_Overwrite;
        KIO::RenameDialog dlg(this, i18n("File Already Exists"), QUrl(), writeURL, mode);
        switch (dlg.exec()) {
        case KIO::Result_Overwrite:
            openNextFile();
            break;
        case KIO::Result_Rename: {
            writeURL = dlg.newDestUrl();
            statDest();
            break;
        }
        default:
            reject();
        }
    }
}

void Combiner::openNextFile()
{
    if (unixNaming) {
        if (readURL.isEmpty())
            readURL = baseURL;
        else {
            QString name = readURL.fileName();
            qsizetype pos = name.length() - 1;
            QChar ch;

            do {
                ch = QChar(name.at(pos).toLatin1() + 1);
                if (ch == QChar('Z' + 1))
                    ch = 'A';
                if (ch == QChar('z' + 1))
                    ch = 'a';
                name[pos] = ch;
                pos--;
            } while (pos >= 0 && ch.toUpper() == QChar('A'));
            readURL = readURL.adjusted(QUrl::RemoveFilename);
            readURL.setPath(readURL.path() + name);
        }
    } else {
        QString index("%1"); /* determining the filename */
        index = index.arg(fileCounter++).rightJustified(3, '0');
        readURL = baseURL.adjusted(QUrl::RemoveFilename);
        readURL.setPath(readURL.path() + baseURL.fileName() + '.' + index);
    }

    /* creating a read job */
    combineReadJob = KIO::get(readURL, KIO::NoReload, KIO::HideProgressInfo);

    connect(combineReadJob, &KIO::TransferJob::data, this, &Combiner::combineDataReceived);
    connect(combineReadJob, &KIO::TransferJob::result, this, &Combiner::combineReceiveFinished);
    if (hasValidSplitFile)
        connect(combineReadJob, &KJob::percentChanged, this, &Combiner::combineWritePercent);
}

void Combiner::combineDataReceived(KIO::Job *, const QByteArray &byteArray)
{
    if (byteArray.size() == 0)
        return;

    crcContext->update(reinterpret_cast<unsigned char *>(const_cast<char *>(byteArray.data())), byteArray.size());
    transferArray = QByteArray(byteArray.data(), byteArray.length());

    receivedSize += byteArray.size();

    if (combineWriteJob == nullptr) {
        combineWriteJob = KIO::put(writeURL, permissions, KIO::HideProgressInfo | KIO::Overwrite);

        connect(combineWriteJob, &KIO::TransferJob::dataReq, this, &Combiner::combineDataSend);
        connect(combineWriteJob, &KIO::TransferJob::result, this, &Combiner::combineSendFinished);
    }

    // continue writing and suspend read job until received data is handed over to the write job
    combineReadJob->suspend();
    combineWriteJob->resume();
}

void Combiner::combineReceiveFinished(KJob *job)
{
    combineReadJob = nullptr; /* KIO automatically deletes the object after Finished signal */
    if (job->error()) {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST) {
            if (fileCounter == 1) { // .000 file doesn't exist but .001 is still a valid first file
                firstFileIs000 = false;
                openNextFile();
            } else if (!firstFileIs000 && fileCounter == 2) { // neither .000 nor .001 exist
                combineAbortJobs();
                KMessageBox::error(nullptr, i18n("Cannot open the first split file of %1.", baseURL.toDisplayString(QUrl::PreferLocalFile)));
                reject();
            } else { // we've received the last file
                // write out the remaining part of the file
                combineWriteJob->resume();

                if (hasValidSplitFile) {
                    QString crcResult = QString("%1").arg(crcContext->result(), 0, 16).toUpper().trimmed().rightJustified(8, '0');

                    if (receivedSize != expectedSize)
                        error = i18n("Incorrect filesize, the file might have been corrupted.");
                    else if (crcResult != expectedCrcSum.toUpper().trimmed())
                        error = i18n("Incorrect CRC checksum, the file might have been corrupted.");
                }
            }
        } else {
            combineAbortJobs();
            dynamic_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
            reject();
        }
    } else
        openNextFile();
}

void Combiner::combineDataSend(KIO::Job *, QByteArray &byteArray)
{
    byteArray = transferArray;
    transferArray = QByteArray();

    if (combineReadJob) {
        // continue reading and suspend write job until data is available
        combineReadJob->resume();
        combineWriteJob->suspend();
    }
}

void Combiner::combineSendFinished(KJob *job)
{
    combineWriteJob = nullptr; /* KIO automatically deletes the object after Finished signal */

    if (job->error()) { /* any error occurred? */
        combineAbortJobs();
        dynamic_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
        reject();
    } else if (!error.isEmpty()) { /* was any error message at reading ? */
        combineAbortJobs(); /* we cannot write out it in combineReceiveFinished */
        KMessageBox::error(nullptr, error); /* because emit accept closes it in this function */
        reject();
    } else
        accept();
}

void Combiner::combineAbortJobs()
{
    if (statJob)
        statJob->kill(KJob::Quietly);
    if (combineReadJob)
        combineReadJob->kill(KJob::Quietly);
    if (combineWriteJob)
        combineWriteJob->kill(KJob::Quietly);

    statJob = combineReadJob = combineWriteJob = nullptr;
}

void Combiner::combineWritePercent(KJob *, unsigned long)
{
    auto percent = static_cast<int>(((static_cast<long double>(receivedSize) / expectedSize) * 100.) + 0.5);
    setValue(percent);
}
