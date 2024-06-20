/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitter.h"
#include "../FileSystem/filesystem.h"

// QtCore
#include <QFileInfo>
// QtWidgets
#include <QLayout>

#include <KFileItem>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KIO/StatJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <kio_version.h>
#include <utility>

Splitter::Splitter(QWidget *parent, QUrl fileNameIn, QUrl destinationDirIn, bool overWriteIn)
    : QProgressDialog(parent, Qt::WindowFlags())
    , fileName(std::move(fileNameIn))
    , destinationDir(std::move(destinationDirIn))
    , splitSize(0)
    , permissions(0)
    , overwrite(overWriteIn)
    , fileNumber(0)
    , outputFileRemaining(0)
    , receivedSize(0)
    , crcContext(new CRC32())
    , statJob(nullptr)
    , splitReadJob(nullptr)
    , splitWriteJob(nullptr)

{
    setMaximum(100);
    setAutoClose(false); /* don't close or reset the dialog automatically */
    setAutoReset(false);
    setLabelText("Krusader::Splitter");
    setWindowModality(Qt::WindowModal);
}

Splitter::~Splitter()
{
    splitAbortJobs();
    delete crcContext;
}

void Splitter::split(KIO::filesize_t splitSizeIn)
{
    Q_ASSERT(!splitReadJob);
    Q_ASSERT(!splitWriteJob);
    Q_ASSERT(!fileNumber);
    Q_ASSERT(!receivedSize);
    Q_ASSERT(!outputFileRemaining);

    splitReadJob = splitWriteJob = nullptr;
    fileNumber = 0;
    receivedSize = outputFileRemaining = 0;

    splitSize = splitSizeIn;

    KFileItem file(fileName);
    file.refresh(); // FIXME: works only for local files - use KIO::stat() instead

    permissions = file.permissions() | QFile::WriteUser;

    setWindowTitle(i18n("Krusader::Splitting..."));
    setLabelText(i18n("Splitting the file %1...", fileName.toDisplayString(QUrl::PreferLocalFile)));

    if (file.isDir()) {
        KMessageBox::error(nullptr, i18n("Cannot split a folder."));
        return;
    }

    splitReadJob = KIO::get(fileName, KIO::NoReload, KIO::HideProgressInfo);

    connect(splitReadJob, &KIO::TransferJob::data, this, &Splitter::splitDataReceived);
    connect(splitReadJob, &KIO::TransferJob::result, this, &Splitter::splitReceiveFinished);
    connect(splitReadJob, &KJob::percentChanged, this, &Splitter::splitReceivePercent);

    exec();
}

void Splitter::splitDataReceived(KIO::Job *, const QByteArray &byteArray)
{
    Q_ASSERT(!transferArray.length()); // transfer buffer must be empty

    if (byteArray.size() == 0)
        return;

    crcContext->update(reinterpret_cast<unsigned char *>(const_cast<char *>(byteArray.data())), byteArray.size());
    receivedSize += byteArray.size();

    if (!splitWriteJob)
        nextOutputFile();

    transferArray = QByteArray(byteArray.data(), byteArray.length());

    // suspend read job until transfer buffer is handed to the write job
    splitReadJob->suspend();

    if (splitWriteJob)
        splitWriteJob->resume();
}

void Splitter::splitReceiveFinished(KJob *job)
{
    splitReadJob = nullptr; /* KIO automatically deletes the object after Finished signal */

    if (splitWriteJob)
        splitWriteJob->resume(); // finish writing the output

    if (job->error()) { /* any error occurred? */
        splitAbortJobs();
        KMessageBox::error(nullptr, i18n("Error reading file %1: %2", fileName.toDisplayString(QUrl::PreferLocalFile), job->errorString()));
        reject();
        return;
    }

    QString crcResult = QString("%1").arg(crcContext->result(), 0, 16).toUpper().trimmed().rightJustified(8, '0');

    splitInfoFileContent =
        QString("filename=%1\n").arg(fileName.fileName()) + QString("size=%1\n").arg(KIO::number(receivedSize)) + QString("crc32=%1\n").arg(crcResult);
}

void Splitter::splitReceivePercent(KJob *, unsigned long percent)
{
    setValue(static_cast<int>(percent));
}

void Splitter::nextOutputFile()
{
    Q_ASSERT(!outputFileRemaining);

    fileNumber++;

    outputFileRemaining = splitSize;

    QString index("%1"); /* making the split filename */
    index = index.arg(fileNumber).rightJustified(3, '0');
    QString outFileName = fileName.fileName() + '.' + index;

    writeURL = destinationDir;
    writeURL = writeURL.adjusted(QUrl::StripTrailingSlash);
    writeURL.setPath(writeURL.path() + '/' + (outFileName));

    if (overwrite)
        openOutputFile();
    else {
        statJob = KIO::stat(writeURL, KIO::StatJob::DestinationSide, KIO::StatNoDetails, KIO::HideProgressInfo);
        connect(statJob, &KIO::Job::result, this, &Splitter::statOutputFileResult);
    }
}

void Splitter::statOutputFileResult(KJob *job)
{
    statJob = nullptr;

    if (job->error()) {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST)
            openOutputFile();
        else {
            dynamic_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
            reject();
        }
    } else { // destination already exists
        KIO::RenameDialog dlg(this,
                              i18n("File Already Exists"),
                              QUrl(),
                              writeURL,
                              static_cast<KIO::RenameDialog_Options>(KIO::RenameDialog_MultipleItems  | KIO::RenameDialog_Overwrite | KIO::RenameDialog_NoRename));
        switch (dlg.exec()) {
        case KIO::Result_Overwrite:
            openOutputFile();
            break;
        case KIO::Result_OverwriteAll:
            overwrite = true;
            openOutputFile();
            break;
        default:
            reject();
        }
    }
}

void Splitter::openOutputFile()
{
    // create write job
    splitWriteJob = KIO::put(writeURL, permissions, KIO::HideProgressInfo | KIO::Overwrite);
    connect(splitWriteJob, &KIO::TransferJob::dataReq, this, &Splitter::splitDataSend);
    connect(splitWriteJob, &KIO::TransferJob::result, this, &Splitter::splitSendFinished);
}

void Splitter::splitDataSend(KIO::Job *, QByteArray &byteArray)
{
    KIO::filesize_t bufferLen = transferArray.size();

    if (!outputFileRemaining) { // current output file needs to be closed ?
        byteArray = QByteArray(); // giving empty buffer which indicates closing
    } else if (bufferLen > outputFileRemaining) { // maximum length reached ?
        byteArray = QByteArray(transferArray.data(), static_cast<int>(outputFileRemaining));
        transferArray = QByteArray(transferArray.data() + outputFileRemaining, static_cast<int>(bufferLen - outputFileRemaining));
        outputFileRemaining = 0;
    } else {
        outputFileRemaining -= bufferLen; // write the whole buffer to the output file

        byteArray = transferArray;
        transferArray = QByteArray();

        if (splitReadJob) {
            // suspend write job until transfer buffer is filled or the read job is finished
            splitWriteJob->suspend();
            splitReadJob->resume();
        } // else: write job continues until transfer buffer is empty
    }
}

void Splitter::splitSendFinished(KJob *job)
{
    splitWriteJob = nullptr; /* KIO automatically deletes the object after Finished signal */

    if (job->error()) { /* any error occurred? */
        splitAbortJobs();
        KMessageBox::error(nullptr, i18n("Error writing file %1: %2", writeURL.toDisplayString(QUrl::PreferLocalFile), job->errorString()));
        reject();
        return;
    }

    if (transferArray.size()) /* any data remained in the transfer buffer? */
        nextOutputFile();
    else if (splitReadJob)
        splitReadJob->resume();
    else { // read job is finished and transfer buffer is empty -> splitting is finished
        /* writing the split information file out */
        writeURL = destinationDir;
        writeURL = writeURL.adjusted(QUrl::StripTrailingSlash);
        writeURL.setPath(writeURL.path() + '/' + (fileName.fileName() + ".crc"));
        splitWriteJob = KIO::put(writeURL, permissions, KIO::HideProgressInfo | KIO::Overwrite);
        connect(splitWriteJob, &KIO::TransferJob::dataReq, this, &Splitter::splitFileSend);
        connect(splitWriteJob, &KIO::TransferJob::result, this, &Splitter::splitFileFinished);
    }
}

void Splitter::splitAbortJobs()
{
    if (statJob)
        statJob->kill(KJob::Quietly);
    if (splitReadJob)
        splitReadJob->kill(KJob::Quietly);
    if (splitWriteJob)
        splitWriteJob->kill(KJob::Quietly);

    splitReadJob = splitWriteJob = nullptr;
}

void Splitter::splitFileSend(KIO::Job *, QByteArray &byteArray)
{
    byteArray = splitInfoFileContent.toLocal8Bit();
    splitInfoFileContent = "";
}

void Splitter::splitFileFinished(KJob *job)
{
    splitWriteJob = nullptr; /* KIO automatically deletes the object after Finished signal */

    if (job->error()) { /* any error occurred? */
        KMessageBox::error(nullptr, i18n("Error writing file %1: %2", writeURL.toDisplayString(QUrl::PreferLocalFile), job->errorString()));
        reject();
        return;
    }

    accept();
}
