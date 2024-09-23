/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITTER_H
#define SPLITTER_H

// QtCore
#include <QString>
#include <QUrl>
// QtWidgets
#include <QProgressDialog>

#include <KIO/Job>
#include <KIO/TransferJob>
#include <KIO/Global>

#include "crc32.h"

class Splitter : public QProgressDialog
{
    Q_OBJECT

public:
    Splitter(QWidget *parent, QUrl fileNameIn, QUrl destinationDirIn, bool overWriteIn);
    ~Splitter() override;

    void split(KIO::filesize_t splitSizeIn);

private slots:
    void splitDataReceived(KIO::Job *, const QByteArray &);
    void splitDataSend(KIO::Job *, QByteArray &);
    void splitSendFinished(KJob *);
    void splitReceiveFinished(KJob *);
    void splitReceivePercent(KJob *, unsigned long);
    void splitFileSend(KIO::Job *, QByteArray &);
    void splitFileFinished(KJob *);
    void statOutputFileResult(KJob *job);

private:
    void splitAbortJobs();
    void nextOutputFile();
    void openOutputFile();

    // parameters
    QUrl fileName;
    QUrl destinationDir;
    KIO::filesize_t splitSize;
    int permissions;
    bool overwrite;

    // current split file stuff
    int fileNumber;
    QUrl writeURL;
    // how much can still be written to the current output file
    KIO::filesize_t outputFileRemaining;

    QByteArray transferArray;
    KIO::filesize_t receivedSize;
    QString splitInfoFileContent;
    CRC32 *crcContext;
    KIO::Job *statJob;
    KIO::TransferJob *splitReadJob;
    KIO::TransferJob *splitWriteJob;
};

#endif /* __SPLITTER_H__ */
