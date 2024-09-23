/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMBINER_H
#define COMBINER_H

// QtCore
#include <QString>
#include <QUrl>
// QtWidgets
#include <QProgressDialog>

#include <KIO/Job>
#include <KIO/Global>
#include <KIO/TransferJob>

#include "crc32.h"

class Combiner : public QProgressDialog
{
    Q_OBJECT

public:
    Combiner(QWidget *parent, QUrl baseURLIn, QUrl destinationURLIn, bool unixNamingIn = false);
    ~Combiner() override;

    void combine();

private slots:
    void statDest();
    void statDestResult(KJob *job);
    void combineSplitFileDataReceived(KIO::Job *, const QByteArray &byteArray);
    void combineSplitFileFinished(KJob *job);
    void combineDataReceived(KIO::Job *, const QByteArray &);
    void combineReceiveFinished(KJob *);
    void combineDataSend(KIO::Job *, QByteArray &);
    void combineSendFinished(KJob *);
    void combineWritePercent(KJob *, unsigned long);

private:
    void openNextFile();
    void combineAbortJobs();

    QUrl splURL;
    QUrl readURL;
    QUrl writeURL;

    QUrl baseURL;
    QUrl destinationURL;
    CRC32 *crcContext;
    QByteArray transferArray;

    QString splitFile;
    QString error;

    bool hasValidSplitFile;
    QString expectedFileName;
    KIO::filesize_t expectedSize;
    QString expectedCrcSum;

    int fileCounter;
    bool firstFileIs000;
    int permissions;
    KIO::filesize_t receivedSize;

    KIO::Job *statJob;
    KIO::TransferJob *combineReadJob;
    KIO::TransferJob *combineWriteJob;

    bool unixNaming;
};

#endif /* __COMBINER_H__ */
