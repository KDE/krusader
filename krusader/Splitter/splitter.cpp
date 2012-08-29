/***************************************************************************
                          splitter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "splitter.h"
#include "../VFS/vfs.h"

#include <QtGui/QLayout>
#include <QtCore/QFileInfo>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kfileitem.h>
#include <kio/jobuidelegate.h>


Splitter::Splitter(QWidget* parent,  KUrl fileNameIn, KUrl destinationDirIn) :
        QProgressDialog(parent, 0),
        fileName(fileNameIn),
        destinationDir(destinationDirIn),
        splitSize(0),
        permissions(0),
        overwrite(false),
        fileNumber(0),
        outputFileRemaining(0),
        recievedSize(0),
        crcContext(new CRC32()),
        statJob(0),
        splitReadJob(0),
        splitWriteJob(0)

{
    setMaximum(100);
    setAutoClose(false);    /* don't close or reset the dialog automatically */
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
    Q_ASSERT(!recievedSize);
    Q_ASSERT(!outputFileRemaining);

    splitReadJob = splitWriteJob = 0;
    fileNumber = recievedSize = outputFileRemaining = 0;

    splitSize = splitSizeIn;

    KFileItem file(KFileItem::Unknown, KFileItem::Unknown, fileName);
    file.refresh(); //FIXME: works only for local files - use KIO::stat() instead

    permissions = file.permissions() | QFile::WriteUser;

    setWindowTitle(i18n("Krusader::Splitting..."));
    setLabelText(i18n("Splitting the file %1...", fileName.pathOrUrl()));

    if (file.isDir()) {
        KMessageBox::error(0, i18n("Can't split a directory!"));
        return;
    }

    splitReadJob = KIO::get(fileName, KIO::NoReload, KIO::HideProgressInfo);

    connect(splitReadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            this, SLOT(splitDataReceived(KIO::Job *, const QByteArray &)));
    connect(splitReadJob, SIGNAL(result(KJob*)),
            this, SLOT(splitReceiveFinished(KJob *)));
    connect(splitReadJob, SIGNAL(percent(KJob *, unsigned long)),
            this, SLOT(splitReceivePercent(KJob *, unsigned long)));

    exec();
}

void Splitter::splitDataReceived(KIO::Job *, const QByteArray &byteArray)
{
    Q_ASSERT(!transferArray.length()); // transfer buffer must be empty

    if (byteArray.size() == 0)
        return;

    crcContext->update((unsigned char *)byteArray.data(), byteArray.size());
    recievedSize += byteArray.size();

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
    splitReadJob = 0;   /* KIO automatically deletes the object after Finished signal */

    if (splitWriteJob)
        splitWriteJob->resume(); // finish writing the output

    if (job->error()) {   /* any error occurred? */
        splitAbortJobs();
        KMessageBox::error(0, i18n("Error reading file %1: $2", fileName.pathOrUrl(),
                                   job->errorString()));
        emit reject();
        return;
    }

    QString crcResult = QString("%1").arg(crcContext->result(), 0, 16).toUpper().trimmed()
                        .rightJustified(8, '0');

    splitInfoFileContent = QString("filename=%1\n").arg(fileName.fileName()) +
                QString("size=%1\n")    .arg(KIO::number(recievedSize)) +
                QString("crc32=%1\n")   .arg(crcResult);
}

void Splitter::splitReceivePercent(KJob *, unsigned long percent)
{
    setValue(percent);
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
    writeURL.addPath(outFileName);

    if (overwrite)
        openOutputFile();
    else {
        statJob = KIO::stat(writeURL, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        connect(statJob, SIGNAL(result(KJob*)), SLOT(statOutputFileResult(KJob*)));
    }
}

void Splitter::statOutputFileResult(KJob* job)
{
    statJob = 0;

    if (job->error()) {
        if (job->error() == KIO::ERR_DOES_NOT_EXIST)
            openOutputFile();
        else {
            static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
            emit reject();
        }
    } else { // destination already exists
        KIO::RenameDialog dlg(this, i18n("File Already Exists"), QUrl(), writeURL,
                static_cast<KIO::RenameDialog_Mode>(KIO::M_MULTI | KIO::M_OVERWRITE | KIO::M_NORENAME));
        switch (dlg.exec()) {
        case KIO::R_OVERWRITE:
            openOutputFile();
            break;
        case KIO::R_OVERWRITE_ALL:
            overwrite = true;
            openOutputFile();
            break;
        default:
            emit reject();
        }
    }
}

void Splitter::openOutputFile()
{
    // create write job
    splitWriteJob = KIO::put(writeURL, permissions, KIO::HideProgressInfo | KIO::Overwrite);
    connect(splitWriteJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
            this, SLOT(splitDataSend(KIO::Job *, QByteArray &)));
    connect(splitWriteJob, SIGNAL(result(KJob*)),
            this, SLOT(splitSendFinished(KJob *)));

}

void Splitter::splitDataSend(KIO::Job *, QByteArray &byteArray)
{
    KIO::filesize_t bufferLen = transferArray.size();

    if (!outputFileRemaining) { // current output file needs to be closed ?
        byteArray = QByteArray();  // giving empty buffer which indicates closing
    } else if (bufferLen > outputFileRemaining) { // maximum length reached ?
        byteArray = QByteArray(transferArray.data(), outputFileRemaining);
        transferArray = QByteArray(transferArray.data() + outputFileRemaining,
                                   bufferLen - outputFileRemaining);
        outputFileRemaining = 0;
    } else {
        outputFileRemaining -= bufferLen;  // write the whole buffer to the output file

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
    splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

    if (job->error()) {   /* any error occurred? */
        splitAbortJobs();
        KMessageBox::error(0, i18n("Error writing file %1: %2", writeURL.pathOrUrl(),
                                   job->errorString()));
        emit reject();
        return;
    }

    if (transferArray.size())  /* any data remained in the transfer buffer? */
        nextOutputFile();
    else if (splitReadJob)
        splitReadJob->resume();
    else { // read job is finished and transfer buffer is empty -> splitting is finished
        /* writing the split information file out */
        writeURL      = destinationDir;
        writeURL.addPath(fileName.fileName() + ".crc");
        splitWriteJob = KIO::put(writeURL, permissions, KIO::HideProgressInfo | KIO::Overwrite);
        connect(splitWriteJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
                this, SLOT(splitFileSend(KIO::Job *, QByteArray &)));
        connect(splitWriteJob, SIGNAL(result(KJob*)),
                this, SLOT(splitFileFinished(KJob *)));
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

    splitReadJob = splitWriteJob = 0;
}

void Splitter::splitFileSend(KIO::Job *, QByteArray &byteArray)
{
    byteArray = splitInfoFileContent.toLocal8Bit();
    splitInfoFileContent = "";
}

void Splitter::splitFileFinished(KJob *job)
{
    splitWriteJob = 0;  /* KIO automatically deletes the object after Finished signal */

    if (job->error()) {   /* any error occurred? */
        KMessageBox::error(0, i18n("Error at writing file %1: %2", writeURL.pathOrUrl(),
                                   job->errorString()));
        emit reject();
        return;
    }

    emit accept();
}

#include "splitter.moc"
